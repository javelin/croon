#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#include "LyricsPartsCtrl.h"

void LyricsPartsCtrl::Paint(Draw& w) {
	Size sz = GetSize();
    int rh  = RowHeight();
    int bs  = BoxSize();

    // Background
    w.DrawRect(sz, White());

    // Visible range
    int first = max(0, sb / rh);
    int last  = min(lines.GetCount(), (sb + sz.cy + rh - 1) / rh);

    // Grid X coordinates
    int x0 = 0;
    int xL = sz.cx - BoxesWidth(); // lyric/boxes boundary
    if(xL < 0) xL = 0;
    int xR = xL + BoxesWidth();    // right edge of last box column
    
    Rect lyricr = RectC(x0, 0, max(0, xL - x0), rh);
            w.DrawRect(lyricr, White());
    w.DrawText(max(5, (xL - x0)/2), (HeaderHeight() - boldFont.GetHeight()) / 2,
                   "LYRICS", boldFont, Black());
    // Box boundaries including lyric/box boundary and last right edge
    for(int b = 0; b <= BOX_COUNT; b++) {
        int x = xL + b * bs;
        w.DrawLine(x, 0, x, HeaderHeight(), 1, SColorText());
        if (b < BOX_COUNT) {
            w.DrawText(x + BoxSize()/2, (HeaderHeight() - boldFont.GetHeight()) / 2,
                        IntStr(b + 1), boldFont, Black());
        }
    }

    for(int i = first; i < last; i++) {
        //int yT = i * rh - sb;
        int yT = HeaderHeight() + i * rh - sb;
        int yB = yT + rh;

        const bool has = HasLyrics(i);

        // 1) Fills (only for lyric rows)
        if(has) {
            // Lyric cell background (optional)
            Rect lyricr = RectC(x0, yT, max(0, xL - x0), rh);
            w.DrawRect(lyricr, White());

            // Box cell backgrounds + highlights
            for(int b = 0; b < BOX_COUNT; b++) {
                Rect r = RectC(xL + b * bs, yT, bs, rh);
                bool on = (state[i] & (1u << b)) != 0;

                w.DrawRect(r, SColorFace());
                if(on)
                    w.DrawRect(r.Deflated(3, 3), Color(255, 220, 120));
            }
        }

        // 2) Text (always, but clipped to lyric area so it doesn't go under boxes)
        auto line = TrimBoth(lines[i]);
        if (!line.IsEmpty() && line != "-") {
            Rect clipr = RectC(0, yT, max(0, xL - 1), rh);
            w.Clip(clipr);
            int text_x = 6;
            int text_y = yT + (rh - font.GetHeight()) / 2;
            w.DrawText(text_x, text_y, lines[i], font, Black);
            w.End();
        }

        // 3) Grid lines (only for lyric rows, drawn last)
        if(has) {
            // Left border
            w.DrawLine(x0, yT, x0, yB, 1, SColorText());

            // Box boundaries including lyric/box boundary and last right edge
            for(int b = 0; b <= BOX_COUNT; b++) {
                int x = xL + b * bs;
                w.DrawLine(x, yT, x, yB, 1, SColorText());
            }

            // Top & bottom edges spanning lyric rect + all boxes
            w.DrawLine(x0, yT, xR, yT, 1, SColorText());
            w.DrawLine(x0, yB, xR, yB, 1, SColorText());
        }
    }
}
