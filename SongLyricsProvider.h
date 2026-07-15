/*
 * File  : SongLyricsProvider.h
 * Author: Mark Documento
 */

#ifndef _Croon_SongLyricsProvider_h_
#define _Croon_SongLyricsProvider_h_

struct SongLyricsProvider {
    static const char* Name();
    static String BuildUrl(String title, String artist);
    static bool ExtractLyrics(String content, String& lyrics);

private:
    static const char* UrlFormat();
};

#endif
