/*
 * File  : SubtitleLineProcessor.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

String& SubtitleLineProcessor::ReplaceMetadata(String& line, const KarData& data, bool replaceDash) {
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

void SubtitleLineProcessor::ProcessMetadata(const KarData& data, Vector<TimeLyrics>& vtl, int linesToDisplay) {
    vtl.Clear();
    vtl.Add({data.duration, "", -1});
    static const auto blankLine = "\u00A0";
    static const auto defaultTempo = 60; // 60bpm
    double lastTs = 0.0f;
    int maxBlanks = linesToDisplay - 1;
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
            auto countIn = TimeFormatter::CountInDuration(bpm);
            auto totalDec = countIn*3 + CountInDelay;
            if (ts - totalDec > lastTs) {
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
        bool isMeta = false;
        if (lyrics.StartsWith("\\@")) {
            lyrics.Remove(0, 1);
        }
        else if (lyrics.StartsWith("@")) {
            isMeta = true;
        }
        vtl.Add({ts, ReplaceMetadata(lyrics, data), idx, isMeta});
        lastTs = ts;
    }
}

VocalPart SubtitleLineProcessor::ResolveVocalPart(int partIndex,
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

String SubtitleLineProcessor::ResolveCountInStyle(VocalPart part, const String& incomingLyrics) {
    if (incomingLyrics.StartsWith("~"))  return "BUC";
    if (incomingLyrics.StartsWith("("))  return "MC";
    switch (part) {
        case VP_V2: return "CountInV2";
        case VP_B:  return "CountInB";
        default:    return "CountInV1";
    }
}

String SubtitleLineProcessor::ResolveStyle(VocalPart part, String& line, bool isCountIn,
                                           const String& incomingLyrics, bool isMeta) {
    if (isCountIn) return ResolveCountInStyle(part, incomingLyrics);
    if (isMeta) {
        if (line.StartsWith("@")) line.Remove(0, 1);
        return "MC";
    }
    if (line.StartsWith("~")) {
        line.Remove(0);
        line = Format("{\\i1}%s{\\i0}", line);
        return "BUC";
    }
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

String SubtitleLineProcessor::ResolveDimStyle(VocalPart part, String& line, bool isMeta) {
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

VocalPart SubtitleLineProcessor::LookaheadVocalPart(const Vector<TimeLyrics>& vtl, int startIdx,
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
