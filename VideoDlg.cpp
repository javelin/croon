/*
 * File  : VideoDlg.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_header.h>

#include "Constants.h"
#include "ConfigService.h"
#include "Config.h"
#include "UiScaler.h"
#include "LyricsPartsCtrl.h"
#include "ListCtrl.h"
#include "AppIdentity.h"
#include "AppPaths.h"
#include "KarData.h"
#include "Visualization.h"
#include "LyricsTransformer.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ProjectLoader.h"
#include "Page.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "ProgressDlg.h"
#include "GatherDlg.h"
#include "SaveProjectDlg.h"
#include "VidThumbnail.h"
#include "Page3.h"

#define LAYOUTFILE <Croon/CroonVideoDlg.lay>
#include <CtrlCore/lay.h>

#include "VideoDlg.h"

VideoDlg::VideoDlg(KarData& data) : gatherDlg(), page3(data, gatherDlg) {
    CtrlLayout(*this);
    Add(page3.HSizePosZ().VSizePosZ(0, 40));
    Title("Background Video").NoZoomable().Sizeable().SetRect(0, 0, UiScaler::X(450), UiScaler::Y(550));
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
