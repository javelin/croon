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
#include "LyricsTransformer.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ProjectLoader.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "ProgressDlg.h"
#include "DownloadDefaults.h"
#include "DownloadDlg.h"

struct DownloadDlg::RequestState {
    HttpRequest request;
};

DownloadDlg::DownloadDlg() : request(new RequestState) {
    WhenAbortingProcess = [=] (bool byUser) {
        if (byUser && Request().request.IsOpen()) {
            Request().request.Abort();
        }
    };
    WhenUpdateProgress << [=] {
        static int phase = -1;
        if (phase < Request().request.GetPhase()) phase = Request().request.GetPhase();
        progress.Set(phase, HttpRequest::FINISHED - HttpRequest::BEGIN);
        monitor.SetLabel(Request().request.GetPhaseName());
    };
}

DownloadDlg::~DownloadDlg() = default;

DownloadDlg::RequestState& DownloadDlg::Request() {
    return *request;
}

const DownloadDlg::RequestState& DownloadDlg::Request() const {
    return *request;
}

String DownloadDlg::GetContent() const {
    return Request().request.GetContent();
}

int DownloadDlg::Run(String url, String title, String userAgent) {
    Request().request.Url(url).UserAgent(userAgent).New();
    SetTimeCallback(80, [=] {
        PollProgress();
    }, timerId);
    return RunDlg(title);
}

void DownloadDlg::PollProgress() {
    SetTimeCallback(20, [=] {
        if (Request().request.Do()) {
            UpdateProgress();
            PollProgress();
        }
        else {
            int code = Request().request.GetStatusCode();
            ProcessEnded(code);
            if (Request().request.IsSuccess()) {
                WhenDownloadSuccess(Request().request.GetContent());
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
