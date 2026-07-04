/*
 * File  : ProgressDlg.cpp
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
#include "FfmpegCommandBuilder.h"
#include "LyricsTransformer.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ProjectLoader.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "ProgressDlg.h"

ProgressDlg::ProgressDlg() {
    CtrlLayout(*this, "Progress");
    NoSizeable().NoZoomable().NoCloseBox();
    CenterOwner();
    cancelBtn.Cancel();
    cancelBtn << [=] { Close(); };
    WhenAbortingProcess << [=] (bool byUser) {
        if (byUser && process.IsRunning()) {
            process.Kill();
        }
    };
    WhenWarnOnClose << [=] { return true; };
}

void ProgressDlg::Close() {
    if (WhenWarnOnClose()) {
        if (PromptYesNo("Abort operation?") == 1) {
            KillTimeCallback(timerId);
            WhenAbortingProcess(true);
            ProcessAbortedByUser();
            Break(IDABORT);
            TopWindow::Close();
            Hide();
            PromptOK("Operation cancelled.");
        }
    }
    else TopWindow::Close();
}

void ProgressDlg::PollProgress() {
    SetTimeCallback(100, [=] {
        String output;
        if (process.Read(output)) {
            ProcessOutput(output);
            UpdateProgress();
            PollProgress();
        }
        else if (process.IsRunning()){
            PollProgress();
        }
        else {
            int code = process.GetExitCode();
            bool abort = ProcessEnded(code);
            if (abort) {
                ProcessAborted();
                Break(IDABORT);
                TopWindow::Close();
                Hide();
                return;
            }
            UpdateProgress();
            SetTimeCallback(100, [=] {
                StartNextProcess();
            }, timerId);
        }
    }, timerId);
}
