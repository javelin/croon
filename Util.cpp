/*
 * File  : Util.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

#define SRT_PATTERN "(\\d+)\n(.+) -->.+\n(.+)\n"

// Reds
#define RED_H               "&H000000FF"
#define RED_D               "&H00000080"
#define STRAWBERRY_RED_C    "&H001612F8"
#define STRAWBERRY_RED_H    "&H005350FA"
#define STRAWBERRY_RED_D    "&H004644D9"

// Greens
#define GREEN_H             "&H0000FF00"
#define GREEN_D             "&H00008000"
#define MALACHITE_C         "&H0059F907"
#define MALACHITE_H         "&H0051DA0B"
#define MALACHITE_D         "&H0040AA0A"

// Blues
#define BLUE_H              "&H00FF0000"
#define BLUE_D              "&H00800000"
#define LIGHT_BLUE_C        "&H00FF0000"
#define LIGHT_BLUE_H        "&H00FFD590"
#define LIGHT_BLUE_D        "&H00FFB957"

// Purples
#define PURPLE_H            "&H00FF00FF"
#define PURPLE_D            "&H00800080"
#define ORCHID_C            "&H00ED34F4"
#define ORCHID_H            "&H00E980ED"
#define ORCHID_D            "&H00C66DC9"

// Yellows
#define YELlOW_H            "&H0000FFFF"
#define YELlOW_D            "&H00008080"
#define MAIZE_C             "&H0008E4FB"
#define MAIZE_H             "&H005DECFB"
#define MAIZE_D             "&H0053CFDC"

#define GRAY                "&H00808080"


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
    String decor;
    while (line.StartsWith(">")) {
        decor += '>';
        line.Remove(0, 1);
    }
    RegExp rxp("{\\s*(\\d*)\\s*}(.*)");
    if (rxp.Study() && rxp.Match(line)) {
        decor += "{" + rxp.GetString(0) + "}";
        line = rxp.GetString(1);
    }
    line = TrimBoth(line);
    return decor;
}

Vector<TimeLyrics> RawToUntimedLyrics(const KarData& data) {
    Vector<TimeLyrics> lyrics{TimeLyrics(data.duration, "")};
    int lastLF = 0;
    int pos = -1;
    
    while ((pos = data.rawLyrics.Find('\n', lastLF)) > -1) {
        auto line = data.rawLyrics.Mid(lastLF, pos - lastLF);
        lastLF = pos + 1;
        line = TrimBoth(line);
        auto decor = SplitLyrics(line);
        if (!line.IsEmpty()) {
            String _line = "";
            Vector<String> split = Split(line, ' ');
            for (int i = 0; i < split.GetCount(); i++) {
                if (_line.IsEmpty()) {
                    _line = split[i];
                }
                else if (_line.GetLength() + split[i].GetLength() + 1 > MaxLineLength) {
                    lyrics.Add(TimeLyrics(0.0f, decor + TrimBoth(_line)));
                    decor = "";
                    _line = split[i];
                }
                else {
                    _line += " ";
                    _line += split[i];
                }
            }
            if (!_line.IsEmpty()) {
                lyrics.Add(TimeLyrics(0.0f, decor + TrimBoth(_line)));
            }
        }
    }
    auto line = data.rawLyrics.Right(data.rawLyrics.GetLength() - lastLF);
    line = TrimBoth(line);
    auto decor = SplitLyrics(line);
    if (!line.IsEmpty()) {
        String _line = "";
        Vector<String> split = Split(line, ' ');
        for (int i = 0; i < split.GetCount(); i++) {
            if (_line.IsEmpty()) {
                _line = split[i];
            }
            else if (_line.GetLength() + split[i].GetLength() + 1 > MaxLineLength) {
                lyrics.Add(TimeLyrics(0.0f, decor + TrimBoth(_line)));
                decor = "";
                _line = split[i];
            }
            else {
                _line += " ";
                _line += split[i];
            }
        }
        if (!_line.IsEmpty()) {
            lyrics.Add(TimeLyrics(0.0f, decor + TrimBoth(_line)));
        }
    }
    
    return lyrics;
}

String TimedLyricsToRaw(const Vector<TimeLyrics>& vtl, bool removeMetadata) {
    Vector<String> vs;
    for (const auto& tl : vtl) {
        if (vtl.GetIndex(tl) == 0) continue;
        if (removeMetadata) {
            auto lyrics = tl.lyrics;
            if (lyrics == "-") lyrics = "";
            else (void)SplitLyrics(lyrics);
            if (!lyrics.IsEmpty() && !lyrics.StartsWith("@")) vs.Add(lyrics);
        }
        else vs.Add(tl.lyrics);
    }
    return Join(vs, "\n");
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
    if (data.timedLyrics.IsEmpty()) return "";
    Vector<TimeLyrics> vtl;
    ProcessMetadata(data, vtl, linesToDisplay);
    
    // Style definition macro: Name, PrimaryColour (H), SecondaryColour (D)
    // ASS color format: &H00BBGGRR
    #define ASS_STYLE(name, primary, secondary, bold) \
        Format("Style: " name ",Arial,%d," primary "," secondary ",&H00000000,&H64000000,%d,0,0,0,100,100,0,0,1,2,1,2,10,10,120,1", fontSize, bold)
    
    auto S = [&](const char* name, const char* primary, const char* secondary, bool bold=true) {
        auto size = bold ? data.fontSize:(int)(data.fontSize*0.7);
        return Format("Style: %s,Arial,%d,%s,%s,&H00000000,&H64000000,%d,0,0,0,100,100,0,0,1,2,1,2,10,10,120,1",
                        name, size, primary, secondary, bold ? -1:0);
    };
    
    Vector<String> vs{
        "[Script Info]",
        Format("Title: %s by %s", data.title, data.artist),
        "ScriptType: v4.00",
        Format("PlayResY: %d", resY),
        Format("PlayResX: %d", resX),
        "Timer: 100.0000",
        "[V4+ Styles]",
        "Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, "
           "Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, "
           "Alignment, MarginL, MarginR, MarginV, Encoding",
        // V1: red
        S("V1",       STRAWBERRY_RED_H, STRAWBERRY_RED_D),
        S("V1Normal", STRAWBERRY_RED_D, STRAWBERRY_RED_H, false),
        // V2: blue
        S("V2",       LIGHT_BLUE_H, LIGHT_BLUE_D),
        S("V2Normal", LIGHT_BLUE_D, LIGHT_BLUE_H, false),
        // B (both): purple (V1 red + V2 blue)
        S("B",        ORCHID_H, ORCHID_D),
        S("BNormal",  ORCHID_D, ORCHID_H, false),
        // BUC (backup, ~): green
        S("BUC",       MALACHITE_H, MALACHITE_D),
        S("BUCNormal", MALACHITE_D, MALACHITE_H, false),
        // MC (miscellaneous, (...)): yellow
        S("MC",       MAIZE_H, MAIZE_D),
        S("MCNormal", MAIZE_D, MAIZE_H, false),
        // Count-in styles inherit vocal color: D for unbeaten, H for beaten
        S("CountInV1", RED_H, STRAWBERRY_RED_D),
        S("CountInV2", BLUE_H, LIGHT_BLUE_D),
        S("CountInB",  PURPLE_H, ORCHID_D),
        // Grayed: already displayed lines
        S("Grayed",   GRAY, GRAY),
        "",
        "[Events]",
        "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text"
    };
    #undef ASS_STYLE
    
    String lastLine{""};
    for (int i = 1; i < vtl.GetCount(); ++i) {
        const auto& tl = vtl[i];
        
        auto startTS = tl.time;
        auto endTS = startTS + 5.0f;
        String line = TrimBoth(tl.lyrics);
        if (line == "\u00A0" && lastLine == "\u00A0") continue;
        bool hasCountIn = line.StartsWith("@CountIn");
        line.Replace("@CountIn", "");
        if (hasCountIn) lastLine.Clear();
        if (i + 1 < vtl.GetCount()) endTS = vtl[i + 1].time;
        
        // Resolve vocal part for this line
        String incomingLyrics;
        VocalPart part = hasCountIn
            ? LookaheadVocalPart(vtl, i + 1, data.parts, incomingLyrics)
            : ResolveVocalPart(tl.partIndex, data.parts);
        
        // Add upcoming/preview lines in reverse order.
        // ASS codec will render in the correct order.
        for (int j = max(1, linesToDisplay - (lastLine.IsEmpty() ? 1:2)); j > 0; --j) {
            if (i + j < vtl.GetCount()) {
                const auto& ntl = vtl[i + j];
                String nextLine = TrimBoth(ntl.lyrics);
                if (nextLine.StartsWith("@CountIn")) nextLine = "\u00A0";
                nextLine.Replace("\\(", "(");
                nextLine.Replace("\\)", ")");
                VocalPart nextPart = ResolveVocalPart(ntl.partIndex, data.parts);
                String dimStyle = ResolveDimStyle(nextPart, nextLine, ntl.isMeta);
                vs.AddPick(Format("Dialogue: 0,%s,%s,%s,,0,0,0,,%s%s",
                                    FormatTimeASS(startTS),
                                    FormatTimeASS(endTS),
                                    dimStyle,
                                    hasCountIn ? "":"{\\fad(150,100)}",
                                    nextLine));
            }
        }
        
        // Currently singing line
        String singLine = line;
        singLine.Replace("\\(", "(");
        singLine.Replace("\\)", ")");
        String hilite = ResolveStyle(part, singLine, hasCountIn, incomingLyrics, tl.isMeta);
        vs.AddPick(Format("Dialogue: 0,%s,%s,%s,,0,0,0,,%s",
                            FormatTimeASS(startTS),
                            FormatTimeASS(endTS),
                            hilite,
                            singLine));
        
        // Previously sung line (grayed)
        if (!lastLine.IsEmpty()) {
            String grayLine = lastLine;
            grayLine.Replace("\\(", "(");
            grayLine.Replace("\\)", ")");
            // strip ~ and (...) markers but keep italic wrapping for grayed display
            if (grayLine.StartsWith("~")) {
                grayLine.Remove(0);
                grayLine = Format("{\\i1}%s{\\i0}", grayLine);
            }
            else if (grayLine.StartsWith("(")) {
                grayLine = Format("{\\i1}%s{\\i0}", grayLine);
            }
            else if (grayLine.StartsWith("@")) {
                grayLine = grayLine.Mid(1);
            }
            vs.AddPick(Format("Dialogue: 0,%s,%s,Grayed,,0,0,0,,%s",
                                FormatTimeASS(startTS),
                                FormatTimeASS(endTS),
                                grayLine));
        }
        lastLine = hasCountIn ? "\u00A03... 2... 1...":line;
    }
    return Join(vs, "\n");
}

String TimedToRichASS(const KarData& data, int linesToDisplay, int resX, int resY) {
    if (data.timedLyrics.IsEmpty()) return "";
    Vector<TimeLyrics> vtl;
    ProcessMetadata(data, vtl, linesToDisplay);
    RTHelper rth;
    rth.Fmt("@4").Text("[Script Info]").EFmt().NL();
    rth.Fmt("@3").Text("Title: ").EFmt("@c").Text(Format("%s by %s", data.title, data.artist)).EFmt().NL();
    rth.Fmt("@3").Text("ScriptType: ").EFmt("@c").Text("v4.00").EFmt().NL();
    rth.Fmt("@3").Text("PlayResY: ").EFmt("@c").Text(IntStr(resY)).EFmt().NL();
    rth.Fmt("@3").Text("PlayResX: ").EFmt("@c").Text(IntStr(resX)).EFmt().NL();
    rth.Fmt("@3").Text("Timer: ").EFmt("@c").Text("100.0000").EFmt().NL();
    rth.Fmt("@4").Text("[V4+ Styles]").EFmt().NL();
    rth.Fmt("@3").Text("Format: ").EFmt("@c").Text("Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, "
           "Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, "
           "Alignment, MarginL, MarginR, MarginV, Encoding").EFmt().NL();
    
    // Helper lambda to emit a style line into the RTHelper
    auto S = [&](const char* name, const char* primary, const char* secondary, bool bold=true) {
        auto size = bold ? data.fontSize:(int)(data.fontSize*0.7);
        rth.Fmt("@3").Text("Style: ").EFmt("@c")
            .Text(Format("%s,Arial,%d,%s,%s,&H00000000,&H64000000,%d,0,0,0,100,100,0,0,1,2,1,2,10,10,120,1",
                            name, size, primary, secondary, bold ? -1:0)).EFmt().NL();
    };
    /*
    // V1: red
    S("V1",         "&H000000FF", "&H00000080", -1);
    S("V1Normal",   "&H00000080", "&H00000080", 0);
    // V2: blue
    S("V2",         "&H00FF0000", "&H00800000", -1);
    S("V2Normal",   "&H00800000", "&H00800000", 0);
    // B (both): purple (V1 red + V2 blue)
    S("B",          "&H00FF0080", "&H00800040", -1);
    S("BNormal",    "&H00800040", "&H00800040", 0);
    // BUC (backup, ~): green
    S("BUC",        "&H0000FF00", "&H00008000", -1);
    S("BUCNormal",  "&H00008000", "&H00008000", 0);
    // MC (miscellaneous, (...)): yellow
    S("MC",         "&H0000FFFF", "&H00008080", -1);
    S("MCNormal",   "&H00008080", "&H00008080", 0);
    // Count-in styles inherit vocal color
    S("CountInV1",  "&H000000FF", "&H00000080", -1);
    S("CountInV2",  "&H00FF0000", "&H00800000", -1);
    S("CountInB",   "&H00FF0080", "&H00800040", -1);
    // Grayed: already displayed
    S("Grayed",     "&H00505050", "&H00505050", -1);
    */
    
    // V1: red
    S("V1",       STRAWBERRY_RED_H, STRAWBERRY_RED_D);
    S("V1Normal", STRAWBERRY_RED_D, STRAWBERRY_RED_H, false);
    // V2: blue
    S("V2",       LIGHT_BLUE_H, LIGHT_BLUE_D);
    S("V2Normal", LIGHT_BLUE_D, LIGHT_BLUE_H, false);
    // B (both): purple (V1 red + V2 blue)
    S("B",        ORCHID_H, ORCHID_D);
    S("BNormal",  ORCHID_D, ORCHID_H, false);
    // BUC (backup, ~): green
    S("BUC",       MALACHITE_H, MALACHITE_D);
    S("BUCNormal", MALACHITE_D, MALACHITE_H, false);
    // MC (miscellaneous, (...)): yellow
    S("MC",       MAIZE_H, MAIZE_D);
    S("MCNormal", MAIZE_D, MAIZE_H, false);
    // Count-in styles inherit vocal color: D for unbeaten, H for beaten
    S("CountInV1", STRAWBERRY_RED_H, STRAWBERRY_RED_D);
    S("CountInV2", LIGHT_BLUE_H, LIGHT_BLUE_D);
    S("CountInB",  ORCHID_H, ORCHID_D);
    // Grayed: already displayed lines
    S("Grayed",   GRAY, GRAY);
    
    rth.NL();
    rth.Fmt("@4").Text("[Events]").EFmt().NL();
    rth.Fmt("@3").Text("Format: ").EFmt("@c").Text("Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text").EFmt().NL();
    
    String lastLine{""};
    for (int i = 1; i < vtl.GetCount(); ++i) {
        const auto& tl = vtl[i];
        
        auto startTS = tl.time;
        auto endTS = startTS + 5.0f;
        String line = TrimBoth(tl.lyrics);
        if (line == "\u00A0" && lastLine == "\u00A0") continue;
        bool hasCountIn = line.StartsWith("@CountIn");
        if (hasCountIn) lastLine.Clear();
        line.Replace("@CountIn", "");
        if (i + 1 < vtl.GetCount()) endTS = vtl[i + 1].time;
        
        // Resolve vocal part for this line
        String incomingLyrics;
        VocalPart part = hasCountIn
            ? LookaheadVocalPart(vtl, i + 1, data.parts, incomingLyrics)
            : ResolveVocalPart(tl.partIndex, data.parts);
        
        // Upcoming/preview lines (dimmed)
        for (int j = max(1, linesToDisplay - (lastLine.IsEmpty() ? 1:2)); j > 0; --j) {
            if (i + j < vtl.GetCount()) {
                const auto& ntl = vtl[i + j];
                String nextLine = TrimBoth(ntl.lyrics);
                if (nextLine.StartsWith("@CountIn")) nextLine = "\u00A0";
                nextLine.Replace("\\(", "(");
                nextLine.Replace("\\)", ")");
                VocalPart nextPart = ResolveVocalPart(ntl.partIndex, data.parts);
                String dimStyle = ResolveDimStyle(nextPart, nextLine, ntl.isMeta);
                rth.Fmt("@3").Text("Dialogue: ")
                    .EFmt("@c").Text("0,")
                    .EFmt("@6").Text(Format("%s,%s,", FormatTimeASS(startTS), FormatTimeASS(endTS)))
                    .EFmt("@c").Text(Format("%s,,0,0,0,,", dimStyle))
                    .EFmt("@m").Text(hasCountIn ? "":"{\\fad(150,100)}")
                    .EFmt("@0").Text(nextLine).EFmt().NL();
            }
        }
        
        // Currently singing line
        String singLine = line;
        singLine.Replace("\\(", "(");
        singLine.Replace("\\)", ")");
        String hilite = ResolveStyle(part, singLine, hasCountIn, incomingLyrics, tl.isMeta);
        rth.Fmt("@3").Text("Dialogue: ")
            .EFmt("@c").Text("0,")
            .EFmt("@6").Text(Format("%s,%s,", FormatTimeASS(startTS), FormatTimeASS(endTS)))
            .EFmt("@c").Text(Format("%s,,0,0,0,,", hilite))
            .EFmt("@0").Text(singLine).EFmt().NL();
            
        // Previously sung line (grayed)
        if (!lastLine.IsEmpty()) {
            String grayLine = lastLine;
            grayLine.Replace("\\(", "(");
            grayLine.Replace("\\)", ")");
            if (grayLine.StartsWith("~")) {
                grayLine.Remove(0);
                grayLine = Format("{\\i1}%s{\\i0}", grayLine);
            }
            else if (grayLine.StartsWith("(")) {
                grayLine = Format("{\\i1}%s{\\i0}", grayLine);
            }
            rth.Fmt("@3").Text("Dialogue: ")
                .EFmt("@c").Text("0,")
                .EFmt("@6").Text(Format("%s,%s,", FormatTimeASS(startTS), FormatTimeASS(endTS)))
                .EFmt("@c").Text("Grayed,,0,0,0,,")
                .EFmt("@0").Text(grayLine).EFmt().NL();
        }
        lastLine = hasCountIn ? "\u00A03... 2... 1...":line;
    }
    return rth.ToString();
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
