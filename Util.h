/*
 * File  : Util.h
 * Author: Mark Documento
 */

#ifndef _Croon_Util_h_
#define _Croon_Util_h_

#include "RichTextBuilder.h"
#include "SubtitleLineProcessor.h"
#include "TextTools.h"
#include "TimeFormatter.h"

String GetDataDirectory();
inline double CountInDuration(int bpm) {
    return TimeFormatter::CountInDuration(bpm);
}
String SplitLyrics(String& lyrics);
String TimedLyricsToRaw(const Vector<TimeLyrics>& vtl, bool removeMetadata=false);
Vector<TimeLyrics> RawToUntimedLyrics(const KarData& data);
String& ReplaceMetadata(String& line, const KarData& data, bool replaceDash=true);
void ProcessMetadata(const KarData& data, Vector<TimeLyrics>& vtl, int linesToDisplay);
String TimedToASS(const KarData& data, int linesToDisplay=DefaultASSDisplayLines, int resX=1920, int resY=1080);
String TimedToRichASS(const KarData& data, int linesToDisplay=DefaultASSDisplayLines, int resX=1920, int resY=1080);
Vector<String> GetPaths(String dir, String pattern);
bool DownloadLyrics(String title, String artist, String& lyrics);

VocalPart ResolveVocalPart(int partIndex,
                                   const Vector<Tuple<int,bool,bool,bool>>& parts);
String ResolveCountInStyle(VocalPart part, const String& incomingLyrics);
String ResolveStyle(VocalPart part, String& line, bool isCountIn,
                           const String& incomingLyrics="", bool isMeta=false);
String ResolveDimStyle(VocalPart part, String& line, bool isMeta=false);
VocalPart LookaheadVocalPart(const Vector<TimeLyrics>& vtl, int startIdx,
                                     const Vector<Tuple<int,bool,bool,bool>>& parts,
                                     String& outLyrics);

inline String FormatTime(double seconds, bool roundMs=false, char decimal='.') {
    return TimeFormatter::Format(seconds, roundMs, decimal);
}

inline String FormatTimeASS(double seconds) {
    return TimeFormatter::Ass(seconds);
}

inline String FormatTimeSRT(double seconds) {
    return TimeFormatter::Srt(seconds);
}

inline String FormatTime2(double seconds) {
    return TimeFormatter::Clock(seconds);
}

inline String CleanSpacing(const String& s) {
    return TextTools::CleanSpacing(s);
}

inline String StripNonAlnum(String s) {
    return TextTools::StripNonAlnum(s);
}

inline String ShortenMiddle(String s, int max) {
    return TextTools::ShortenMiddle(s, max);
}

int _Zx(int zx);
int _Zy(int zy);
bool ComputeFfmpegTs(String s, double& ts, String& formatted, String key="time=");
const Vector<String>& GetGenres();

#endif
