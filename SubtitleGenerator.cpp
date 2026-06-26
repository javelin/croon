/*
 * File  : SubtitleGenerator.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

// Reds
#define RED_H               "&H000000FF"
#define STRAWBERRY_RED_H    "&H005350FA"
#define STRAWBERRY_RED_D    "&H004644D9"

// Greens
#define MALACHITE_H         "&H0051DA0B"
#define MALACHITE_D         "&H0040AA0A"

// Blues
#define BLUE_H              "&H00FF0000"
#define LIGHT_BLUE_H        "&H00FFD590"
#define LIGHT_BLUE_D        "&H00FFB957"

// Purples
#define PURPLE_H            "&H00FF00FF"
#define ORCHID_H            "&H00E980ED"
#define ORCHID_D            "&H00C66DC9"

// Yellows
#define MAIZE_H             "&H005DECFB"
#define MAIZE_D             "&H0053CFDC"

#define GRAY                "&H00808080"

String SubtitleGenerator::ToAss(const KarData& data, int linesToDisplay, int resX, int resY) {
    if (data.timedLyrics.IsEmpty()) return "";
    Vector<TimeLyrics> vtl;
    SubtitleLineProcessor::ProcessMetadata(data, vtl, linesToDisplay);
    
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
        
        String incomingLyrics;
        VocalPart part = hasCountIn
            ? SubtitleLineProcessor::LookaheadVocalPart(vtl, i + 1, data.parts, incomingLyrics)
            : SubtitleLineProcessor::ResolveVocalPart(tl.partIndex, data.parts);
        
        for (int j = max(1, linesToDisplay - (lastLine.IsEmpty() ? 1:2)); j > 0; --j) {
            if (i + j < vtl.GetCount()) {
                const auto& ntl = vtl[i + j];
                String nextLine = TrimBoth(ntl.lyrics);
                if (nextLine.StartsWith("@CountIn")) nextLine = "\u00A0";
                nextLine.Replace("\\(", "(");
                nextLine.Replace("\\)", ")");
                VocalPart nextPart = SubtitleLineProcessor::ResolveVocalPart(ntl.partIndex, data.parts);
                String dimStyle = SubtitleLineProcessor::ResolveDimStyle(nextPart, nextLine, ntl.isMeta);
                vs.AddPick(Format("Dialogue: 0,%s,%s,%s,,0,0,0,,%s%s",
                                    TimeFormatter::Ass(startTS),
                                    TimeFormatter::Ass(endTS),
                                    dimStyle,
                                    hasCountIn ? "":"{\\fad(150,100)}",
                                    nextLine));
            }
        }
        
        String singLine = line;
        singLine.Replace("\\(", "(");
        singLine.Replace("\\)", ")");
        String hilite = SubtitleLineProcessor::ResolveStyle(part, singLine, hasCountIn, incomingLyrics, tl.isMeta);
        vs.AddPick(Format("Dialogue: 0,%s,%s,%s,,0,0,0,,%s",
                            TimeFormatter::Ass(startTS),
                            TimeFormatter::Ass(endTS),
                            hilite,
                            singLine));
        
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
            else if (grayLine.StartsWith("@")) {
                grayLine = grayLine.Mid(1);
            }
            vs.AddPick(Format("Dialogue: 0,%s,%s,Grayed,,0,0,0,,%s",
                                TimeFormatter::Ass(startTS),
                                TimeFormatter::Ass(endTS),
                                grayLine));
        }
        lastLine = hasCountIn ? "\u00A03... 2... 1...":line;
    }
    return Join(vs, "\n");
}

String SubtitleGenerator::ToRichAss(const KarData& data, int linesToDisplay, int resX, int resY) {
    if (data.timedLyrics.IsEmpty()) return "";
    Vector<TimeLyrics> vtl;
    SubtitleLineProcessor::ProcessMetadata(data, vtl, linesToDisplay);
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
    
    auto S = [&](const char* name, const char* primary, const char* secondary, bool bold=true) {
        auto size = bold ? data.fontSize:(int)(data.fontSize*0.7);
        rth.Fmt("@3").Text("Style: ").EFmt("@c")
            .Text(Format("%s,Arial,%d,%s,%s,&H00000000,&H64000000,%d,0,0,0,100,100,0,0,1,2,1,2,10,10,120,1",
                            name, size, primary, secondary, bold ? -1:0)).EFmt().NL();
    };
    
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
        
        String incomingLyrics;
        VocalPart part = hasCountIn
            ? SubtitleLineProcessor::LookaheadVocalPart(vtl, i + 1, data.parts, incomingLyrics)
            : SubtitleLineProcessor::ResolveVocalPart(tl.partIndex, data.parts);
        
        for (int j = max(1, linesToDisplay - (lastLine.IsEmpty() ? 1:2)); j > 0; --j) {
            if (i + j < vtl.GetCount()) {
                const auto& ntl = vtl[i + j];
                String nextLine = TrimBoth(ntl.lyrics);
                if (nextLine.StartsWith("@CountIn")) nextLine = "\u00A0";
                nextLine.Replace("\\(", "(");
                nextLine.Replace("\\)", ")");
                VocalPart nextPart = SubtitleLineProcessor::ResolveVocalPart(ntl.partIndex, data.parts);
                String dimStyle = SubtitleLineProcessor::ResolveDimStyle(nextPart, nextLine, ntl.isMeta);
                rth.Fmt("@3").Text("Dialogue: ")
                    .EFmt("@c").Text("0,")
                    .EFmt("@6").Text(Format("%s,%s,", TimeFormatter::Ass(startTS), TimeFormatter::Ass(endTS)))
                    .EFmt("@c").Text(Format("%s,,0,0,0,,", dimStyle))
                    .EFmt("@m").Text(hasCountIn ? "":"{\\fad(150,100)}")
                    .EFmt("@0").Text(nextLine).EFmt().NL();
            }
        }
        
        String singLine = line;
        singLine.Replace("\\(", "(");
        singLine.Replace("\\)", ")");
        String hilite = SubtitleLineProcessor::ResolveStyle(part, singLine, hasCountIn, incomingLyrics, tl.isMeta);
        rth.Fmt("@3").Text("Dialogue: ")
            .EFmt("@c").Text("0,")
            .EFmt("@6").Text(Format("%s,%s,", TimeFormatter::Ass(startTS), TimeFormatter::Ass(endTS)))
            .EFmt("@c").Text(Format("%s,,0,0,0,,", hilite))
            .EFmt("@0").Text(singLine).EFmt().NL();
            
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
                .EFmt("@6").Text(Format("%s,%s,", TimeFormatter::Ass(startTS), TimeFormatter::Ass(endTS)))
                .EFmt("@c").Text("Grayed,,0,0,0,,")
                .EFmt("@0").Text(grayLine).EFmt().NL();
        }
        lastLine = hasCountIn ? "\u00A03... 2... 1...":line;
    }
    return rth.ToString();
}
