/*
 * File  : Util.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

#define SRT_PATTERN "(\\d+)\n(.+) -->.+\n(.+)\n"

String GetDataDirectory() {
    return AppPaths::DataDirectory();
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
    return SubtitleLineProcessor::ReplaceMetadata(line, data, replaceDash);
}

void ProcessMetadata(const KarData& data, Vector<TimeLyrics>& vtl, int linesToDisplay) {
    SubtitleLineProcessor::ProcessMetadata(data, vtl, linesToDisplay);
}

VocalPart ResolveVocalPart(int partIndex,
                                   const Vector<Tuple<int,bool,bool,bool>>& parts) {
    return SubtitleLineProcessor::ResolveVocalPart(partIndex, parts);
}

String ResolveCountInStyle(VocalPart part, const String& incomingLyrics) {
    return SubtitleLineProcessor::ResolveCountInStyle(part, incomingLyrics);
}

String ResolveStyle(VocalPart part, String& line, bool isCountIn,
                            const String& incomingLyrics, bool isMeta) {
    return SubtitleLineProcessor::ResolveStyle(part, line, isCountIn, incomingLyrics, isMeta);
}

String ResolveDimStyle(VocalPart part, String& line, bool isMeta) {
    return SubtitleLineProcessor::ResolveDimStyle(part, line, isMeta);
}

VocalPart LookaheadVocalPart(const Vector<TimeLyrics>& vtl, int startIdx,
                                     const Vector<Tuple<int,bool,bool,bool>>& parts,
                                     String& outLyrics) {
    return SubtitleLineProcessor::LookaheadVocalPart(vtl, startIdx, parts, outLyrics);
}

String TimedToASS(const KarData& data, int linesToDisplay, int resX, int resY) {
    return SubtitleGenerator::ToAss(data, linesToDisplay, resX, resY);
}

String TimedToRichASS(const KarData& data, int linesToDisplay, int resX, int resY) {
    return SubtitleGenerator::ToRichAss(data, linesToDisplay, resX, resY);
}

Vector<String> GetPaths(String dir, String pattern) {
    return AppPaths::FindFiles(dir, pattern);
}

bool DownloadLyrics(String title, String artist, String& lyrics) {
    return LyricsDownloadService::Download(title, artist, lyrics);
}

int _Zx(int zx) {
    return UiScaler::X(zx);
}

int _Zy(int zy) {
    return UiScaler::Y(zy);
}

bool ComputeFfmpegTs(String s, double& ts, String& formatted, String key) {
    return FfmpegProgressParser::ParseTimestamp(s, ts, formatted, key);
}

const Vector<String>& GetGenres() {
    return GenreCatalog::List();
}
