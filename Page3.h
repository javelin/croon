/*
 * File  : Page3.h
 * Author: Mark Documento
 */

#ifndef _Croon_Page3_h_
#define _Croon_Page3_h_

class Page3 : public WithCroonWizardPage3Layout<Page> {
public:
    Page3(String gatherKey = "");
    void Layout() override;
    void HideButtons() override {
        Page::HideButtons(); gatherBtn.Hide(); gatherBtn.Disable(); }
    bool SetPath(String path);
    void GatherVideos();
    void AddVideoItem(ListCtrl* list, String path, String tnPath, Image img, ListCtrl* other);
    Button& GatherButton() { return gatherBtn; };
    Button& GatherButton(bool show, bool enable, const char* label=nullptr) {
        if (label) gatherBtn.SetLabel(label);
        gatherBtn.Show(show);
        gatherBtn.Enable(enable);
        return gatherBtn;
    }
    void Populate() override;
    void Reset() override;
    void SaveData() override;
    void Rehint(bool set) { rehint = set; }
    bool Rehint() const { return rehint; }
    
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
};

#endif
