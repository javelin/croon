/*
 * File  : LyricsDownloadService.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

String LyricsDownloadService::BuildAzLyricsUrl(String title, String artist) {
    String t = Join(Split(TextTools::CleanSpacing(ToLower(TextTools::StripNonAlnum(TrimBoth(title)))), ' '), "");
    String a = ToLower(TextTools::StripNonAlnum(TrimBoth(artist)));
    a.TrimStart("the ");
    a = Join(Split(TextTools::CleanSpacing(a), ' '), "");
    return Format(AZ_URL, a, t);
}

bool LyricsDownloadService::ExtractAzLyrics(String content, String& lyrics) {
    static RegExp rx(AZ_PATTERN, RegExp::DOTALL | RegExp::MULTILINE | RegExp::UTF8);
    if (rx.Study() && rx.Match(content)) {
        auto vl = Split(rx.GetString(0), "<br>");
        for (int i = 0; i < vl.GetCount(); i++) {
            vl[i] = TrimBoth(vl[i]);
        }
        lyrics = Join(vl, "\n");
        return true;
    }
    lyrics = "";
    return false;
}

bool LyricsDownloadService::Download(String title, String artist, String& lyrics) {
    DownloadDlg dlg;
    String* output = &lyrics;
    dlg.WhenDownloadSuccess << [output](String content) {
        LyricsDownloadService::ExtractAzLyrics(content, *output);
    };
    return dlg.Run(BuildAzLyricsUrl(title, artist), "Downloading lyrics") == IDOK;
}
