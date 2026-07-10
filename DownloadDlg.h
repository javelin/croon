/*
 * File  : DownloadDlg.h
 * Author: Mark Documento
 */

#ifndef _Croon_DownloadDlg_h_
#define _Croon_DownloadDlg_h_

#include <memory>

class DownloadDlg : public ProgressDlg {
public:
    DownloadDlg();
    ~DownloadDlg();
    int Run(String url, String title="Download", String userAgent=DownloadDefaults::UserAgent());
    String GetContent() const;
    
    Event<String> WhenDownloadSuccess;
    
private:
    struct RequestState;
    RequestState& Request();
    const RequestState& Request() const;
    void PollProgress() override;
    
private:
    std::unique_ptr<RequestState> request;
};

#endif
