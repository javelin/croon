/*
 * File  : OpenProjectDlg.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

#define _PATTERN_1 "Input #0.+?("
#define _PATTERN_2 ")\\s*?:\\s*(.+?)\n"
#define METADATA_PATTERN(MD) _PATTERN_1 MD _PATTERN_2

OpenProjectDlg::OpenProjectDlg() : phase(Audio) {
    ffmpeg = Config::Get(FFMPEG_LOCATION);
    SetRect(0, 0, Zx(450), Zy(70));
    CenterOwner();
    cancelBtn.Disable();
    cancelBtn.Hide();
    
    WhenProcessEnded << [=] (int code) {
        if (code == 0) {
            if (phase == Audio) {
                KarData temp{LoadFile(data->infoFilePath)};
                temp.projectPath = data->projectPath;
                temp.audioFilePath = data->audioFilePath;
                temp.infoFilePath = data->infoFilePath;
                temp.rawLyrics = TimedLyricsToRaw(temp.timedLyrics);
                *data = pick(temp);
                if (data->origVideoFile.StartsWith("@@")) {
                    data->videoFilePath = data->origVideoFile;
                    data->videoThumbnail = VIZ::Thumbnail(data->origVideoFile);
                    phase = Finished;
                    return false;
                }
            }
            else if (phase == Thumbnail) {
                data->videoThumbnail = StreamRaster::LoadFileAny(data->thumbnailFilePath);
            }
            ++phase;
        }
        else {
            ErrorOK(Format("Unable to load %s. Child process returned %d. Failed to open project!", Phase(), code));
        }
        return code != 0;
    };
    
    WhenStartNextProcess << [=] {
        switch (phase) {
        case Audio:
            ExtractAudio();
            break;
        case Video:
            ExtractVideo();
            break;
        case Thumbnail:
            LoadThumbnail();
            break;
        case Finished:
            if (data->timed >= data->timedLyrics.GetCount()) {
                Exclamation(DeQtfLf("Warning!\n"
                                "This project's timed lines is more than the actual number of \n"
                                "lines in the lyrics. You may need to retime your project."));
                data->timed = data->timedLyrics.GetCount() - 1;
            }
            Break(IDOK);
        };
    };
    
    WhenUpdateProgress << [=] {
        progress.Set(phase, Finished);
        monitor.SetLabel(Format("Loading %s...", Phase()));
    };
}

int OpenProjectDlg::Run(String projectPath, KarData& karData) {
    data = &karData;
    data->projectPath = projectPath;
    PostCallback([=]{ StartNextProcess(); });
    return RunDlg("Open Project");
}

void OpenProjectDlg::ExtractAudio() {
    data->infoFilePath = AppIdentity::TempFileName(".json");
    data->audioFilePath = AppIdentity::TempFileName(".ogg");
    auto vs = Ffmpeg::ProjectExtractAudioAndInfo(data->projectPath,
                                                    data->audioFilePath,
                                                    data->infoFilePath);
    bool res = process.Start(ffmpeg, vs);
    if (!res) {
        Exclamation(Format("Unable to load %s. Failed to open project!", Phase()));
        Break(IDCANCEL);
    }
    else {
        PollProgress();
    }
}

void OpenProjectDlg::ExtractVideo() {
    data->videoFilePath = AppIdentity::TempFileName(".mp4");
    auto vs = Ffmpeg::ProjectExtractVideo(data->projectPath, data->videoFilePath);
    bool res = process.Start(ffmpeg, vs);
    if (!res) {
        Exclamation(Format("Unable to load %s. Failed to open project!", Phase()));
        Break(IDCANCEL);
    }
    else {
        PollProgress();
    }
}

void OpenProjectDlg::LoadThumbnail() {
    data->thumbnailFilePath = AppIdentity::TempFileName(".png");
    auto vs = Ffmpeg::GenerateThumbnail(data->videoFilePath, data->thumbnailFilePath,
                                        ThumbnailDim, ThumbnailDim);
    bool res = process.Start(ffmpeg, vs);
    if (!res) {
        Exclamation(Format("Unable to load %s. Failed to open project!", Phase()));
        Break(IDCANCEL);
    }
    else {
        PollProgress();
    }
}

String OpenProjectDlg::Phase() const {
    switch (phase) {
    case Audio: return "audio, metadata and subtitles"; break;
    case Video: return "video"; break;
    case Thumbnail: return "thumbnail"; break;
    }
    return "";
}
