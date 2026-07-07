/*
 * File  : LyricsDownloadService.h
 * Author: Mark Documento
 */

#ifndef _Croon_LyricsDownloadService_h_
#define _Croon_LyricsDownloadService_h_

struct LyricsDownloadService {
    enum DownloadStatus {
        DownloadOk,
        DownloadCancelled,
        ExtractionFailed,
    };

    static const char* ProviderName();
    static const char* DownloadStatusLabel(DownloadStatus status);
    static String BuildAzLyricsUrl(String title, String artist);
    static bool ExtractAzLyrics(String content, String& lyrics);
    static DownloadStatus DownloadWithStatus(String title, String artist, String& lyrics);
    static bool Download(String title, String artist, String& lyrics);

private:
    static const char* AzLyricsUrlFormat();
    static const char* AzLyricsPattern();
};

#endif
