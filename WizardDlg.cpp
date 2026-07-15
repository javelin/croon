/*
 * File  : WizardDlg.cpp
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
#include "AppPaths.h"
#include "KarData.h"
#include "Visualization.h"
#include "LyricsTransformer.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ProjectLoader.h"
#include "GenreCatalog.h"
#include "LyricsDownloadService.h"
#include "TextTools.h"
#include "Page.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "ProgressDlg.h"
#include "SaveProjectDlg.h"
#include "VidThumbnail.h"
#include "Page1.h"
#include "Page2.h"
#include "Page3.h"

#define LAYOUTFILE <Croon/CroonWizardShell.lay>
#include <CtrlCore/lay.h>

#include "WizardDlg.h"

WizardDlg::WizardDlg(KarData& data) : data(data), page1(data), page2(data), page3(data), pages{&page1, &page2, &page3}, currPage(0) {
    CtrlLayout(*this, "Create Project");
    NoZoomable().Sizeable();
    CenterScreen();
    cancelBtn.Cancel();
    pageName.SetLabel(page1.GetPageName());
    
    for (int i = 0; i < maxPages; ++i) {
        auto page = pages[i];
        Add(page->HSizePosZ(5, 5).VSizePosZ(32, 45));
        *this << page->GetPrevButton().LeftPosZ(5, 100).TopPosZ(5, 25)
            << page->GetNextButton().RightPosZ(5, 100).TopPosZ(5, 25);
        page->Hide();
        page->Disable();
        if (i > 0) {
            page->WhenPreviousPage << [=] {
                page->HidePage();
                auto dest = pages[(currPage = i - 1)];
                dest->ShowPage();
                pageName.SetLabel(dest->GetPageName());
            };
        }
        if (i < maxPages - 1) {
            page->WhenNextPage << [=] {
                page->HidePage();
                auto dest = pages[(currPage = i + 1)];
                dest->ShowPage();
                pageName.SetLabel(dest->GetPageName());
            };
        }
    }
    
    page3.WhenProjectSaved << [=] {
        Break(IDOK);
        TopWindow::Close();
    };
    
    *this << page3.GatherButton().SetLabel("Find Videos").LeftPosZ(5, 100).BottomPosZ(15, 25);
    cancelBtn << [=] {
        if (PromptYesNoCancel("This action will abandon all progress. Are you sure?") == 1) {
            Break(IDCANCEL);
            TopWindow::Close();
        }
    };
}

void WizardDlg::Close() {
    if (PromptYesNoCancel("Closing this window will abandon all progress. Are you sure?") == 1) {
        Break(IDCANCEL);
        TopWindow::Close();
    }
}

int WizardDlg::Run(String tempAudioPath, double duration, String origAudioFilePath) {
    data.Reset();
    data.audioFilePath = tempAudioPath;
    data.origAudioFilePath = origAudioFilePath;
    data.duration = duration;
    CenterOwner();
    for (auto page : pages) {
        page->Reset();
        page->HideButtons();
        page->Disable();
        page->Hide();
    }
    page1.ShowPage();
    SetTimeCallback(500, [=] { page3.Rehint(false); });
    int code = TopWindow::Run();
    page3.StopGathering();
    TopWindow::Close();
    return code;
}
