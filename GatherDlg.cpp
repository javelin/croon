/*
 * File  : GatherDlg.cpp
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
#include "LyricsTransformer.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ProjectLoader.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "ProgressDlg.h"
#include "VideoCatalog.h"
#include "TextTools.h"
#include "GatherDlg.h"

GatherDlg::GatherDlg() {
    ffmpeg = Config::Get(FFMPEG_LOCATION);
    WhenProcessEnded << [=] (int code) {
        if (code == 0) {
            String tnPath = VideoCatalog::ThumbnailPath(paths[curPath]);
            Image image = VideoCatalog::LoadThumbnail(paths[curPath]);
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
        
        String tnPath = VideoCatalog::ThumbnailPath(paths[curPath]);
        bool existing = VideoCatalog::HasThumbnail(paths[curPath]);
        auto fn = [=]() {
            Image image = VideoCatalog::LoadThumbnail(paths[curPath]);
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
                if (!VideoCatalog::DeleteThumbnail(paths[curPath])) {
                    if (fn()) return;
                    ++curPath;
                    UpdateProgress();
                }
            }
            else {
                if (fn()) return;
                if (!VideoCatalog::DeleteThumbnail(paths[curPath])) {
                    ++curPath;
                    progress.Set(curPath, paths.GetCount());
                    UpdateProgress();
                }
            }
        }
        
        procArgs = VideoCatalog::BuildThumbnailCommand(paths[curPath]);
        
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
            monitor.SetLabel(Format("Adding %s", TextTools::ShortenMiddle(paths[curPath], 60)));
        }
    };
}

int GatherDlg::Run(String videoDir) {
    paths = VideoCatalog::FindVideoFiles(videoDir);
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
