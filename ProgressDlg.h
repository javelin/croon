/*
 * File  : ProgressDlg.h
 * Author: Mark Documento
 */

#ifndef _Croon_ProgressDlg_h_
#define _Croon_ProgressDlg_h_

class ProgressDlg : public WithCroonProgressLayout<TopWindow> {
public:
    ProgressDlg();
    virtual void Close() override;
    virtual int RunDlg(const char* title) {
        auto ret = Title(title).Run();
        TopWindow::Close();
        Hide();
        return ret;
    }
    
    Event<bool> WhenAbortingProcess;
    Event<bool> WhenProcessAborted;
    Gate<int> WhenProcessEnded;
    Event<> WhenStartNextProcess;
    Event<String> WhenProcessOutput;
    Event<> WhenUpdateProgress;
    Gate<> WhenWarnOnClose;
    
protected:
    virtual void ProcessAborted() { WhenProcessAborted(false); }
    virtual void ProcessAbortedByUser() { WhenProcessAborted(true); }
    virtual void PollProgress();
    virtual bool ProcessEnded(int code) { return WhenProcessEnded(code); }
    virtual void ProcessOutput(String output) { WhenProcessOutput(output); }
    virtual void StartNextProcess() { WhenStartNextProcess(); }
    virtual void UpdateProgress() { WhenUpdateProgress(); }
    
public:
    static const int timerId{1};
    
protected:
    LocalProcess process;
    Vector<String> procArgs;
};

#endif
