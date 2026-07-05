/*
 * File  : DownloadDlg.cpp
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
#include "DownloadDefaults.h"
#include "DownloadDlg.h"

DownloadDlg::DownloadDlg() {
    WhenAbortingProcess = [=] (bool byUser) {
        if (byUser && request.IsOpen()) {
            request.Abort();
        }
    };
    WhenUpdateProgress << [=] {
        static int phase = -1;
        if (phase < request.GetPhase()) phase = request.GetPhase();
        progress.Set(phase, HttpRequest::FINISHED - HttpRequest::BEGIN);
        monitor.SetLabel(request.GetPhaseName());
    };
}

int DownloadDlg::Run(String url, String title, String userAgent) {
    request.Url(url).UserAgent(userAgent).New();
    SetTimeCallback(80, [=] {
        PollProgress();
    }, timerId);
    return RunDlg(title);
}

void DownloadDlg::PollProgress() {
    SetTimeCallback(20, [=] {
        if (request.Do()) {
            UpdateProgress();
            PollProgress();
        }
        else {
            int code = request.GetStatusCode();
            ProcessEnded(code);
            if (request.IsSuccess()) {
                WhenDownloadSuccess(request.GetContent());
                Break(IDOK);
            }
            else {
                Break(IDCANCEL);
            }
            TopWindow::Close();
            Hide();
        }
    }, timerId);
}
