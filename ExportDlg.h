/*
 * File  : ExportDlg.h
 * Author: Mark Documento
 */

#ifndef _Croon_ExportDlg_h_
#define _Croon_ExportDlg_h_

class ExportDlg : public ProgressDlg {
public:
    ExportDlg();
    int Run(const KarData& karData, String outputPath, int len=1, double thumbnailTS=-1.0f);
    
private:
    enum Phase {
        SaveASS,
        Dehiss,
        ExportVideo,
        GenerateThumbnail,
        Finished
    };
    
private:
    void ExportASS();
    void DehissAudio();
    void ExportViz();
    void ExportBg();
    void StartExport();
    void GenThumbnail();
    
private:
    Phase phase{SaveASS};
    int length{1};
    const KarData* data{nullptr};
    String dehissedAudioFilepath;
    String outputPath;
    String assFilePath;
    Vector<String> args;
    double thumbnailTS{0.0f};
    String renderTime;
    int progressVal{0};
    String ffmpeg;
};

#endif
