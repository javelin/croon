#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_header.h>

#include "AppIdentity.h"
#include "ConfigService.h"
#include "Config.h"
#include "KarData.h"
#include "Visualization.h"
#include "FfmpegCommandBuilder.h"
#include "LyricsTransformer.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ProjectLoader.h"

void ProjectLoader::LoadProjects() {
    projects = RecentProjectService::LoadPaths();
    curPath = 0;
    infoFilePath = AppIdentity::TempFileName(".json");
    thumbnailPath = AppIdentity::TempFileName(".png");
    if (!projects.IsEmpty()) {
        SetTimeCallback(100, [=] {
            StartNextProcess();
        }, timerId);
    }
    else {
        WhenDoneLoading();
    }
}

void ProjectLoader::PollProgress() {
    SetTimeCallback(100, [=] {
        String _;
        if (process.Read(_)) {
            //std::cout << _.ToStd();
            PollProgress();
        }
        else if (process.IsRunning()){
            PollProgress();
        }
        else {
            int code = process.GetExitCode();
            //std::cout << "Exitcode: " << code << std::endl;
            if (code == 0) {
                // Process the project
                KarData data{LoadFile(infoFilePath)};
                auto thumbnail = StreamRaster::LoadFileAny(thumbnailPath);
                WhenProjectLoaded(projectPath,
                                    data.title,
                                    data.artist,
                                    LyricsTransformer::TimedToRaw(data.timedLyrics, true),
                                    thumbnail);
                FileDelete(infoFilePath);
                FileDelete(thumbnailPath);
            }
            Set(++curPath, projects.GetCount());
            if (curPath == projects.GetCount()) {
                WhenDoneLoading();
            }
            else {
                SetTimeCallback(100, [=] {
                    StartNextProcess();
                }, timerId);
            }
        }
    }, timerId);
}

void ProjectLoader::StartNextProcess() {
    while (true) {
        projectPath = TrimBoth(projects[curPath]);
        if (projectPath.IsEmpty()) {
            Set(++curPath, projects.GetCount());
            if (curPath == projects.GetCount()) {
                WhenDoneLoading();
                return;
            }
            else continue;
        }
        else {
            if (FileExists(infoFilePath)) FileDelete(infoFilePath);
            if (FileExists(thumbnailPath)) FileDelete(thumbnailPath);
            auto res = process.Start(ffmpeg,
                            FfmpegCommandBuilder::DumpAttachmentAndGenerateThumbnail(
                                projectPath,
                                infoFilePath,
                                thumbnailPath,
                                100,
                                100
                            ));
            if (res) {
                PollProgress();
                return;
            }
            else {
                Set(++curPath, projects.GetCount());
                if (curPath == projects.GetCount()) {
                    WhenDoneLoading();
                    return;
                }
                else continue;
            }
        }
    }
}
