/*
 * File  : TimingLine.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

#include <iomanip>
#include <sstream>

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
#include "FfmpegCommandBuilder.h"
#include "LyricsTransformer.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ProjectLoader.h"
#include "TimeFormatter.h"
#include "TimingLine.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "TimingLineDlg.h"

const Color TimingLine::NColor{0xF4, 0xF4, 0xF4};
const Color TimingLine::H1Color{0xCB, 0xE7, 0xFF};
const Color TimingLine::H2Color{0xF8, 0xF3, 0xD9};

void TimingLine::Initialize(bool withButtons) {
    decor = LyricsTransformer::SplitDecorations(lyrics);
    WantFocus(false);
    decSecBtn.WantFocus(false);
    incSecBtn.WantFocus(false);
    EnableTimeBtns(false);
    if (!blank) {
        *this << timeLbl.SetLabel("").AlignVCenter().LeftPosZ(35, 65).VSizePosZ(5, 5)
                << lyricsLbl.SetLabel(decor + lyrics).AlignVCenter().HSizePosZ(115, 40).VSizePosZ(5, 5);
        if (withButtons) {
            *this << decSecBtn.SetLabel("-").LeftPosZ(10, 20).VCenterPosZ(20)
                    << incSecBtn.SetLabel("+").LeftPosZ(90, 20).VCenterPosZ(20)
                    << retimeBtn.SetLabel("R").RightPosZ(10, 20).VCenterPosZ(20);
        }
        AddFrame(frame);
        DisplayTime();
        Normal();
        decSecBtn << [=] {
            double pos = position - 0.1f;
            pos = pos < 0.0f ? 0.0f:pos;
            if (WhenSetPosition(pos)) {
                position = pos;
                DisplayTime();
            }
        };
        incSecBtn << [=] {
            double pos = position + 0.1f;
            pos = pos > duration ? duration:pos;
            if (WhenSetPosition(pos)) {
                position = pos;
                DisplayTime();
            }
        };
        retimeBtn << [=] {
            WhenInterrupt();
            if (PromptYesNoCancel("Reset timing from this line?") > 0) {
                WhenRetime();
            }
        };
    }
}

void TimingLine::Paint(Draw& w) {
    if (blank) {
        Ctrl::Paint(w);
    }
    else {
        Size sz = GetSize();
        w.DrawRect(sz, bgColor);
    }
}

void TimingLine::LeftUp(Point p, dword) {
    WhenLeftClicked();
}

void TimingLine::LeftDouble(Point pos, dword) {
    if (blank || !withButtons) return;
    WhenEditingLyrics();
    TimingLineDlg tlDlg(decor, lyrics);
    if (tlDlg.ExecuteOK()) {
        decor = tlDlg.GetDecor();
        lyrics = tlDlg.GetLyrics();
        lyricsLbl.SetLabel(decor + lyrics);
        WhenLyricsEdited(lyrics);
    }
}

void TimingLine::DisplayTime() {
    timeLbl.SetLabel(TimeFormatter::Format(position).Mid(3));
}

void TimingLine::Normal() {
    frame.SetColor(Black());
    frame.SetMargins(Rect(1, 1, 1, 1));
    bgColor = NColor;
    Refresh();
}

void TimingLine::Highlight1() {
    frame.SetColor(LtRed());
    frame.SetMargins(Rect(3, 3, 3, 3));
    bgColor = H1Color;
    Refresh();
}

void TimingLine::Highlight2() {
    frame.SetColor(Black());
    frame.SetMargins(Rect(1, 1, 1, 1));
    bgColor = H2Color;
    Refresh();
}

void TimingLine::SetPosition(double position) {
    this->position = position > duration ? duration:position;
    if (this->position < 0.0f)
        this->position = 0.0f;
    DisplayTime();
}

void TimingLine::EnableTimeBtns(bool enable) {
    decSecBtn.Enable(enable);
    incSecBtn.Enable(enable);
    retimeBtn.Enable(enable);
}

void TimingLine::ResetTiming() {
    SetPosition(0.0f);
    EnableTimeBtns(false);
    WhenTimeButtonsDisabled();
    DisplayTime();
    Normal();
}
