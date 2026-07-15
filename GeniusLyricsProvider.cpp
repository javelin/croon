/*
 * File  : GeniusLyricsProvider.cpp
 * Author: Mark Documento
 */

#include <Core/Core.h>

using namespace Upp;

#include "LyricsProviderTools.h"
#include "GeniusLyricsProvider.h"

const char* GeniusLyricsProvider::Name() {
    return "Genius";
}

const char* GeniusLyricsProvider::UrlFormat() {
    return "https://genius.com/%s-%s-lyrics";
}

String GeniusLyricsProvider::BuildUrl(String title, String artist) {
    return Format(UrlFormat(),
                  LyricsProviderTools::HyphenSlug(artist),
                  LyricsProviderTools::HyphenSlug(title));
}

bool GeniusLyricsProvider::ExtractLyrics(String content, String& lyrics) {
    Vector<String> blocks = Split(content, "data-lyrics-container=\"true\"");
    String html;
    for (int i = 1; i < blocks.GetCount(); i++) {
        int start = blocks[i].Find(">");
        int end = blocks[i].Find("</div>");
        if (start >= 0 && end > start)
            html << blocks[i].Mid(start + 1, end - start - 1) << "\n";
    }

    lyrics = LyricsProviderTools::CleanupHtmlLyrics(html);
    return !lyrics.IsEmpty();
}
