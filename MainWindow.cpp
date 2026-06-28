/*
 * File  : MainWindow.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

MainWindow::MainWindow() {
    CtrlLayout(*this);
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
    
    project.WhenProjectSaved << [=] { status = "Project saved."; };
    project.WhenDirty << [=](bool dirty) { if (dirty) status = ""; };
    
    projects.WhenOpeningProject << [=] {
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
    
    projects.WhenLoadingProject << [=] {
        project.CloseProject(false);
    };
    
    projects.WhenProjectLoaded << [=] {
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
    auto menuFn = [=] (Bar& menu) {
            menu.Sub(AppIdentity::ProductName(), [=](Bar& bar) {
                bar.Add("New...", CtrlImg::new_doc(), [=] {
                    project.SaveProject();
                    projects.NewProject();
                }).Key(K_CTRL_N);
                bar.Add("Open...", CtrlImg::open(), [=] {
                    projects.OpenProject();
                }).Key(K_CTRL_O);
                bar.Add("Save", CtrlImg::save(), [=] {
                    project.SaveProject();
                }).Key(K_CTRL_S).Enable(project.IsProjectDirty());
                bar.Add("Save As...", CtrlImg::save_as(), [=] {
                    project.SaveProjectAs();
                }).Key(K_CTRL_A).Enable(project.IsProjectOpen());
                bar.Add("Close", CroonImg::Close(), [=] {
                    if (project.CloseProject(false)) HideProject();
                }).Key(K_CTRL_W).Enable(project.IsProjectOpen());
                bar.Separator();
                bar.Add("Settings...", CroonImg::Settings(), [=] {
                    SettingsDlg sDlg;
                    sDlg.Execute();
                });
                bar.Separator();
                bar.Add(Format("Quit %s", AppIdentity::ProductName()), [=] {
                    Close();
                }).Key(K_CTRL_Q);
            });
            project.MainMenu()(menu);
            menu.Sub("Help", [=](Bar& bar) {
                bar.Add("Help", [=]{
                    PromptOK("Help is on the way...");
                }).Key(K_CTRL_H);
                bar.Separator();
                bar.Add("About...", CroonImg::Info(),  [=]{
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
