/*
 * File  : SubtitleGenerator.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#include "Constants.h"
#include "KarData.h"
#include "RichTextBuilder.h"
#include "SubtitleGenerator.h"
#include "SubtitleLineProcessor.h"
#include "TimeFormatter.h"
#include "VocalPart.h"

// Reds
#define RED_H               "&H000000FF"

// Greens
#define MALACHITE_H         "&H0051DA0B"
#define MALACHITE_D         "&H0040AA0A"

// Blues
#define BLUE_H              "&H00FF0000"

// Purples
#define PURPLE_H            "&H00FF00FF"

// Yellows
#define MAIZE_H             "&H005DECFB"
#define MAIZE_D             "&H0053CFDC"

namespace {

constexpr int SubtitleBottomMargin = 120;
constexpr int SubtitleScrollDurationMs = 450;

int SubtitleSlotY(const KarData& data, int resY, int slot) {
    int normalLineHeight = max(1, data.fontSize);
    int smallLineHeight = max(1, (int)(data.fontSize * 0.7));
    int normalStep = normalLineHeight * 2;
    int smallStep = smallLineHeight * 2;
    int bottom = resY - SubtitleBottomMargin;

    switch (slot) {
        case -1: return bottom - smallStep - normalStep * 3;
        case 0:  return bottom - smallStep - normalStep * 2;
        case 1:  return bottom - smallStep - normalStep;
        case 2:  return bottom - smallStep;
        case 3:  return bottom;
        default: return bottom + smallStep;
    }
}

int SubtitleScrollDuration(double startTS, double endTS) {
    int eventMs = max(1, (int)((endTS - startTS) * 1000.0));
    return min(SubtitleScrollDurationMs, eventMs);
}

String SubtitleMoveTag(const KarData& data, int resX, int resY, double startTS, double endTS,
                       int fromSlot, int toSlot) {
    int x = resX / 2;
    return Format("{\\an2\\move(%d,%d,%d,%d,0,%d)}",
                  x,
                  SubtitleSlotY(data, resY, fromSlot),
                  x,
                  SubtitleSlotY(data, resY, toSlot),
                  SubtitleScrollDuration(startTS, endTS));
}

String SubtitleGrayText(String line) {
    line.Replace("\\(", "(");
    line.Replace("\\)", ")");
    if (line.StartsWith("~")) {
        line.Remove(0);
        line = Format("{\\i1}%s{\\i0}", line);
    }
    else if (line.StartsWith("(")) {
        line = Format("{\\i1}%s{\\i0}", line);
    }
    else if (line.StartsWith("@")) {
        line = line.Mid(1);
    }
    return line;
}

}

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
        S("V1",       VocalPartStyle::V1PrimaryAss(), VocalPartStyle::V1SecondaryAss()),
        S("V1Normal", VocalPartStyle::V1SecondaryAss(), VocalPartStyle::V1PrimaryAss(), false),
        // V2: blue
        S("V2",       VocalPartStyle::V2PrimaryAss(), VocalPartStyle::V2SecondaryAss()),
        S("V2Normal", VocalPartStyle::V2SecondaryAss(), VocalPartStyle::V2PrimaryAss(), false),
        // B (both): purple (V1 red + V2 blue)
        S("B",        VocalPartStyle::BothPrimaryAss(), VocalPartStyle::BothSecondaryAss()),
        S("BNormal",  VocalPartStyle::BothSecondaryAss(), VocalPartStyle::BothPrimaryAss(), false),
        // BUC (backup, ~): green
        S("BUC",       MALACHITE_H, MALACHITE_D),
        S("BUCNormal", MALACHITE_D, MALACHITE_H, false),
        // MC (miscellaneous, (...)): yellow
        S("MC",       MAIZE_H, MAIZE_D),
        S("MCNormal", MAIZE_D, MAIZE_H, false),
        // Count-in styles inherit vocal color: D for unbeaten, H for beaten
        S("CountInV1", RED_H, VocalPartStyle::V1SecondaryAss()),
        S("CountInV2", BLUE_H, VocalPartStyle::V2SecondaryAss()),
        S("CountInB",  PURPLE_H, VocalPartStyle::BothSecondaryAss()),
        // Grayed: already displayed lines
        S("Grayed",   VocalPartStyle::GrayAss(), VocalPartStyle::GrayAss()),
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
        if (hasCountIn) {
            lastLine.Clear();
        }
        if (i + 1 < vtl.GetCount()) endTS = vtl[i + 1].time;
        
        String incomingLyrics;
        VocalPart part = hasCountIn
            ? SubtitleLineProcessor::LookaheadVocalPart(vtl, i + 1, data.parts, incomingLyrics)
            : SubtitleLineProcessor::ResolveVocalPart(tl.partIndex, data.parts);
        
        int futureLines = min(2, max(1, linesToDisplay - (lastLine.IsEmpty() ? 1:2)));
        for (int j = futureLines; j > 0; --j) {
            if (i + j < vtl.GetCount()) {
                const auto& ntl = vtl[i + j];
                String nextLine = TrimBoth(ntl.lyrics);
                if (nextLine.StartsWith("@CountIn")) nextLine = "\u00A0";
                nextLine.Replace("\\(", "(");
                nextLine.Replace("\\)", ")");
                VocalPart nextPart = SubtitleLineProcessor::ResolveVocalPart(ntl.partIndex, data.parts);
                String dimStyle = SubtitleLineProcessor::ResolveDimStyle(nextPart, nextLine, ntl.isMeta);
                vs.AddPick(Format("Dialogue: 0,%s,%s,%s,,0,0,0,,%s%s%s",
                                    TimeFormatter::Ass(startTS),
                                    TimeFormatter::Ass(endTS),
                                    dimStyle,
                                    SubtitleMoveTag(data, resX, resY, startTS, endTS, j + 2, j + 1),
                                    hasCountIn ? "":"{\\fad(150,100)}",
                                    nextLine));
            }
        }
        
        String singLine = line;
        singLine.Replace("\\(", "(");
        singLine.Replace("\\)", ")");
        String hilite = SubtitleLineProcessor::ResolveStyle(part, singLine, hasCountIn, incomingLyrics, tl.isMeta);
        vs.AddPick(Format("Dialogue: 0,%s,%s,%s,,0,0,0,,%s%s",
                            TimeFormatter::Ass(startTS),
                            TimeFormatter::Ass(endTS),
                            hilite,
                            SubtitleMoveTag(data, resX, resY, startTS, endTS, 2, 1),
                            singLine));
        
        if (!lastLine.IsEmpty()) {
            vs.AddPick(Format("Dialogue: 0,%s,%s,Grayed,,0,0,0,,%s%s",
                                TimeFormatter::Ass(startTS),
                                TimeFormatter::Ass(endTS),
                                SubtitleMoveTag(data, resX, resY, startTS, endTS, 1, 0),
                                SubtitleGrayText(lastLine)));
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
    S("V1",       VocalPartStyle::V1PrimaryAss(), VocalPartStyle::V1SecondaryAss());
    S("V1Normal", VocalPartStyle::V1SecondaryAss(), VocalPartStyle::V1PrimaryAss(), false);
    // V2: blue
    S("V2",       VocalPartStyle::V2PrimaryAss(), VocalPartStyle::V2SecondaryAss());
    S("V2Normal", VocalPartStyle::V2SecondaryAss(), VocalPartStyle::V2PrimaryAss(), false);
    // B (both): purple (V1 red + V2 blue)
    S("B",        VocalPartStyle::BothPrimaryAss(), VocalPartStyle::BothSecondaryAss());
    S("BNormal",  VocalPartStyle::BothSecondaryAss(), VocalPartStyle::BothPrimaryAss(), false);
    // BUC (backup, ~): green
    S("BUC",       MALACHITE_H, MALACHITE_D);
    S("BUCNormal", MALACHITE_D, MALACHITE_H, false);
    // MC (miscellaneous, (...)): yellow
    S("MC",       MAIZE_H, MAIZE_D);
    S("MCNormal", MAIZE_D, MAIZE_H, false);
    // Count-in styles inherit vocal color: D for unbeaten, H for beaten
    S("CountInV1", VocalPartStyle::V1PrimaryAss(), VocalPartStyle::V1SecondaryAss());
    S("CountInV2", VocalPartStyle::V2PrimaryAss(), VocalPartStyle::V2SecondaryAss());
    S("CountInB",  VocalPartStyle::BothPrimaryAss(), VocalPartStyle::BothSecondaryAss());
    // Grayed: already displayed lines
    S("Grayed",   VocalPartStyle::GrayAss(), VocalPartStyle::GrayAss());
    
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
        if (hasCountIn) {
            lastLine.Clear();
        }
        line.Replace("@CountIn", "");
        if (i + 1 < vtl.GetCount()) endTS = vtl[i + 1].time;
        
        String incomingLyrics;
        VocalPart part = hasCountIn
            ? SubtitleLineProcessor::LookaheadVocalPart(vtl, i + 1, data.parts, incomingLyrics)
            : SubtitleLineProcessor::ResolveVocalPart(tl.partIndex, data.parts);
        
        int futureLines = min(2, max(1, linesToDisplay - (lastLine.IsEmpty() ? 1:2)));
        for (int j = futureLines; j > 0; --j) {
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
                    .EFmt("@m").Text(SubtitleMoveTag(data, resX, resY, startTS, endTS, j + 2, j + 1))
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
            .EFmt("@m").Text(SubtitleMoveTag(data, resX, resY, startTS, endTS, 2, 1))
            .EFmt("@0").Text(singLine).EFmt().NL();
            
        if (!lastLine.IsEmpty()) {
            rth.Fmt("@3").Text("Dialogue: ")
                .EFmt("@c").Text("0,")
                .EFmt("@6").Text(Format("%s,%s,", TimeFormatter::Ass(startTS), TimeFormatter::Ass(endTS)))
                .EFmt("@c").Text("Grayed,,0,0,0,,")
                .EFmt("@m").Text(SubtitleMoveTag(data, resX, resY, startTS, endTS, 1, 0))
                .EFmt("@0").Text(SubtitleGrayText(lastLine)).EFmt().NL();
        }
        lastLine = hasCountIn ? "\u00A03... 2... 1...":line;
    }
    return rth.ToString();
}
