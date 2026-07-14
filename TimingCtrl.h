/*
 * File  : TimingCtrl.h
 * Author: Mark Documento
 */

#ifndef _Croon_TimingCtrl_h_
#define _Croon_TimingCtrl_h_

#include "VocalPart.h"

class TimingCtrl : public Ctrl {
public:
    TimingCtrl(int lineHeight = 50);
    void MouseWheel(Point, int zdelta, dword) override {
        scrollBar.Wheel(zdelta); }
    void Layout() override {
        Ctrl::Layout(); scrollBar.SetPage(GetSize().cy); }
    bool Key(dword key, int) override;
    void Reset() { listPos = timed = 0; }
    void SetRawLyrics(const String& lyrics);
    void LyricsToLines();
    void AddLinesToList();
    void RemoveLinesFromList();
    void SetMusicPosition(double position, double duration);
    void SetFocusLine();
    void ScrollToLineAndCenter(int row);
    void ReadyForTiming();
    void PlayBackDone();
    
    String GetLyrics() const { return Join(lyrics, "\n"); }
    Vector<TimeLyrics> GetTimedLyrics() const;
    Vector<Tuple<int, bool, bool, bool>> GetParts() const;
    int GetTimed() const { return timed; }
    void SetTimedLyrics(const Vector<TimeLyrics>& timedLyrics, double duration);
    void SetParts(const Vector<Tuple<int, bool, bool, bool>>& parts);
    
    void Adjust(double ms);
    
    Event<> WhenDoneTiming;
    Event<> WhenInterrupt;
    Event<double> WhenPlayAt;
    Event<int> WhenRetime;
    Gate<double> WhenTiming;
    Event<> WhenDirty;
    
private:
    static const int timerId{1};
    int lineHeight;
    ScrollBar scrollBar;
    Array<TimingLine> lines;
    Vector<String> lyrics;
    double position;
    double duration;
    int listPos;
    int timed;
    bool editing{false};
    Vector<VocalPart> vocalParts;

    void ApplyPartsToLines();
};

#endif
