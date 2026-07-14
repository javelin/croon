/*
 * File  : MainWindow.h
 * Author: Mark Documento
 */

#ifndef _Croon_MainWindow_h_
#define _Croon_MainWindow_h_

struct KarData;

class MainWindow : public WithCroonMainWindowLayout<TopWindow> {
public:
    MainWindow(KarData& data);
    ~MainWindow();
    void Close() override;
    void ShowProject(bool show=true);
    void HideProject();
    
private:
    void SetTheMainMenu();
    
private:
    VideoDlg videoDlg;
    WizardDlg wizardDlg;
    Project project;
    ProjectList projects;
    StatusBar status;
#ifndef PLATFORM_COCOA
    MenuBar menuBar;
#endif
};

#endif
