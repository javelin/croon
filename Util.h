/*
 * File  : Util.h
 * Author: Mark Documento
 */

#ifndef _Croon_Util_h_
#define _Croon_Util_h_

struct RTHelper {
    RTHelper& Clear() { vs.Clear(); return *this; }
    RTHelper& Fmt(String s) { vs.Add(Format("[%s ", s)); return *this; }
    RTHelper& EFmt() { vs.Add("]"); return *this; }
    RTHelper& EFmt(String s) { vs.Add(Format("][%s ", s)); return *this; }
    RTHelper& NL() { vs.AddPick("&"); return *this; }
    RTHelper& Text(String s) { vs.Add(DeQtf(s)); return *this; }
    String ToString() const { return Join(vs, ""); }
private:
    Vector<String> vs;
};

String GetDataDirectory();
inline double CountInDuration(int bpm) {
    auto dur = 60.0f/bpm;
    return std::round(dur*100.0f)/100.0f;
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

enum VocalPart {
    VP_V1,
    VP_V2,
    VP_B
};

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
    int hr = (int)seconds/3600;
    int min = ((int)seconds / 60) % 60;
    double sec = std::fmod(seconds, 60);
    int ms = (sec - floor(sec))*(roundMs ? 100:1000);
    String tm = Format("%02d:%02d:%02d%c%0*d", hr, min, (int)sec, decimal, roundMs ? 2:3, ms);
    return tm;
}

inline String FormatTimeASS(double seconds) {
	return FormatTime(seconds, true).Mid(1);
}

inline String FormatTimeSRT(double seconds) {
    return FormatTime(seconds, true, ',');
}

inline String FormatTime2(double seconds) {
    int hr = (int)seconds/3600;
    int min = ((int)seconds / 60) % 60;
    int sec = (int)seconds%60;
    return Format("%02d:%02d:%02d", hr, min, sec);
}

inline String CleanSpacing(const String& s) {
    auto vw = Split(s, ' ');
    for (int i = 0; i < vw.GetCount(); i++) {
        vw[i] = TrimBoth(vw[i]);
    }
    return Join(vw, " ");
}

inline String StripNonAlnum(String s) {
    String ret;
    for (auto c : s) if (IsAlNum(c)) ret += c;
    return ret;
}

inline String ShortenMiddle(String s, int max) {
    String ret = s;
    if (s.GetLength() > max) {
        ret = s.Left(max/2 - 2) + "...";
        ret += s.Right(max/2 - 2);
    }
    return ret;
}

int _Zx(int zx);
int _Zy(int zy);
bool ComputeFfmpegTs(String s, double& ts, String& formatted, String key="time=");
const Vector<String>& GetGenres();

#endif
