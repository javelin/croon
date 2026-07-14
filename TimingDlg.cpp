/*
 * File  : TimingDlg.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#include "KarData.h"
#include "LyricsTransformer.h"
#include "TimeFormatter.h"
#include "UiScaler.h"
#include "TimingLine.h"
#include "TimingCtrl.h"

#define LAYOUTFILE <Croon/CroonTimingDlg.lay>
#include <CtrlCore/lay.h>

#include "AppAudioPlayer.h"
#include "TimingDlg.h"

TimingDlg::TimingDlg() {
    CtrlLayout(*this, "Time Lyrics");
    Title("Time Lyrics").NoZoomable().Sizeable().SetMinSize(Size(UiScaler::X(400), UiScaler::Y(220)));
    SetRect(0, 0, UiScaler::X(640), UiScaler::Y(800));
    CenterOwner();
    playBtn.Ok();
    closeBtn.Cancel();

    sliderCtrl.WhenSlideFinish = [=] {
        double value = (double)sliderCtrl.GetData();
        AppAudioPlayer::Seek(value/100.0f*AppAudioPlayer::Duration());
    };
    timingCtrl.WhenDoneTiming << [=] {
    };
    timingCtrl.WhenInterrupt << [=] {
        if (AppAudioPlayer::IsPlaying()) {
            TogglePlay();
        }
    };
    timingCtrl.WhenPlayAt << [=] (auto position) {
        AppAudioPlayer::Seek(position);
        if (!AppAudioPlayer::IsPlaying()) {
            TogglePlay();
        }
    };
    timingCtrl.WhenDirty << [=] { dirty = true; };
    timingCtrl.WhenTiming << [=] (auto _) {
        return AppAudioPlayer::IsPlaying();
    };
    
    playBtn << [=] {
        TogglePlay();
        timingCtrl.SetFocus();
    };
    
    adjustEd.SetData(500);
    adjustDec << [=] {
        timingCtrl.Adjust(-((double)adjustEd.GetData()));
    };
    adjustInc << [=] {
        timingCtrl.Adjust((double)adjustEd.GetData());
    };
    
    closeBtn << [=] { Close(); };
}

void TimingDlg::Close() {
    AppAudioPlayer::Pause();
    if (PromptYesNoCancel("Save changes and close?") == 1) {
        data->timedLyrics = timingCtrl.GetTimedLyrics();
        data->rawLyrics = LyricsTransformer::TimedToRaw(data->timedLyrics);
        data->timed = timingCtrl.GetTimed();
        TopWindow::Close();
    }
}

int TimingDlg::Run(KarData& karData) {
    data = &karData;
    Populate();
    return TopWindow::Run();
}

bool TimingDlg::Key(dword key, int count) {
    switch (key) {
    case K_P:
        TogglePlay();
        return true;
    }
    return Ctrl::Key(key, count);
}

void TimingDlg::PollProgress() {
    SetTimeCallback(100, [=] {
        if (!AppAudioPlayer::IsOpen()) {
            playBtn.SetLabel("Play");
            AppAudioPlayer::Reopen();
            SetProgress(AppAudioPlayer::Duration());
            timingCtrl.PlayBackDone();
        }
        else if (AppAudioPlayer::IsPlaying()) {
            SetProgress(AppAudioPlayer::Position());
            PollProgress();
        }
    }, timerId);
}

void TimingDlg::Populate() {
    timeLbl.SetLabel("00:00.000/" + TimeFormatter::Format(data->duration));
    timingCtrl.SetTimedLyrics(data->timedLyrics, data->duration);
    timingCtrl.SetFocus();
}

void TimingDlg::SetProgress(double position) {
    auto duration = AppAudioPlayer::Duration();
    if (duration > 0.0f) {
        sliderCtrl.SetData((int)(position/duration*100.0));
        timingCtrl.SetMusicPosition(position, duration);
    }
    else {
        sliderCtrl.SetData(0);
        timingCtrl.SetMusicPosition(0.0f, 0.0f);
    }
    timeLbl.SetLabel((TimeFormatter::Format(position).Mid(3) + "/") + TimeFormatter::Format(duration).Mid(3));
}

void TimingDlg::TogglePlay() {
    if (!AppAudioPlayer::IsPlaying()) {
        timingCtrl.ReadyForTiming();
        AppAudioPlayer::Play();
        PollProgress();
        playBtn.SetLabel("Pause");
    }
    else {
        AppAudioPlayer::Pause();
        playBtn.SetLabel("Play");
    }
}
