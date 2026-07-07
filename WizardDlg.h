/*
 * File  : WizardDlg.h
 * Author: Mark Documento
 */

#ifndef _Croon_WizardDlg_h_
#define _Croon_WizardDlg_h_

struct KarData;

class WizardDlg : public WithCroonWizardLayout<TopWindow> {
public:
    WizardDlg();
    WizardDlg(KarData& data);
    int Run(String tempAudioPath, double duration, String origAudioFilePath);
    void Close() override;
    
public:
    static const int maxPages = 3;
    Page1 page1;
    Page2 page2;
    Page3 page3;
    
private:
    String tempAudioPath;
    KarData& data;
    Page* pages[maxPages];
    int currPage;
};

#endif
