/*
 * File  : TimingCtrl.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#include "Constants.h"
#include "KarData.h"
#include "UiScaler.h"
#include "TimingLine.h"
#include "TimingCtrl.h"

TimingCtrl::TimingCtrl(int lineHght) : lineHeight(UiScaler::Y(lineHght)), position(0.0f), listPos(0), timed(0) {
	scrollBar.Vert().AutoHide();
	AddFrame(BlackFrame());
	AddFrame(scrollBar);
	scrollBar.WhenScroll = [=] {
		int pos = scrollBar.Get();
		for(int i = 0; i < lines.GetCount(); i++){
			lines[i].TopPos(lineHeight*i - pos + UiScaler::Y(10), lineHeight - UiScaler::Y(5));
		}
	};
	scrollBar.SetLine(lineHeight);
	WantFocus();
}

bool TimingCtrl::Key(dword key, int count) {
    switch (key) {
    case K_SPACE:
        if (timed + 2 == lines.GetCount()) {
            // We already timed everything
        }
        else if (timed == 0 && listPos == timed) {
            lines[timed].Normal();
            ++listPos;
            WhenPlayAt(0.0f);
        }
        else if (listPos == timed + 1 && timed < lines.GetCount() - 1) {
            if (WhenTiming(position)) {
                lines[timed].Normal();
                lines[++timed].SetPosition(position);
                WhenDirty();
                lines[timed].EnableTimeBtns();
                listPos = timed + 1;
                SetFocusLine();
                if (timed + 2 == lines.GetCount()) {
                    WhenDoneTiming();
                }
            }
        }
        return true;
    default:
        return GetParent()->Key(key, count);
    }
    
    return false;
}

void TimingCtrl::SetRawLyrics(const String& text) {
    lines.Clear();
    lyrics.Clear();
    int lastLF = 0;
    int pos = -1;
    while ((pos = text.Find('\n', lastLF)) > -1) {
        auto line = text.Mid(lastLF, pos - lastLF);
        lastLF = pos + 1;
        line = TrimBoth(line);
        if (!line.IsEmpty()) {
            String _line = "";
            Vector<String> split = Split(line, ' ');
            for (int i = 0; i < split.GetCount(); i++) {
                if (_line.GetLength() + split[i].GetLength() + 1 > MaxLineLength) {
                    lyrics.Add(TrimBoth(_line));
                    _line = "";
                    _line = split[i];
                }
                else {
                    _line += " ";
                    _line += split[i];
                }
            }
            if (!_line.IsEmpty()) lyrics.Add(TrimBoth(_line));
        }
    }
    auto line = text.Right(text.GetLength() - lastLF);
    line = TrimBoth(line);
    if (!line.IsEmpty()) {
        String _line = "";
        Vector<String> split = Split(line, ' ');
        for (int i = 0; i < split.GetCount(); i++) {
            if (_line.GetLength() + split[i].GetLength() + 1 > MaxLineLength) {
                lyrics.Add(TrimBoth(_line));
                _line = split[i];
            }
            else {
                _line += " ";
                _line += split[i];
            }
        }
        if (!_line.IsEmpty()) lyrics.Add(TrimBoth(_line));
    }
    
    RemoveLinesFromList();
    LyricsToLines();
    AddLinesToList();
    position = 0.0f;
    listPos = timed = 0;
}

void TimingCtrl::LyricsToLines() {
    lines.Clear();
    lines.Add(TimingLine("(INTRO)", 0.0f, false));
    lines.front().WhenLeftClicked << [=] {
        KillSetTimeCallback(1000, [=] {
            listPos = 0;
            ReadyForTiming();
            WhenPlayAt(0.0f);
            SetFocus();
        }, timerId);
    };
    for (int i = 0; i < lyrics.GetCount(); i++) {
        auto& line = lyrics[i];
        lines.Add(TimingLine(line, 0.0f));
        auto& tl = lines.back();
        int n = lines.GetCount() - 1;
        tl.SetDuration(duration);
        tl.WhenEditingLyrics << [=] {
            editing = true;
            WhenInterrupt();
        };
        tl.WhenInterrupt << Proxy(WhenInterrupt);
        tl.WhenLeftClicked << [=] {
            if (n <= timed) {
                KillSetTimeCallback(500, [=] {
                    listPos = n;
                    ReadyForTiming();
                    if (!editing) WhenPlayAt(lines[n].GetPosition());
                    SetFocus();
                }, timerId);
            }
        };
        tl.WhenLyricsEdited << [=](String text) {
            editing = false;
            lyrics[i] = text;
            WhenDirty();
        };
        tl.WhenRetime << [=] {
            for (int j = n; j <= timed + 1; j++) {
                lines[j].ResetTiming();
            }
            timed = n - 1;
            listPos = timed + 1;
            ReadyForTiming();
            WhenRetime(timed);
            WhenDirty();
            WhenPlayAt(lines[timed].GetPosition());
            SetFocus();
        };
        tl.WhenSetPosition << [=] (auto pos) {
            if (pos < 0.0f || pos > duration) return false;
            if (n < timed && pos > lines[n + 1].GetPosition()) return false;
            if (n > 1 && pos < lines[n - 1].GetPosition()) return false;
            WhenDirty();
            return true;
        };
        tl.WhenTimeButtonsDisabled << [=] {
            for (int j = lines.GetCount() - 1; j > i; --j) {
                lines[j].EnableTimeBtns(false);
            }
        };
    }
    lines.Add(TimingLine());
}

void TimingCtrl::AddLinesToList() {
    for (int i = 0; i < lines.GetCount(); ++i) {
        Add(lines[i].HSizePosZ(10, 10).TopPos(UiScaler::Y(10) + (i * lineHeight), lineHeight - UiScaler::Y(5)));
    }
    
    scrollBar.SetPage(GetSize().cy);
    scrollBar.SetTotal(UiScaler::Y(20) + lines.GetCount()*lineHeight);
}

void TimingCtrl::RemoveLinesFromList() {
    for (int i = 0; i < lines.GetCount(); ++i) {
        lines[i].Remove();
    }
}

void TimingCtrl::SetMusicPosition(double position, double duration) {
    this->position = position;
    this->duration = duration;
    for (int i = 0; i < lines.GetCount(); i++) {
        lines[i].SetDuration(duration);
        if (timed == 0 && listPos == timed) continue;
        if (listPos < timed + 1 && lines[i].GetPosition() <= position) {
            if ((i + 1 < lines.GetCount()) && position < lines[i + 1].GetPosition()) {
                listPos = i;
            }
            else if (i == timed) {
                listPos = i;
            }
        }
    }
    if (timed > 0 && listPos == timed && timed + 1 < lines.GetCount()) ++listPos;
    for (int i = 0; i < lines.GetCount(); i++) {
        lines[i].Normal();
    }
    SetFocusLine();
}

void TimingCtrl::SetFocusLine() {
    if (!lines.GetCount()) return;
    ScrollToLineAndCenter(listPos);
    if (timed == 0 && listPos == timed) {
        lines[listPos].Highlight1();
    }
    else if (listPos == timed + 1) {
        lines[listPos].Highlight1();
        lines[timed].Highlight2();
    }
    else {
        lines[listPos].Highlight2();
    }
}

void TimingCtrl::ScrollToLineAndCenter(int row) {
    auto height = GetSize().cy;
    auto pos = lineHeight*row + UiScaler::Y(10) - (height/2 - (lineHeight - UiScaler::Y(5))/2);
    scrollBar.Set(pos);
}

void TimingCtrl::ReadyForTiming() {
    for (int i = 0; i < lines.GetCount(); i++) lines[i].Normal();
    SetFocusLine();
}

Vector<TimeLyrics> TimingCtrl::GetTimedLyrics() const {
    Vector<TimeLyrics> lyrics{ TimeLyrics(duration, "") };
    for (int i = 1; i < lines.GetCount() - 1; i++) {
        String l = lines[i].GetLyrics();
        lyrics.AddPick(TimeLyrics(lines[i].GetPosition(), TrimBoth(l)));
    }
    return lyrics;
}

void TimingCtrl::Adjust(double ms) {
    double secs = ms/1000.f;
    for (int i = 1; i <= timed; i++) {
        double newPos = lines[i].GetPosition() + secs;
        if (newPos < 0.0f) newPos = 0.0f;
        if (newPos > duration) newPos = duration;
        lines[i].SetPosition(newPos);
    }
    WhenDirty();
}

void TimingCtrl::SetTimedLyrics(const Vector<TimeLyrics>& timedLyrics, double duration) {
    this->duration = duration;
    lyrics.Clear();
    for (int i = 1; i < timedLyrics.GetCount(); ++i) {
        lyrics.Add(timedLyrics[i].lyrics);
    }
    RemoveLinesFromList();
    LyricsToLines();
    timed = 0;
    for (int i = timedLyrics.GetCount() - 1; i > 0; --i) {
        lines[i].SetPosition(timedLyrics[i].time);
        if (timed == 0 && timedLyrics[i].time > 0.0f) timed = i;
        lines[i].EnableTimeBtns(timed > 0);
    }
    AddLinesToList();
    ReadyForTiming();
    listPos = 0;
    if (timed == timedLyrics.GetCount() - 1) {
        WhenDoneTiming();
    }
    else {
        listPos = timed + 1;
    }
}

void TimingCtrl::PlayBackDone() {
    // Last line not timed. Let's get it over with.
    if (timed == lines.GetCount() - 3) {
        lines[++timed].SetPosition(duration);
        lines[timed].EnableTimeBtns();
        WhenDirty();
        WhenDoneTiming();
    }
}
