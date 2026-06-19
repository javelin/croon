/*
 * File  : MainWindow.h
 * Author: Mark Documento
 */

#ifndef _Croon_MainWindow_h_
#define _Croon_MainWindow_h_

class MainWindow : public WithCroonMainWindowLayout<TopWindow> {
public:
    MainWindow();
    ~MainWindow();
    void Close() override {
        if (project.CloseProject(true)) TopWindow::Close();
    }
    void ShowProject(bool show=true) {
        projects.Enable(!show); projects.Show(!show);
        project.Enable(show); project.Show(show);
        SetTheMainMenu();
    }
    void HideProject() {
        projects.UpdateListView();
        project.Disable(); project.Hide();
        projects.Enable(); projects.Show();
        SetTheMainMenu();
    }
    
private:
    void SetTheMainMenu();
    
private:
    StatusBar status;
#ifndef PLATFORM_COCOA
    MenuBar menuBar;
#endif
};

#endif
