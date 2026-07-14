/*
 * File  : Page3.h
 * Author: Mark Documento
 */

#ifndef _Croon_Page3_h_
#define _Croon_Page3_h_

struct KarData;
class GatherDlg;

class Page3 : public WithCroonWizardPage3Layout<Page> {
public:
    Page3(KarData& data, GatherDlg& gatherDlg, String gatherKey = "");
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
    
    Event<String, String, Image> WhenSelected;
    Event<> WhenProjectSaved;
    
private:
    int vidCount;
    ListCtrl vizLst;
    ListCtrl videoLst;
    Array<VidThumbnail> vtArray;
    String gatherKey;
    Button gatherBtn;
    bool rehint{true};
    KarData& data;
    GatherDlg& gatherDlg;
};

#endif
