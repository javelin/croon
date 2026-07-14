#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#include "LyricsPartsCtrl.h"

LyricsPartsCtrl::LyricsPartsCtrl() {
    font = StdFont();
    boldFont = font.Bold();
    vpad = 12;
    AddFrame(sb);
    sb.WhenScroll = [=] { Refresh(); };
    sb.SetLine(1);
    dragging = false;
    last_row = last_col = -1;
    SetFrame(InsetFrame());
    BackPaint();
}

void LyricsPartsCtrl::SetLyricsAndParts(Vector<String>&& lyrics,
                               const Vector<Tuple<int, bool, bool, bool>>& parts) {
    lines = pick(lyrics);
    state.SetCount(lines.GetCount(), 0);
    for(const auto& t : parts) {
        int row = t.a;
        if(row < 0 || row >= lines.GetCount()) continue;
        if(!HasLyrics(row)) continue;
        dword bits = 0;
        if(t.b) bits |= 1u;
        if(t.c) bits |= 2u;
        if(t.d) bits |= 4u;
        state[row] = bits;
    }
    SyncScroll();
    Refresh();
}

void LyricsPartsCtrl::ClearParts() {
    for (int i = 0; i < state.GetCount(); ++i) {
        state[i] = 0;
    }
}

int LyricsPartsCtrl::GetCount() const {
    return lines.GetCount();
}

bool LyricsPartsCtrl::GetBox(int row, int col) const {
    if(row < 0 || row >= state.GetCount()) return false;
    if(col < 0 || col >= BOX_COUNT) return false;
    return (state[row] >> col) & 1;
}

void LyricsPartsCtrl::SetBox(int row, int col, bool on) {
    if(row < 0 || row >= state.GetCount()) return;
    if(col < 0 || col >= BOX_COUNT) return;
    if(!HasLyrics(row)) return;
    dword mask = (1u << col);
    bool cur = (state[row] & mask) != 0;
    if(cur == on) return;
    if(on) state[row] |= mask;
    else   state[row] &= ~mask;
    RefreshRow(row);
    WhenToggle(row, col, on);
}

Vector<Tuple<int, bool, bool, bool>> LyricsPartsCtrl::GetParts() const {
    Vector<Tuple<int, bool, bool, bool>> result;
    for(int row = 0; row < lines.GetCount(); row++) {
        if(!HasLyrics(row)) continue;
        bool b1 = (state[row] & 1) != 0;
        bool b2 = (state[row] & 2) != 0;
        bool b3 = (state[row] & 4) != 0;
        result.Add(Tuple<int, bool, bool, bool>(row, b1, b2, b3));
    }
    return result;
}

void LyricsPartsCtrl::SetFont(Font f) {
    font = f;
    boldFont = font.Bold();
    SyncScroll();
    Refresh();
}

void LyricsPartsCtrl::SetVerticalPadding(int px) {
    vpad = max(0, px);
    SyncScroll();
    Refresh();
}

bool LyricsPartsCtrl::HasLyrics(int row) const {
    if(row < 0 || row >= lines.GetCount()) return false;
    auto line = TrimBoth(lines[row]);
    return !line.IsEmpty() && line != "-";
}

int LyricsPartsCtrl::RowHeight() const {
    return max(font.GetHeight() + vpad, 24);
}

int LyricsPartsCtrl::HeaderHeight() const {
    return RowHeight();
}

int LyricsPartsCtrl::BoxSize() const {
    return RowHeight();
}

int LyricsPartsCtrl::BoxesWidth() const {
    return BOX_COUNT * BoxSize();
}

void LyricsPartsCtrl::SyncScroll() {
    int rh = RowHeight();
    int total = lines.GetCount() * rh;
    int page  = max(0, GetSize().cy - HeaderHeight());
    sb.SetTotal(total);
    sb.SetPage(page);
    sb.SetLine(rh);
    int maxpos = max(0, total - page);
    sb.Set(clamp((int)sb, 0, maxpos));
}

void LyricsPartsCtrl::RefreshRow(int row) {
    int rh = RowHeight();
    int y0 = HeaderHeight() + row * rh - sb;
    Refresh(0, y0, GetSize().cx, rh);
}

Rect LyricsPartsCtrl::BoxRect(int row, int col) const {
    Size sz = GetSize();
    int rh = RowHeight();
    int bs = BoxSize();
    int y  = HeaderHeight() + row * rh - sb;
    int xL = sz.cx - BoxesWidth();
    return RectC(xL + col * bs, y, bs, rh);
}

bool LyricsPartsCtrl::HitTestBox(Point p, int& out_row, int& out_col) const {
    Size sz = GetSize();
    int rh = RowHeight();
    int bs = BoxSize();
    int xL = sz.cx - BoxesWidth();
    if(p.x < xL) return false;
    int y = p.y - HeaderHeight() + sb;
    if(y < 0) return false;
    int row = y / rh;
    if(row < 0 || row >= lines.GetCount()) return false;
    if(!HasLyrics(row)) return false;
    int local_x = p.x - xL;
    int col = local_x / bs;
    if(col < 0 || col >= BOX_COUNT) return false;
    Rect r = BoxRect(row, col);
    if(!r.Contains(p)) return false;
    out_row = row;
    out_col = col;
    return true;
}

void LyricsPartsCtrl::ToggleAt(Point p) {
    int row, col;
    if(!HitTestBox(p, row, col)) {
        last_row = last_col = -1;
        return;
    }
    if(row == last_row && col == last_col) return;
    last_row = row;
    last_col = col;
    dword mask = (1u << col);
    bool now_on = ((state[row] & mask) == 0);
    if(now_on) state[row] |= mask;
    else       state[row] &= ~mask;
    RefreshRow(row);
    WhenToggle(row, col, now_on);
}

void LyricsPartsCtrl::Layout() {
    SyncScroll();
}

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

void LyricsPartsCtrl::MouseWheel(Point, int zdelta, dword) {
    sb.Wheel(zdelta);
    Refresh();
}

void LyricsPartsCtrl::LeftDown(Point p, dword) {
    SetFocus();
    dragging = true;
    SetCapture();
    last_row = last_col = -1;
    ToggleAt(p);
}

void LyricsPartsCtrl::MouseMove(Point p, dword) {
    if(dragging) ToggleAt(p);
}

void LyricsPartsCtrl::LeftUp(Point, dword) {
    dragging = false;
    last_row = last_col = -1;
    ReleaseCapture();
}

void LyricsPartsCtrl::LostCapture() {
    dragging = false;
    last_row = last_col = -1;
}
