/*
 * File  : Project.h
 * Author: Mark Documento
 */

#ifndef _Croon_Project_h_
#define _Croon_Project_h_

struct KarData;
class VideoDlg;

class Project : public WithCroonProjectLayout<ParentCtrl> {
public:
    Project(KarData& data, VideoDlg& videoDlg);
    virtual ~Project() { CleanUp(); }
    void Populate();
    void SaveProject();
    void SaveProjectAs();
    void UpdateLyricsData();
    bool CloseProject(bool quitting);
    bool IsProjectOpen() const { return open; }
    bool IsProjectDirty() const { return open && dirty; }
    void Timing();
    void VocalParts();
    void ReplaceAudio();
    void ExportVideo(int length=1);
    Event<bool> WhenDirty;
    Event<> WhenProjectSaved;
    Event<Bar&> MainMenu();
    
public:
    static const int timerId{1};
    
private:
    void SetDirty(bool set=true) {
        WhenDirty((dirty = set));
        saveBtn.Enable(dirty);
    }
    void CleanUp();
    
private:
    DocEdit lyricsEd;
    RichTextCtrl previewRT;
    String videoPath;
    bool open{false};
    bool dirty{false};
    KarData& data;
    VideoDlg& videoDlg;
};

#endif
