/*
 * File  : TimingLineDlg.cpp
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
#include "KarData.h"
#include "Visualization.h"
#include "LyricsTransformer.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ProjectLoader.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "TimingLineDlg.h"

TimingLineDlg::TimingLineDlg(String decor, String lyrics) {
    CtrlLayout(*this, "Edit Lyrics");
    CenterOwner();
    
    decorEd.SetData(decor);
    lyricsEd.SetData(lyrics);
    
    okBtn.Ok();
    okBtn << [=] {
        Break(IDOK);
        Close();
    };

    cancelBtn.Cancel();
    cancelBtn << [=] {
        Break(IDCANCEL);
        Close();
    };
}
