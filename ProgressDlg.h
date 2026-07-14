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
    virtual int RunDlg(const char* title);
    
    Event<bool> WhenAbortingProcess;
    Event<bool> WhenProcessAborted;
    Gate<int> WhenProcessEnded;
    Event<> WhenStartNextProcess;
    Event<String> WhenProcessOutput;
    Event<> WhenUpdateProgress;
    Gate<> WhenWarnOnClose;
    
protected:
    virtual void ProcessAborted();
    virtual void ProcessAbortedByUser();
    virtual void PollProgress();
    virtual bool ProcessEnded(int code);
    virtual void ProcessOutput(String output);
    virtual void StartNextProcess();
    virtual void UpdateProgress();
    
public:
    static const int timerId{1};
    
protected:
    MediaProcessRunner process;
    Vector<String> procArgs;
};

#endif
