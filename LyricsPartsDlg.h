#ifndef _Croon_LyricsPartsDlg_h_
#define _Croon_LyricsPartsDlg_h_

class LyricsPartsDlg : public WithCroonLyricsPartsLayout<TopWindow> {
public:
    LyricsPartsDlg();
    
    int Run(KarData& karData);
    
    bool IsDirty() const { return dirty; }
    Vector<Tuple<int,bool,bool,bool>> GetParts() const { return lpCtrl.GetParts(); }
    
private:
    void Populate();
    
    KarData* data{0};
    bool dirty{false};
};

#endif
