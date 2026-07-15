/*
 * File  : LrcGenerator.cpp
 * Author: Mark Documento
 */

#include <Core/Core.h>
#include <Draw/Draw.h>

#include <cmath>

using namespace Upp;

#include "Constants.h"
#include "KarData.h"
#include "SubtitleLineProcessor.h"
#include "VocalPart.h"
#include "LrcGenerator.h"

namespace {

String LrcTimestamp(double seconds) {
    int totalCsec = (int)std::round(seconds * 100.0);
    int min = totalCsec / 6000;
    int sec = (totalCsec / 100) % 60;
    int csec = totalCsec % 100;
    return Format("[%02d:%02d.%02d]", min, sec, csec);
}

VocalPart AssignedVocalPart(int partIndex, const Vector<Tuple<int,bool,bool,bool>>& parts) {
    if (partIndex < 0) return VP_NONE;
    for (const auto& part : parts) {
        if (part.a == partIndex) {
            VocalPart vocalPart = VocalPartStyle::FromParts(part.b, part.c);
            return VocalPartStyle::IsAssigned(vocalPart) ? vocalPart:VP_NONE;
        }
    }
    return VP_NONE;
}

String VocalPartPrefix(VocalPart part) {
    switch (part) {
        case VP_V1: return "[1]: ";
        case VP_V2: return "[2]: ";
        case VP_B:  return "[B]: ";
        default:    return "";
    }
}

String LrcText(String line, bool isMeta) {
    line.Replace("\\(", "(");
    line.Replace("\\)", ")");
    if (line.StartsWith("@CountIn")) return "************";
    if (line == "\u00A0") return "";
    if (line.StartsWith("~")) {
        line.Remove(0);
        return Format("(%s)", line);
    }
    if (isMeta && line.StartsWith("@")) {
        line.Remove(0);
    }
    return line;
}

}

String LrcGenerator::ToLrc(const KarData& data) {
    if (data.timedLyrics.IsEmpty()) return "";
    Vector<TimeLyrics> lines;
    SubtitleLineProcessor::ProcessMetadata(data, lines, DefaultASSDisplayLines);
    
    Vector<String> lrc;
    for (int i = 1; i < lines.GetCount(); ++i) {
        const TimeLyrics& line = lines[i];
        String text = LrcText(TrimBoth(line.lyrics), line.isMeta);
        if (text.IsEmpty()) continue;
        
        String prefix;
        if (!text.StartsWith("(") && !line.isMeta && !text.StartsWith("************")) {
            prefix = VocalPartPrefix(AssignedVocalPart(line.partIndex, data.parts));
        }
        lrc.Add(LrcTimestamp(line.time) + prefix + text);
    }
    return Join(lrc, "\n");
}
