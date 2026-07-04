/*
 * File  : LyricsEditor.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>
#include <GridCtrl/GridCtrl.h>

using namespace Upp;

#include "KarData.h"
#include "TimeFormatter.h"
#include "LyricsEditor.h"

LyricsEditorLine::LyricsEditorLine(const TimeLyrics& tl) {
    *this << lineEd.SetFrame(NullFrame()).HSizePos().VSizePos();
    lineEd.WhenEnter = Proxy(WhenEnter);
    lineEd.WhenHighlight = Proxy(WhenHighlight);
    lineEd.SetData(tl.lyrics);
    WantFocus(false);
}

LyricsEditor::LyricsEditor() {
    Add(lyricsList.HSizePos().VSizePos());
}

void LyricsEditor::Populate() {
    auto& data = KarData::GetGlobal();
    lyricsList.Clear();
    lyricsList.AddColumn("Time", 150);
    lyricsList.AddColumn("Lyrics", 1000).Edit(editor);
    for (auto& tl : data.timedLyrics) {
        if (data.timedLyrics.GetIndex(tl) > 0) {
            lyricsList.Add(TimeFormatter::Format(tl.time), tl.lyrics);
        }
    }
    lyricsList.WhenStartEdit << [=] {
        editor.CancelSelection();
    };
}
