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

#include "LyricsPartsDlg.h"

LyricsPartsDlg::LyricsPartsDlg() {
    Title("Assign Lyrics Parts").NoZoomable().Sizeable().SetMinSize(Size(UiScaler::X(400), UiScaler::Y(220)));
    CtrlLayout(*this);
    CenterOwner();
    
    okBtn.Ok();
    okBtn << [=] { Break(IDOK); };
    okBtn.Disable();

    cancelBtn.Cancel();
    cancelBtn << [=] {
        if (dirty) {
            if (PromptYesNo("Discard changes and cancel?") != 1) return;
        }
        Break(IDCANCEL);
    };
    
    clearBtn << [=] {
        if (PromptYesNo("Clear all parts assignments?") == 1) {
            lpCtrl.ClearParts();
            dirty = true;
            okBtn.Enable();
        }
    };
    
    lpCtrl.WhenToggle = [=](int row, int col, bool set) { dirty = true; okBtn.Enable(); };
}

int LyricsPartsDlg::Run(KarData& karData) {
    dirty = false;
    data = &karData;
    Populate();
    return TopWindow::Run();
}

void LyricsPartsDlg::Populate() {
    Vector<String> lyrics;
    Vector<Tuple<int, bool, bool, bool>> parts;
    for (auto const &tl : data->timedLyrics) lyrics.Add(tl.lyrics);
    for (auto const &part : data->parts) parts.Add(part);
    lpCtrl.SetLyricsAndParts(pick(lyrics), parts);
}
