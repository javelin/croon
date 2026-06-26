/*
 * File  : LyricsDownloadService.h
 * Author: Mark Documento
 */

#ifndef _Croon_LyricsDownloadService_h_
#define _Croon_LyricsDownloadService_h_

struct LyricsDownloadService {
    static String BuildAzLyricsUrl(String title, String artist);
    static bool ExtractAzLyrics(String content, String& lyrics);
    static bool Download(String title, String artist, String& lyrics);
};

#endif
