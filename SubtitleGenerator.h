/*
 * File  : SubtitleGenerator.h
 * Author: Mark Documento
 */

#ifndef _Croon_SubtitleGenerator_h_
#define _Croon_SubtitleGenerator_h_

struct SubtitleGenerator {
    static Vector<String> HighlightProbeLyrics(const KarData& data,
                                               int linesToDisplay=DefaultASSDisplayLines);
    static String ToAss(const KarData& data,
                        int linesToDisplay=DefaultASSDisplayLines,
                        int resX=1920,
                        int resY=1080);
    static String ToAss(const KarData& data,
                        const Vector<bool>& wrappedHighlights,
                        int linesToDisplay=DefaultASSDisplayLines,
                        int resX=1920,
                        int resY=1080);
    static String ToRichAss(const KarData& data,
                            int linesToDisplay=DefaultASSDisplayLines,
                            int resX=1920,
                            int resY=1080);
};

#endif
