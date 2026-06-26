/*
 * File  : DownloadDlg.h
 * Author: Mark Documento
 */

#ifndef _Croon_DownloadDlg_h_
#define _Croon_DownloadDlg_h_

class DownloadDlg : public ProgressDlg {
public:
    DownloadDlg();
    int Run(String url, String title="Download", String userAgent=DownloadDefaults::UserAgent());
    String GetContent() const { return request.GetContent(); }
    
    Event<String> WhenDownloadSuccess;
    
private:
    void PollProgress() override;
    void ExtractLyrics(String content);
    
private:
    HttpRequest request;
};

#endif
