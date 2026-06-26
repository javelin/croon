/*
 * File  : GatherDlg.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

GatherDlg::GatherDlg() {
    ffmpeg = Config::Get(FFMPEG_LOCATION);
    WhenProcessEnded << [=] (int code) {
        if (code == 0) {
            String tnPath = paths[curPath];
            tnPath.Replace(".mp4", ".thumbnail.png");
            Image image = StreamRaster::LoadFileAny(tnPath);
            if (image) {
                images.Add(image);
                WhenVideoAdded(images.GetCount() - 1, paths[curPath], tnPath, image);
            }
            ++curPath;
        }
        return false;
    };
    WhenStartNextProcess << [=] {
        if (curPath >= paths.GetCount()) {
            PromptOK(DeQtf("No more videos."));
            Break(IDOK);
            TopWindow::Close();
            Hide();
            return;
        }
        
        String tnPath = AppendFileName(GetDataDirectory(), GetFileName(paths[curPath]));
        tnPath.Replace(".mp4", ".thumbnail.png");
        bool existing = FileExists(tnPath);
        auto fn = [=]() {
            Image image = StreamRaster::LoadFileAny(tnPath);
            if (image) {
                images.Add(image);
                WhenVideoAdded(images.GetCount() - 1, paths[curPath], tnPath, image);
                
                ++curPath;
                UpdateProgress();
                SetTimeCallback(100, [=] {
                    StartNextProcess();
                }, timerId);
            }
            return (bool)image;
        };
        
        if (existing) {
            if (overwriteTN) {
                if (!FileDelete(tnPath)) {
                    if (fn()) return;
                    ++curPath;
                    UpdateProgress();
                }
            }
            else {
                if (fn()) return;
                if (!FileDelete(tnPath)) {
                    ++curPath;
                    progress.Set(curPath, paths.GetCount());
                    UpdateProgress();
                }
            }
        }
        
        procArgs = FfmpegCommandBuilder::GenerateThumbnail(paths[curPath], tnPath, ThumbnailDim, ThumbnailDim);
        
        bool res = process.Start(ffmpeg, procArgs, nullptr, nullptr);
        if (!res) {
            ++curPath;
            UpdateProgress();
            SetTimeCallback(100, [=] {
                StartNextProcess();
            }, timerId);
        }
        else {
            PollProgress();
        }
    };
    WhenUpdateProgress << [=] {
        progress.Set(curPath, paths.GetCount());
        if (curPath >= paths.GetCount()) {
            monitor.SetLabel("");
        }
        else {
            monitor.SetLabel(Format("Adding %s", ShortenMiddle(paths[curPath], 60)));
        }
    };
}

int GatherDlg::Run(String videoDir) {
    paths = GetPaths(videoDir, "*.mp4");
    if (paths.IsEmpty()) {
        PromptOK("No videos found.");
    }
    else {
        auto res = PromptYesNoCancel(Format("Found %d videos. Would you like to "
                                        "keep all existing thumbnails?", paths.GetCount()));
        overwriteTN = res != 1;
        if (res == -1) {
            PromptOK("Operation cancelled.");
            return IDCANCEL;
        }
        images.Clear();
        curPath = 0;
        UpdateProgress();
        SetTimeCallback(500, [=] {
            StartNextProcess();
        }, timerId);
        return RunDlg("Discover Videos");
    }
    return IDCANCEL;
}

