/*
 * File  : ExportDlg.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_header.h>

#include "Constants.h"
#include "ConfigService.h"
#include "Config.h"
#include "UiScaler.h"
#include "LyricsPartsCtrl.h"
#include "ListCtrl.h"
#include "AppIdentity.h"
#include "KarData.h"
#include "Visualization.h"
#include "FfmpegAudioCommandBuilder.h"
#include "FfmpegExportCommandBuilder.h"
#include "LyricsTransformer.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ProjectLoader.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "ProgressDlg.h"
#include "FfmpegProgressParser.h"
#include "SubtitleGenerator.h"
#include "SubtitleWrapProbeRunner.h"
#include "ExportDlg.h"

namespace {

int ReadDigitsBack(const String& text, int pos) {
    int start = pos - 1;
    while (start >= 0 && IsDigit(text[start]))
        start--;
    return start + 1 < pos ? StrInt(text.Mid(start + 1, pos - start - 1)) : 0;
}

int ReadDigitsForward(const String& text, int pos) {
    int end = pos;
    while (end < text.GetCount() && IsDigit(text[end]))
        end++;
    return end > pos ? StrInt(text.Mid(pos, end - pos)) : 0;
}

bool ParseVideoSize(const String& output, Size& size) {
    Vector<String> lines = Split(output, '\n');
    for (const auto& line : lines) {
        if (line.Find("Video:") < 0 && line.Find('x') < 0)
            continue;
        for (int i = 1; i + 1 < line.GetCount(); i++) {
            if (line[i] != 'x' || !IsDigit(line[i - 1]) || !IsDigit(line[i + 1]))
                continue;
            int cx = ReadDigitsBack(line, i);
            int cy = ReadDigitsForward(line, i + 1);
            if (cx >= 160 && cy >= 90) {
                size = Size(cx, cy);
                return true;
            }
        }
    }
    return false;
}

String FfprobePath(const String& ffmpegPath) {
    String name = GetFileName(ffmpegPath);
    if (name.IsEmpty())
        return "ffprobe";
    name.Replace("ffmpeg", "ffprobe");
    name.Replace("FFMPEG", "FFPROBE");
    String dir = GetFileDirectory(ffmpegPath);
    return dir.IsEmpty() ? name:AppendFileName(dir, name);
}

bool ReadVideoSizeWithFfprobe(const String& ffmpegPath, const String& videoPath, Size& size) {
    MediaProcessRunner process;
    Vector<String> args{
        "-v", "error",
        "-select_streams", "v:0",
        "-show_entries", "stream=width,height",
        "-of", "csv=s=x:p=0",
        videoPath
    };
    if (!process.Start(FfprobePath(ffmpegPath), args))
        return false;

    String chunk;
    String output;
    while (process.Read(chunk) || process.IsRunning()) {
        output.Cat(chunk);
        Sleep(10);
    }
    return process.GetExitCode() == 0 && ParseVideoSize(output, size);
}

bool ReadVideoSizeWithFfmpeg(const String& ffmpegPath, const String& videoPath, Size& size) {
    MediaProcessRunner process;
    if (!process.Start(ffmpegPath, Vector<String>{"-i", videoPath}))
        return false;

    String chunk;
    String output;
    while (process.Read(chunk) || process.IsRunning()) {
        output.Cat(chunk);
        Sleep(10);
    }
    return ParseVideoSize(output, size);
}

Size ProbeCanvasForExport(const KarData& data, const String& ffmpegPath) {
    if (data.videoFilePath.StartsWith("@@"))
        return Size(1920, 1080);

    Size videoSize;
    if (ReadVideoSizeWithFfprobe(ffmpegPath, data.videoFilePath, videoSize))
        return videoSize;
    if (ReadVideoSizeWithFfmpeg(ffmpegPath, data.videoFilePath, videoSize))
        return videoSize;
    return Size(1920, 1080);
}

int ProbeCropHeight(int resY) {
    return min(380, max(1, resY));
}

int ProbeCropY(int resY) {
    return max(0, resY - ProbeCropHeight(resY));
}

// The ASS is authored against a fixed reference height so the burned-in font
// size stays a constant fraction of the frame regardless of the source video's
// resolution. libass renders the script at the real output frame with full
// sharpness but scales positions/sizes uniformly, so a 4K, HD, or original-
// resolution export all show the subtitles at the same "just right" size that
// was tuned for 1080p. Width tracks the source aspect ratio (kept even).
constexpr int SubtitleReferenceHeight = 1080;

Size NormalizeSubtitleCanvas(Size canvas) {
    if (canvas.cx <= 0 || canvas.cy <= 0)
        return Size(1926, SubtitleReferenceHeight);
    int w = canvas.cx * SubtitleReferenceHeight / canvas.cy;
    w -= w % 2;
    return Size(max(2, w), SubtitleReferenceHeight);
}

}

ExportDlg::ExportDlg() : ffmpeg(Config::Get(FFMPEG_LOCATION)) {
    WhenClose << [=] {
        if (FileExists(assFilePath)) {
            FileDelete(assFilePath);
        }
    };
    
    WhenProcessAborted << [=] (bool byUser) {
        if (byUser && FileExists(outputPath)) {
            FileDelete(outputPath);
        }
    };
    
    WhenProcessEnded << [=] (int code) {
        if (code == 0) {
            if (phase == Dehiss) {
                phase = ExportVideo;
                progressVal = 0;
                renderTime = "";
            }
            else if (phase == ExportVideo) {
                phase = Finished;
                if (ExportIsLowRes(length)) {
                    PromptOK("Preview video exported successfully.");
                }
                else {
                    PromptOK("Video exported successfully.");
                }

            }
        }
        else {
            ErrorOK(Format("Child process result: %d. Unable to save video.", code));
        }
        return code != 0;
    };
    
    WhenProcessOutput << [=] (String output) {
        if (phase == Dehiss || phase == ExportVideo) {
            double tPos;
            if (FfmpegProgressParser::ParseTimestamp(output, tPos, renderTime)) {
                progressVal = (int)tPos;
            }
        }
    };
    
    WhenStartNextProcess << [=] {
        switch (phase) {
        case SaveASS:
            ExportASS();
            break;
        case Dehiss:
            DehissAudio();
            break;
        case ExportVideo:
            if (data->videoFilePath.StartsWith("@@")) ExportViz();
            else ExportBg();
            break;
        case GenerateThumbnail:
            GenThumbnail();
            break;
        case Finished:
            Break(IDOK);
            Close();
            break;
        }
    };
    
    WhenUpdateProgress << [=] {
        int previewSeconds = ExportPreviewSeconds(length);
        progress.Set(progressVal, phase == ExportVideo && previewSeconds > 0 ? previewSeconds:(int)data->duration);
        static const char* captions[]{"subtitles...", "audio...", "video...", "thumbnail...", "Done."};
        monitor.SetLabel(Format("%s %s %s", phase == Finished ? "":"Exporting",
                                captions[(int)phase],
                                renderTime));
    };
    
    WhenWarnOnClose = [=] { return phase != Finished; };
}

int ExportDlg::Run(const KarData& karData, String outputPath, ExportQuality len, double thumbnailTS) {
    length = len;
    this->thumbnailTS = thumbnailTS;
    data = &karData;
    this->outputPath = outputPath;
    dehissedAudioFilepath = karData.dehiss ? AppIdentity::TaggedTempFileName("dehissed", ".ogg"):"";
    PostCallback([=] { StartNextProcess(); });
    return RunDlg("Export Video");
}

void ExportDlg::ExportASS() {
    UpdateProgress();
    assFilePath = AppIdentity::TempFileName(".ass");
    Vector<bool> wrappedHighlights;
    Vector<bool> wrappedIncoming;
    Vector<String> probeLyrics = SubtitleGenerator::HighlightProbeLyrics(*data, data->subtitleLines);
    Size probeCanvas = ProbeCanvasForExport(*data, ffmpeg);
    probeCanvas = NormalizeSubtitleCanvas(probeCanvas);
    int probeCropY = ProbeCropY(probeCanvas.cy);
    int probeCropHeight = ProbeCropHeight(probeCanvas.cy);
    Vector<SubtitleWrapProbeFrame> probeFrames;
    if (SubtitleWrapProbeRunner::Run(*data, probeLyrics, probeFrames, ffmpeg,
                                     probeCanvas.cx, probeCanvas.cy, probeCropY, probeCropHeight)) {
        for (const auto& frame : probeFrames)
            wrappedHighlights.Add(SubtitleWrapProbe::IsWrappedFrame(frame, data->fontSize));
    }
    int incomingFontSize = max(1, (int)(data->fontSize * 0.7));
    Vector<SubtitleWrapProbeFrame> incomingProbeFrames;
    if (SubtitleWrapProbeRunner::Run(*data, probeLyrics, incomingProbeFrames, ffmpeg,
                                     probeCanvas.cx, probeCanvas.cy, probeCropY, probeCropHeight, incomingFontSize, false)) {
        for (const auto& frame : incomingProbeFrames)
            wrappedIncoming.Add(SubtitleWrapProbe::IsWrappedFrame(frame, incomingFontSize));
    }
    SaveFile(assFilePath, SubtitleGenerator::ToAss(*data, wrappedHighlights, wrappedIncoming,
                                                   data->subtitleLines, probeCanvas.cx, probeCanvas.cy));
    SetTimeCallback(500, [=] {
        phase = Dehiss;
        StartNextProcess();
    }, timerId);
}

void ExportDlg::DehissAudio() {
    if (!dehissedAudioFilepath.IsEmpty()) {
        args = FfmpegAudioCommandBuilder::Dehiss(data->audioFilePath, dehissedAudioFilepath);
        StartExport();
    }
    else {
        UpdateProgress();
        SetTimeCallback(500, [=] {
            phase = ExportVideo;
            StartNextProcess();
        }, timerId);
    }
}

void ExportDlg::ExportViz() {
    int canvasHeight = ExportScaleHeight(length);
    if (canvasHeight <= 0) canvasHeight = 1080;   // a visualization has no source resolution
    args = FfmpegExportCommandBuilder::WithVisualization(*data, assFilePath, outputPath, dehissedAudioFilepath, canvasHeight);
    if (args.IsEmpty()) {
        ErrorOK("Unknown visualization type. Unable to save video.");
        phase = Finished;
        Break(IDABORT);
        Close();
        Hide();
        return;
    }
    
    if (FileExists(outputPath) && !FileDelete(outputPath)) {
        ErrorOK(Format("Unable to save to %s", outputPath));
        phase = Finished;
        Break(IDABORT);
        Close();
        Hide();
        return;
    }
    
    int previewSeconds = ExportPreviewSeconds(length);
    if (previewSeconds > 0) {
        args.Insert(11, IntStr(previewSeconds));
        args.Insert(11, "-t");
    }

    StartExport();
}

void ExportDlg::ExportBg() {
    if (FileExists(outputPath) && !FileDelete(outputPath)) {
        ErrorOK(Format("Unable to save to %s", outputPath));
        phase = Finished;
        Break(IDCANCEL);
        Close();
        Hide();
    }
    
    args = FfmpegExportCommandBuilder::WithBackgroundVideo(*data, assFilePath, outputPath, dehissedAudioFilepath, ExportScaleHeight(length));

    int previewSeconds = ExportPreviewSeconds(length);
    if (previewSeconds > 0) {
        args.Insert(15, IntStr(previewSeconds));
        args.Insert(15, "-t");
    }

    StartExport();
}

void ExportDlg::StartExport() {
    UpdateProgress();
    SetTimeCallback(500, [=] {
        bool res = process.Start(ffmpeg, args, nullptr, nullptr);
        if (!res) {
            ErrorOK("Unable to start child process. Unable to save video.");
            phase = Finished;
            Break(IDABORT);
        }
        else {
            PollProgress();
        }
    }, timerId);
}

void ExportDlg::GenThumbnail() {
    String tnPath = outputPath;
    tnPath.Replace(".mp4", ".cover.png");
    if (FileExists(tnPath) && !FileDelete(tnPath)) {
        ErrorOK("Unable to generate thumbnail.");
        phase = Finished;
        Break(IDCANCEL);
        Close();
        Hide();
    }
    auto ts = thumbnailTS < 0.0f && data->timedLyrics.GetCount() > 1 ?
                        min<double>(data->timedLyrics[1].time + 1.0f, data->duration - 1.0f):thumbnailTS;
    auto vs = FfmpegExportCommandBuilder::GenerateCoverImage(outputPath, tnPath, max<double>(ts, 0.0f));
    if (!FileExists(tnPath) || FileDelete(tnPath)) {
        bool res = process.Start(ffmpeg, vs);
        if (!res) {
            Exclamation("Unable to generate thumbnail!");
            phase = Finished;
            Break(IDOK);
        }
        else {
            PollProgress();
        }
    }
}
