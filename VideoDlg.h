/*
 * File  : VideoDlg.h
 * Author: Mark Documento
 */

#ifndef _Croon_VideoDlg_h_
#define _Croon_VideoDlg_h_

class VideoDlg : public WithCroonVideoDlgLayout<TopWindow> {
public:
    VideoDlg(KarData& data);
    int Run();
    void SetData(const Value& data) override;
    Value GetData() const override;
    void SetPath(String path);
    Image GetImage();
    String GetThumbnailPath() const;
    
private:
    GatherDlg gatherDlg;
    Page3 page3;
    Image image;
    String value;
    String tnPath;
};

#endif
