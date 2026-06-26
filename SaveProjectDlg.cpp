/*
 * File  : SaveProjectDlg.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

SaveProjectDlg::SaveProjectDlg() {
    ffmpeg = Config::Get(FFMPEG_LOCATION);
    SetRect(0, 0, Zx(450), Zy(70));
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
                            Ffmpeg::ProjectSaveWithVisualization(*data, tempFilename) :
                            Ffmpeg::ProjectSaveWithBackgroundVideo(*data, tempFilename));
    if (!res) {
        Exclamation("Unable to save project!");
        Break(IDOK);
    }
    else {
        PollProgress();
    }
}
