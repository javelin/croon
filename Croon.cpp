/*
 * File  : Croon.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_source.h>

GatherDlg& GetGatherDlg() {
    static GatherDlg* dlg = new GatherDlg();
    return *dlg;
}

VideoDlg& GetVideoDlg() {
    static VideoDlg* dlg = new VideoDlg();
    return *dlg;
}

WizardDlg& GetWizardDlg() {
    static WizardDlg* dlg = new WizardDlg();
    return *dlg;
}

void RunCroon() {
    auto ffmpegLoc = Config::Get(FFMPEG_LOCATION);
    if (ffmpegLoc.IsVoid() || ffmpegLoc.IsEmpty()) ffmpegLoc = "ffmpeg";
    MediaProcessRunner proc;
    if (!proc.Start(ffmpegLoc)) {
        if (PromptYesNoCancel("Unable to validate ffmpeg executable. Manually look for ffmpeg?") != 1) {
            Exclamation("Croon needs ffmpeg to operate. Exiting application.");
            return;
        }
        FileSel fsel;
        fsel <<= ffmpegLoc;
        if (!fsel.ExecuteOpen("Find Ffmpeg Executable...")) {
            Exclamation("Croon needs ffmpeg to operate. Exiting application.");
            return;
        }
        ffmpegLoc = ~fsel;
        Config::Set(FFMPEG_LOCATION, ffmpegLoc);
    }
    
    MusicPlayer::InitPlayer();
    
    (void)GetVideoDlg();
    (void)GetWizardDlg();
    MainWindow().Run();
    
    MusicPlayer::DeInitPlayer();
}
