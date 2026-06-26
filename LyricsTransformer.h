/*
 * File  : LyricsTransformer.h
 * Author: Mark Documento
 */

#ifndef _Croon_LyricsTransformer_h_
#define _Croon_LyricsTransformer_h_

struct LyricsTransformer {
    static String SplitDecorations(String& lyrics);
    static Vector<TimeLyrics> RawToUntimed(const KarData& data);
    static String TimedToRaw(const Vector<TimeLyrics>& timedLyrics, bool removeMetadata=false);
};

#endif
