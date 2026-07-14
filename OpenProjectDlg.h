/*
 * File  : OpenProjectDlg.h
 * Author: Mark Documento
 */

#ifndef _Croon_OpenProjectDlg_h_
#define _Croon_OpenProjectDlg_h_

class OpenProjectDlg : public ProgressDlg {
public:
    OpenProjectDlg();
    int Run(String savePath, KarData& data);
    void Close() override;
    
private:
    enum Phase {
        Audio = 1,
        Video,
        Thumbnail,
        Finished
    };
    void StartOpen();
    void ExtractAudio();
    void ExtractVideo();
    void LoadThumbnail();
    
    String Phase() const;
    
private:
    KarData* data;
    String ffmpeg;
    int phase;
    String output;
};

#endif
