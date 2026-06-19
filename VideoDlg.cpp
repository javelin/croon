/*
 * File  : VideoDlg.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

VideoDlg::VideoDlg() {
    CtrlLayout(*this);
    Title("Background Video").NoZoomable().Sizeable().SetRect(0, 0, Zx(450), Zy(550));
    CenterScreen();
    okBtn.Ok();
    cancelBtn.Cancel();
    *this << page3.GatherButton(true, true, "Find Videos").LeftPosZ(10, 70).BottomPosZ(10, 25);
    page3.WhenSelected << [=] (String path, String tnpath, Image img) {
        okBtn.Enable();
        SetData(path);
        tnPath = tnpath;
        image = img;
    };
    
    cancelBtn << [=] {
        Break(IDCANCEL);
    };
    okBtn << [=] {
        Break(IDOK);
    };
    okBtn.Disable();
}
