/*
 * File  : SubtitleLineProcessor.h
 * Author: Mark Documento
 */

#ifndef _Croon_SubtitleLineProcessor_h_
#define _Croon_SubtitleLineProcessor_h_

#include "VocalPart.h"

struct SubtitleLineProcessor {
    static String& ReplaceMetadata(String& line, const KarData& data, bool replaceDash=true);
    static void ProcessMetadata(const KarData& data, Vector<TimeLyrics>& vtl, int linesToDisplay);
    static VocalPart ResolveVocalPart(int partIndex,
                                      const Vector<Tuple<int,bool,bool,bool>>& parts);
    static String ResolveCountInStyle(VocalPart part, const String& incomingLyrics);
    static String ResolveStyle(VocalPart part, String& line, bool isCountIn,
                               const String& incomingLyrics="", bool isMeta=false);
    static String ResolveDimStyle(VocalPart part, String& line, bool isMeta=false);
    static VocalPart LookaheadVocalPart(const Vector<TimeLyrics>& vtl, int startIdx,
                                        const Vector<Tuple<int,bool,bool,bool>>& parts,
                                        String& outLyrics);
};

#endif
