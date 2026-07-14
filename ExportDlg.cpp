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
#include "ExportDlg.h"

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
                if (length != 1) {
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
        progress.Set(progressVal, phase == ExportVideo && length > 1 ? length:(int)data->duration);
        static const char* captions[]{"subtitles...", "audio...", "video...", "thumbnail...", "Done."};
        monitor.SetLabel(Format("%s %s %s", phase == Finished ? "":"Exporting",
                                captions[(int)phase],
                                renderTime));
    };
    
    WhenWarnOnClose = [=] { return phase != Finished; };
}

int ExportDlg::Run(const KarData& karData, String outputPath, int len, double thumbnailTS) {
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
    //SaveFile(assFilePath, SubtitleGenerator::ToAss(*data, Config::GetFontSize(), 4));
    SaveFile(assFilePath, SubtitleGenerator::ToAss(*data, 4));
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
    args = FfmpegExportCommandBuilder::WithVisualization(*data, assFilePath, outputPath, dehissedAudioFilepath, length != 1);
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
    
    if (length > 1) {
        args.Insert(11, IntStr(length));
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
    
    args = FfmpegExportCommandBuilder::WithBackgroundVideo(*data, assFilePath, outputPath, dehissedAudioFilepath, length != 1);
    
    if (length > 1) {
        args.Insert(15, IntStr(length));
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
    runCode = ProgressDlg::Run("Export Video");
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
