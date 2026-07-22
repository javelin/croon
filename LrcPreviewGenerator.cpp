/*
 * File  : LrcPreviewGenerator.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#include "Constants.h"
#include "KarData.h"
#include "LrcGenerator.h"
#include "RichTextBuilder.h"
#include "SubtitleLineProcessor.h"
#include "VocalPart.h"
#include "LrcPreviewGenerator.h"

namespace {

String LrcPreviewStyle(const String& style) {
    if (style.StartsWith("BUC"))
        return "/*@(11.218.81)";
    if (style.StartsWith("V2") || style == "CountInV2")
        return "*@(144.213.255)";
    if (style.StartsWith("B") || style == "CountInB")
        return "*@(237.128.233)";
    if (style.StartsWith("MC"))
        return "*@(251.236.93)";
    return "*@(250.80.83)";
}

String ResolvePreviewStyle(const Vector<TimeLyrics>& lines,
                           int index,
                           const Vector<Tuple<int,bool,bool,bool>>& parts) {
    const TimeLyrics& line = lines[index];
    String styleLine = TrimBoth(line.lyrics);
    bool hasCountIn = styleLine.StartsWith("@CountIn");
    String incomingLyrics;
    VocalPart part = hasCountIn
        ? SubtitleLineProcessor::LookaheadVocalPart(lines, index + 1, parts, incomingLyrics)
        : SubtitleLineProcessor::ResolveVocalPart(line.partIndex, parts);
    return SubtitleLineProcessor::ResolveStyle(part, styleLine, hasCountIn, incomingLyrics, line.isMeta);
}

}

String LrcPreviewGenerator::ToQtf(const KarData& data) {
    RTHelper rth;
    Vector<String> lrcLines = Split(LrcGenerator::ToLrc(data), '\n');
    Vector<TimeLyrics> processedLines;
    SubtitleLineProcessor::ProcessMetadata(data, processedLines, DefaultASSDisplayLines);
    int lrcIndex = 0;
    for (int i = 1; i < processedLines.GetCount() && lrcIndex < lrcLines.GetCount(); ++i) {
        String exportedText = LrcGenerator::LrcText(TrimBoth(processedLines[i].lyrics), processedLines[i].isMeta);
        if (exportedText.IsEmpty())
            continue;

        String lrcLine = lrcLines[lrcIndex++];
        int endTimestamp = lrcLine.Find(']');
        if (lrcLine.StartsWith("[") && endTimestamp > 0) {
            String timestamp = lrcLine.Mid(0, endTimestamp + 1);
            String text = lrcLine.Mid(endTimestamp + 1);
            rth.Fmt("@(96.96.96)").Text(timestamp).EFmt()
               .Fmt(LrcPreviewStyle(ResolvePreviewStyle(processedLines, i, data.parts))).Text(text).EFmt()
               .NL();
        }
        else {
            rth.Fmt("*@(250.80.83)").Text(lrcLine).EFmt().NL();
        }
    }
    return rth.ToString();
}
