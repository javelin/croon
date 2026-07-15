/*
 * File  : LyricsProviderTools.h
 * Author: Mark Documento
 */

#ifndef _Croon_LyricsProviderTools_h_
#define _Croon_LyricsProviderTools_h_

struct LyricsProviderTools {
    static String HyphenSlug(String text);
    static String CompactSlug(String text);
    static String StripFeatureSuffix(String text);
    static String StripVersionSuffixes(String text);
    static String CleanupHtmlLyrics(String html);

private:
    static String NormalizeSlugText(String text);
    static String DecodeHtmlEntities(String text);
    static String StripHtmlTags(String html);
};

#endif
