/*
 * File  : SettingsDlg.cpp
 * Author: Mark Documento
 */

#ifndef _Croon_SettingsDlg_h_
#define _Croon_SettingsDlg_h_

class SettingsDlg : public WithCroonSettingsLayout<TopWindow> {
public:
    SettingsDlg();
    void Close() override;
    
private:
    String ffmpegLoc;
};

#endif
