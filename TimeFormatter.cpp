/*
 * File  : TimeFormatter.cpp
 * Author: Mark Documento
 */

#include <Core/Core.h>

#include <cmath>

using namespace Upp;

#include "TimeFormatter.h"

double TimeFormatter::CountInDuration(int bpm) {
    auto dur = 60.0f/bpm;
    return std::round(dur*100.0f)/100.0f;
}

String TimeFormatter::Format(double seconds, bool roundMs, char decimal) {
    int hr = (int)seconds/3600;
    int min = ((int)seconds / 60) % 60;
    double sec = std::fmod(seconds, 60);
    int ms = (sec - floor(sec))*(roundMs ? 100:1000);
    String tm = Upp::Format("%02d:%02d:%02d%c%0*d", hr, min, (int)sec, decimal, roundMs ? 2:3, ms);
    return tm;
}

String TimeFormatter::Ass(double seconds) {
    return Format(seconds, true).Mid(1);
}

String TimeFormatter::Srt(double seconds) {
    return Format(seconds, true, ',');
}

String TimeFormatter::Clock(double seconds) {
    int hr = (int)seconds/3600;
    int min = ((int)seconds / 60) % 60;
    int sec = (int)seconds%60;
    return Upp::Format("%02d:%02d:%02d", hr, min, sec);
}
