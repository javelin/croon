/*
 * File  : VideoDlg.h
 * Author: Mark Documento
 */

#ifndef _Croon_VideoDlg_h_
#define _Croon_VideoDlg_h_

class VideoDlg : public WithCroonVideoDlgLayout<TopWindow> {
public:
    VideoDlg(KarData& data);
    int Run() {
        page3.Reset();
        SetTimeCallback(500, [=] { page3.Rehint(false); });
        return Execute();
    }
    void SetData(const Value& data) override { SetPath((String)data); }
    Value GetData() const override { return Value(value); }
    void SetPath(String path) {
        bool found = page3.SetPath(path);
        okBtn.Enable(found);
        if (found) value = path;
    }
    Image GetImage() { return image; }
    String GetThumbnailPath() const { return tnPath; }
    
private:
    GatherDlg gatherDlg;
    Page3 page3;
    Image image;
    String value;
    String tnPath;
};

#endif
