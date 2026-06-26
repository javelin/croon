/*
 * File  : LyricsTransformer.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

String LyricsTransformer::SplitDecorations(String& line) {
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

Vector<TimeLyrics> LyricsTransformer::RawToUntimed(const KarData& data) {
    Vector<TimeLyrics> lyrics{TimeLyrics(data.duration, "")};
    int lastLF = 0;
    int pos = -1;
    
    while ((pos = data.rawLyrics.Find('\n', lastLF)) > -1) {
        auto line = data.rawLyrics.Mid(lastLF, pos - lastLF);
        lastLF = pos + 1;
        line = TrimBoth(line);
        auto decor = SplitDecorations(line);
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
    auto decor = SplitDecorations(line);
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

String LyricsTransformer::TimedToRaw(const Vector<TimeLyrics>& timedLyrics, bool removeMetadata) {
    Vector<String> vs;
    for (const auto& tl : timedLyrics) {
        if (timedLyrics.GetIndex(tl) == 0) continue;
        if (removeMetadata) {
            auto lyrics = tl.lyrics;
            if (lyrics == "-") lyrics = "";
            else (void)SplitDecorations(lyrics);
            if (!lyrics.IsEmpty() && !lyrics.StartsWith("@")) vs.Add(lyrics);
        }
        else vs.Add(tl.lyrics);
    }
    return Join(vs, "\n");
}
