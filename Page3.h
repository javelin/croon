/*
 * File  : Page3.h
 * Author: Mark Documento
 */

#ifndef _Croon_Page3_h_
#define _Croon_Page3_h_

#include "MediaProcessRunner.h"

struct KarData;

class Page3 : public WithCroonWizardPage3Layout<Page> {
public:
    Page3(KarData& data, String gatherKey = "");
    virtual ~Page3();
    void Layout() override;
    void HideButtons() override;
    bool SetPath(String path);
    void GatherVideos();
    void AddVideoItem(ListCtrl* list, String path, String tnPath, Image img, ListCtrl* other);
    Button& GatherButton();
    Button& GatherButton(bool show, bool enable, const char* label=nullptr);
    void Populate() override;
    void Reset() override;
    void SaveData() override;
    void Rehint(bool set);
    bool Rehint() const;
    void StopGathering();

    Event<String, String, Image> WhenSelected;
    Event<> WhenProjectSaved;

private:
    void StartGather(String videoDir);
    void StopGather(bool killProcess);
    void GatherNextVideo();
    void PollGatherProcess();
    bool AddGatheredVideo(String path);
    void FinishGather();
    
private:
    static const int gatherTimerId{2};
    int vidCount;
    ListCtrl vizLst;
    ListCtrl videoLst;
    Array<VidThumbnail> vtArray;
    String gatherKey;
    Button gatherBtn;
    bool rehint{true};
    bool gathering{false};
    bool overwriteTN{false};
    int gatherIndex{0};
    String ffmpeg;
    Vector<String> gatherPaths;
    MediaProcessRunner gatherProcess;
    KarData& data;
};

#endif
