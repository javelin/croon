/*
 * File  : Util.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

#define SRT_PATTERN "(\\d+)\n(.+) -->.+\n(.+)\n"

String GetDataDirectory() {
    static String dataDir = AppendFileName(GetAppDataFolder(),
#ifdef PLATFORM_POSIX
                                           AppIdentity::PosixDataDirectory()
#else
                                           AppIdentity::DataDirectory()
#endif
                                           );
    if (!DirectoryExists(dataDir)) DirectoryCreate(dataDir);
    return dataDir;
}

String SplitLyrics(String& line) {
    return LyricsTransformer::SplitDecorations(line);
}

Vector<TimeLyrics> RawToUntimedLyrics(const KarData& data) {
    return LyricsTransformer::RawToUntimed(data);
}

String TimedLyricsToRaw(const Vector<TimeLyrics>& vtl, bool removeMetadata) {
    return LyricsTransformer::TimedToRaw(vtl, removeMetadata);
}

String& ReplaceMetadata(String& line, const KarData& data, bool replaceDash) {
    auto copyright{data.owner.IsEmpty() ? String(""):
                                        Format("Copyright \u00A9 %s%s",
                                                data.year > 0 ? IntStr(data.year) + " ":"",
                                                data.owner)};
    if (replaceDash && line == "-") line = "\u00A0";
    else {
        static auto escape = [](const auto &text) {
            auto escaped{text};
            escaped.Replace("(", "\\(");
            escaped.Replace(")", "\\)");
            return escaped;
        };
        line.Replace("@Title", escape(data.title));
        line.Replace("@Artist", escape(data.artist));
        line.Replace("@Genre", escape(data.genre));
        line.Replace("@Writer", escape(data.writer));
        line.Replace("@Copyright", escape(copyright));
        line.Replace("@Owner", escape(data.owner));
    }
    return line;
}

void ProcessMetadata(const KarData& data, Vector<TimeLyrics>& vtl, int linesToDisplay) {
    vtl.Clear();
    vtl.Add({data.duration, "", -1});
    static const auto blankLine = "\u00A0";
    static const auto defaultTempo = 60; // 60bpm
    double lastTs = 0.0f;
    int maxBlanks = linesToDisplay - 1;
    // Use indexed loop to preserve original index for partIndex tracking
    for (int idx = 1; idx < data.timedLyrics.GetCount(); ++idx) {
        const auto& vt = data.timedLyrics[idx];
        auto lyrics = vt.lyrics;
        auto ts = vt.time;
        auto blanks = 0;
        auto bpm = 0;
        RegExp rxBlank{"^(>+)(.+)"};
        if (rxBlank.Study() && rxBlank.Match(lyrics)) {
            blanks = min(rxBlank.GetString(0).GetLength(), maxBlanks);
            lyrics = TrimBoth(rxBlank.GetString(1));
        }
        RegExp rxTempo{"^{(.*)}(.+)"};
        if (rxTempo.Study() && rxTempo.Match(lyrics)) {
            lyrics = TrimBoth(rxTempo.GetString(1));
            auto tempo = rxTempo.GetString(0);
            bpm = StrInt(tempo.IsEmpty() ? "0":tempo);
            if (bpm == 0) {
                bpm = defaultTempo;
            }
            else if (bpm < 0 || bpm > 300) {
                bpm = 0;
            }
        }
        if (lyrics.IsEmpty()) continue;
        if (bpm > 0) {
            auto countIn = CountInDuration(bpm);
            auto totalDec = countIn*3 + CountInDelay;
            if (ts - totalDec > lastTs) { //Is there enough time for a count in?
                for (int i = 0; i < blanks; ++i) vtl.Add({ts - totalDec, blankLine, -1});
                int csec = countIn*100;
                vtl.Add({ts - totalDec, Format("@CountIn{\\k%d}\u00A0{\\k%d}3... {\\k%d}2... {\\k%d}1...",
                                                (int)CountInDelay*100, csec, csec, csec), -1});
            }
            else {
                for (int i = 0; i < blanks; ++i) vtl.Add({ts, blankLine, -1});
            }
        }
        else {
            for (int i = 0; i < blanks; ++i) vtl.Add({ts, blankLine, -1});
        }
        // Actual lyric line carries the original timedLyrics index
        // \@ is an escaped @ — treat as normal vocal line, strip the backslash
        // Any other @ prefix marks a metadata/special line rendered in MC
        bool isMeta = false;
        if (lyrics.StartsWith("\\@")) {
            lyrics.Remove(0, 1); // strip the backslash, leave the @ as literal text
        }
        else if (lyrics.StartsWith("@")) {
            isMeta = true;
        }
        vtl.Add({ts, ReplaceMetadata(lyrics, data), idx, isMeta});
        lastTs = ts;
    }
}

// ─── Vocal part helpers ───────────────────────────────────────────────────────

VocalPart ResolveVocalPart(int partIndex,
                                   const Vector<Tuple<int,bool,bool,bool>>& parts) {
    if (partIndex < 0) return VP_V1;
    for (const auto& p : parts) {
        if (p.a == partIndex) {
            bool v1 = p.b;
            bool v2 = p.c;
            if (v1 && v2) return VP_B;
            if (v2)       return VP_V2;
            return VP_V1;
        }
    }
    return VP_V1;
}

// Returns the count-in style name based on the incoming line's vocal part,
// respecting ~ and (...) overrides.
String ResolveCountInStyle(VocalPart part, const String& incomingLyrics) {
    if (incomingLyrics.StartsWith("~"))  return "BUC";
    if (incomingLyrics.StartsWith("("))  return "MC";
    switch (part) {
        case VP_V2: return "CountInV2";
        case VP_B:  return "CountInB";
        default:    return "CountInV1";
    }
}

// Resolves the ASS style name and applies italic wrapping for BUC and MC.
String ResolveStyle(VocalPart part, String& line, bool isCountIn,
                            const String& incomingLyrics, bool isMeta) {
    if (isCountIn) return ResolveCountInStyle(part, incomingLyrics);
    // @ prefix: metadata/special line, rendered as MC; strip @ if still present
    if (isMeta) {
        if (line.StartsWith("@")) line.Remove(0, 1);
        return "MC";
    }
    // ~ prefix: backup vocals, italic, overrides parts assignment
    if (line.StartsWith("~")) {
        line.Remove(0);
        line = Format("{\\i1}%s{\\i0}", line);
        return "BUC";
    }
    // (...) enclosure: miscellaneous, italic, overrides parts assignment
    if (line.StartsWith("(")) {
        line = Format("{\\i1}%s{\\i0}", line);
        return "MC";
    }
    switch (part) {
        case VP_V2: return "V2";
        case VP_B:  return "B";
        default:    return "V1";
    }
}

// For upcoming/preview lines the dimmed (Normal) style variant is used.
String ResolveDimStyle(VocalPart part, String& line, bool isMeta) {
    // @ prefix: metadata/special line, rendered as MC; strip @ if still present
    if (isMeta) {
        if (line.StartsWith("@")) line.Remove(0, 1);
        return "MCNormal";
    }
    if (line.StartsWith("~")) {
        line.Remove(0);
        line = Format("{\\i1}%s{\\i0}", line);
        return "BUCNormal";
    }
    if (line.StartsWith("(")) {
        line = Format("{\\i1}%s{\\i0}", line);
        return "MCNormal";
    }
    switch (part) {
        case VP_V2: return "V2Normal";
        case VP_B:  return "BNormal";
        default:    return "V1Normal";
    }
}

// Scans forward in vtl from startIdx to find the VocalPart of the next real
// (non-inserted) line. Used to determine count-in color.
// outLyrics is set to the found line's lyrics so the caller can detect overrides.
VocalPart LookaheadVocalPart(const Vector<TimeLyrics>& vtl, int startIdx,
                                     const Vector<Tuple<int,bool,bool,bool>>& parts,
                                     String& outLyrics) {
    for (int k = startIdx; k < vtl.GetCount(); ++k) {
        if (vtl[k].partIndex >= 0) {
            outLyrics = TrimBoth(vtl[k].lyrics);
            return ResolveVocalPart(vtl[k].partIndex, parts);
        }
    }
    outLyrics = "";
    return VP_V1;
}

String TimedToASS(const KarData& data, int linesToDisplay, int resX, int resY) {
    return SubtitleGenerator::ToAss(data, linesToDisplay, resX, resY);
}

String TimedToRichASS(const KarData& data, int linesToDisplay, int resX, int resY) {
    return SubtitleGenerator::ToRichAss(data, linesToDisplay, resX, resY);
}

Vector<String> GetPaths(String dir, String pattern) {
    Vector<String> paths;
    for (FindFile ff(dir + "/*.*"); ff; ff++) {
        String p = ff.GetPath();
        if(PatternMatchMulti(pattern, ff.GetName()) && ff.IsFile()) {
            paths.Add(ff.GetPath());
        }
    }
    return paths;
}

bool DownloadLyrics(String title, String artist, String& lyrics) {
    String t = Join(Split(CleanSpacing(ToLower(StripNonAlnum(TrimBoth(title)))), ' '), "");
    String a = ToLower(StripNonAlnum(TrimBoth(artist)));
    a.TrimStart("the ");
    a = Join(Split(CleanSpacing(a), ' '), "");
    String url = Format(AZ_URL, a, t);
    DownloadDlg dlg;
    String* _lyrics = &lyrics;
    dlg.WhenDownloadSuccess << [_lyrics](String content) {
        static RegExp rx(AZ_PATTERN, RegExp::DOTALL | RegExp::MULTILINE | RegExp::UTF8);
        if (rx.Study() && rx.Match(content)) {
            auto vl = Split(rx.GetString(0), "<br>");
            for (int i = 0; i < vl.GetCount(); i++) {
                vl[i] = TrimBoth(vl[i]);
            }
            *_lyrics = Join(vl, "\n");
        }
        else {
            *_lyrics = "";
        }
    };
    return dlg.Run(url, "Downloading lyrics") == IDOK;
}

int _Zx(int zx) {
    static int mx = 0, dx = 0;
    if (!mx || !dx) {
        Size m, d;
        Ctrl::GetZoomRatio(m, d);
        mx = m.cx;
        dx = d.cx;
    }
    return zx*dx/mx;
}

int _Zy(int zy) {
    static int my = 0, dy = 0;
    if (!my || !dy) {
        Size m, d;
        Ctrl::GetZoomRatio(m, d);
        my = m.cy;
        dy = d.cy;
    }
    return zy*dy/my;
}

bool ComputeFfmpegTs(String s, double& ts, String& formatted, String key) {
    TrimBoth(s);
    int pos = s.Find(key);
    if (pos > -1) {
        auto tsStr = s.Mid(pos + key.GetLength(), 11);
        auto vs = Split(tsStr, ':');
        if (vs.GetCount() < 3) return false;
        ts = StrDbl(vs[0])*360.0f;
        ts += StrDbl(vs[1])*60.0f;
        ts += StrDbl(vs[2]);
        formatted = s.Mid(pos + 5, 11);
        return true;
    }
    return false;
}

const Vector<String>& GetGenres() {
    static Vector<String> genre{
        "Ballad",
        "Blues",
        "Blues Rock",
        "Country",
        "Folk",
        "Heavy Metal",
        "Hip-Hop",
        "Pop Standard",
        "Power Ballad",
        "OPM",
        "Metal",
        "Pop",
        "R&B",
        "Rock",
        "Rock n Roll",
        "Soft Rock"
    };
    return genre;
}
