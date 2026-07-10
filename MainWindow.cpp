/*
 * File  : MainWindow.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

#include <ctime>
#include <filesystem>

using namespace Upp;

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_header.h>

#include "Constants.h"
#include "AppIdentity.h"
#include "AppPaths.h"
#include "ConfigService.h"
#include "Config.h"
#include "KarData.h"
#include "LyricsTransformer.h"
#include "SubtitleGenerator.h"
#include "TimeFormatter.h"
#include "UiScaler.h"
#include "GenreCatalog.h"
#include "RichTextBuilder.h"
#include "TextTools.h"
#include "Visualization.h"
#include "FfmpegProgressParser.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ListCtrl.h"
#include "LyricsPartsCtrl.h"
#include "ProjectLoader.h"
#include "Page.h"
#include "LyricsDownloadService.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "ProgressDlg.h"
#include "ConvertDlg.h"
#include "OpenProjectDlg.h"
#include "SaveProjectDlg.h"
#include "ExportDlg.h"
#include "LyricsPartsDlg.h"
#include "TimingLine.h"
#include "TimingCtrl.h"

#define LAYOUTFILE <Croon/CroonTimingDlg.lay>
#include <CtrlCore/lay.h>

#include "AppAudioPlayer.h"
#include "TimingDlg.h"
#include "GatherDlg.h"
#include "VidThumbnail.h"
#include "Page1.h"
#include "Page2.h"
#include "Page3.h"

#define LAYOUTFILE <Croon/CroonWizardShell.lay>
#include <CtrlCore/lay.h>

#define LAYOUTFILE <Croon/CroonVideoDlg.lay>
#include <CtrlCore/lay.h>

#include "VideoDlg.h"
#include "WizardDlg.h"
#include "Project.h"
#include "ProjectList.h"

#define LAYOUTFILE <Croon/CroonMainWindow.lay>
#include <CtrlCore/lay.h>

#include "SettingsDlg.h"
#include "MainWindow.h"

MainWindow::MainWindow(KarData& data) : videoDlg(data), wizardDlg(data), project(data, videoDlg), projects(data, wizardDlg) {
    CtrlLayout(*this);
    Add(projects.HSizePos().VSizePos());
    Add(project.HSizePos().VSizePos());
    Title(AppIdentity::ProductName()).Sizeable().Zoomable().SetMinSize(Size(UiScaler::X(640), UiScaler::Y(490)));
    int x{Config::GetInt(WIN_X, -9999)},
        y{Config::GetInt(WIN_Y, -9999)},
        w{Config::GetInt(WIN_W, 2048)},
        h{Config::GetInt(WIN_H, 2048)};
    bool center = x == -9999 || y == -9999;
    SetRect(center ? 0:x, center ? 0:y, w, h);
    if (center) CenterScreen();
    
    AddFrame(status);
    status = "No project loaded.";
    
    project.WhenProjectSaved << [this] { status = "Project saved."; };
    project.WhenDirty << [this](bool dirty) { if (dirty) status = ""; };
    
    projects.WhenOpeningProject << [this] {
        if (project.IsProjectDirty()) {
            auto res = PromptYesNoCancel("Save open project first?");
            if (res == 1) {
                project.SaveProject();
                return true;
            }
            return res == 0;
        }
        return true;
    };
    
    projects.WhenLoadingProject << [this] {
        project.CloseProject(false);
    };
    
    projects.WhenProjectLoaded << [this] {
        project.Populate();
        ShowProject();
        status = "Project loaded.";
    };
    
    HideProject();
    
#ifndef PLATFORM_COCOA
    AddFrame(menuBar);
#endif

    SetTheMainMenu();
}

MainWindow::~MainWindow() {
    Config::Set(WIN_X, GetRect().left);
    Config::Set(WIN_Y, GetRect().top);
    Config::Set(WIN_W, GetSize().cx);
    Config::Set(WIN_H, GetSize().cy);
}

void MainWindow::SetTheMainMenu() {
    auto menuFn = [this] (Bar& menu) {
            menu.Sub(AppIdentity::ProductName(), [this](Bar& bar) {
                bar.Add("New...", CtrlImg::new_doc(), [this] {
                    project.SaveProject();
                    projects.NewProject();
                }).Key(K_CTRL_N);
                bar.Add("Open...", CtrlImg::open(), [this] {
                    projects.OpenProject();
                }).Key(K_CTRL_O);
                bar.Add("Save", CtrlImg::save(), [this] {
                    project.SaveProject();
                }).Key(K_CTRL_S).Enable(project.IsProjectDirty());
                bar.Add("Save As...", CtrlImg::save_as(), [this] {
                    project.SaveProjectAs();
                }).Key(K_CTRL_A).Enable(project.IsProjectOpen());
                bar.Add("Close", CroonImg::Close(), [this] {
                    if (project.CloseProject(false)) HideProject();
                }).Key(K_CTRL_W).Enable(project.IsProjectOpen());
                bar.Separator();
                bar.Add("Settings...", CroonImg::Settings(), [] {
                    SettingsDlg sDlg;
                    sDlg.Execute();
                });
                bar.Separator();
                bar.Add(Format("Quit %s", AppIdentity::ProductName()), [this] {
                    Close();
                }).Key(K_CTRL_Q);
            });
            project.MainMenu()(menu);
            menu.Sub("Help", [](Bar& bar) {
                bar.Add("Help", []{
                    PromptOK("Help is on the way...");
                }).Key(K_CTRL_H);
                bar.Separator();
                bar.Add("About...", CroonImg::Info(),  []{
                    PromptOK(AppIdentity::VersionText());
                });
            });
        };
#ifdef PLATFORM_COCOA
    SetMainMenu(menuFn);
#else
    //AddFrame(menuBar);
    menuBar.Set(menuFn);
#endif
}
