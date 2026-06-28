#include "Croon.h"

LyricsPartsDlg::LyricsPartsDlg() {
    Title("Assign Lyrics Parts").NoZoomable().Sizeable().SetMinSize(Size(UiScaler::X(400), UiScaler::Y(220)));
    CtrlLayout(*this);
    CenterOwner();
    
    okBtn << [=] { Break(IDOK); };
    okBtn.Disable();
    
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

void LyricsPartsDlg::Populate() {
    Vector<String> lyrics;
    Vector<Tuple<int, bool, bool, bool>> parts;
    for (auto const &tl : data->timedLyrics) lyrics.Add(tl.lyrics);
    for (auto const &part : data->parts) parts.Add(part);
    lpCtrl.SetLyricsAndParts(pick(lyrics), parts);
}
