/*
 * File  : TimingDlg.h
 * Author: Mark Documento
 */

#ifndef _Croon_TimingDlg_h_
#define _Croon_TimingDlg_h_

class TimingDlg : public WithCroonTimingDlgLayout<TopWindow> {
public:
    TimingDlg();
    void Close() override;
    int Run(KarData& karData) {
        data = &karData;
        Populate();
        return TopWindow::Run();
    }
    bool IsDirty() const { return dirty; }
    
public:
    static const int lineHeight = 50;
    static const int timerId = 1;
    
private:
    bool Key(dword key, int count) override;
    void PollProgress();
    void Populate();
    void SetProgress(double position);
    void TogglePlay();
    
private:
    KarData* data;
    bool dirty{false};
};

#endif
