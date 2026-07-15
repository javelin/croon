/*
 * File  : SongLyricsProvider.cpp
 * Author: Mark Documento
 */

#include <Core/Core.h>

using namespace Upp;

#include "LyricsProviderTools.h"
#include "SongLyricsProvider.h"

const char* SongLyricsProvider::Name() {
    return "SongLyrics";
}

const char* SongLyricsProvider::UrlFormat() {
    return "https://www.songlyrics.com/%s/%s-lyrics/";
}

String SongLyricsProvider::BuildUrl(String title, String artist) {
    return Format(UrlFormat(),
                  LyricsProviderTools::HyphenSlug(artist),
                  LyricsProviderTools::HyphenSlug(title));
}

bool SongLyricsProvider::ExtractLyrics(String content, String& lyrics) {
    int id = content.Find("id=\"songLyricsDiv\"");
    if (id < 0)
        id = content.Find("id='songLyricsDiv'");
    if (id < 0) {
        lyrics = "";
        return false;
    }

    int start = content.ReverseFind("<", id);
    int end = content.Find("</p>", id);
    if (start < 0 || end < 0 || end <= start) {
        lyrics = "";
        return false;
    }

    lyrics = LyricsProviderTools::CleanupHtmlLyrics(content.Mid(start, end - start));
    return !lyrics.IsEmpty() && lyrics.Find("We do not have the lyrics for") < 0;
}
