/*
 * File  : SaveProjectDlg.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

#include <filesystem>

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
#include "FfmpegCommandBuilder.h"
#include "LyricsTransformer.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ProjectLoader.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "ProgressDlg.h"
#include "SaveProjectDlg.h"

SaveProjectDlg::SaveProjectDlg() {
    ffmpeg = Config::Get(FFMPEG_LOCATION);
    SetRect(0, 0, UiScaler::X(450), UiScaler::Y(70));
    CenterOwner();
    cancelBtn.Disable();
    cancelBtn.Hide();
    
    WhenProcessEnded << [=] (int code) {
        if (code == 0) {
            progressVal = 100;
            try {
                std::filesystem::copy(tempFilename.ToStd(), data->projectPath.ToStd(), std::filesystem::copy_options::overwrite_existing);
                FileDelete(tempFilename);
            } catch (const std::filesystem::filesystem_error& e) {
                ErrorOK("Unable to write to project file.");
            }
        }
        else {
            ErrorOK(Format("Child process result: %d. Unable to save project.", code));
        }
        return code != 0;
    };
    
    WhenProcessOutput << [=] (auto _) {
        progressVal = 50;
    };
    
    WhenStartNextProcess << [=] {
        Break(IDOK);
    };
    
    WhenUpdateProgress << [=] {
        progress.Set(progressVal, 100);
        monitor.SetLabel("Saving project...");
    };
}

int SaveProjectDlg::Run(String savePath, KarData& karData) {
    data = &karData;
    data->projectPath = savePath;
    progressVal = 0;
    PostCallback([=]{
        if (FileExists(data->projectPath) && !FileDelete(data->projectPath)) {
            ErrorOK("Unable to save project!");
            Break(IDCANCEL);
            TopWindow::Close();
            Hide();
        }
        StartSave();
    });
    return RunDlg("Create Project");
}

void SaveProjectDlg::StartSave() {
    tempFilename = AppIdentity::ProjectTempFileName();
    SaveFile(data->infoFilePath, data->ToJSONStr());
    
    auto res = process.Start(ffmpeg, data->videoFilePath.StartsWith("@@") ?
                            FfmpegCommandBuilder::ProjectSaveWithVisualization(*data, tempFilename) :
                            FfmpegCommandBuilder::ProjectSaveWithBackgroundVideo(*data, tempFilename));
    if (!res) {
        Exclamation("Unable to save project!");
        Break(IDOK);
    }
    else {
        PollProgress();
    }
}
