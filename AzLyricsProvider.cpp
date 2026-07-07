/*
 * File  : AzLyricsProvider.cpp
 * Author: Mark Documento
 */

#include <Core/Core.h>
#include <plugin/pcre/Pcre.h>

using namespace Upp;

#include "TextTools.h"
#include "AzLyricsProvider.h"

const char* AzLyricsProvider::Name() {
    return "AZLyrics";
}

const char* AzLyricsProvider::UrlFormat() {
    return "https://www.azlyrics.com/lyrics/%s/%s.html";
}

const char* AzLyricsProvider::LyricsPattern() {
    return "<!-- Usage of azlyrics.com content by any third-party.+?-->(.+?)</div>";
}

String AzLyricsProvider::BuildUrl(String title, String artist) {
    String t = Join(Split(TextTools::CleanSpacing(ToLower(TextTools::StripNonAlnum(TrimBoth(title)))), ' '), "");
    String a = ToLower(TextTools::CleanSpacing(TrimBoth(artist)));
    a.TrimStart("the ");
    a = TextTools::StripNonAlnum(a);
    a = Join(Split(TextTools::CleanSpacing(a), ' '), "");
    return Format(UrlFormat(), a, t);
}

bool AzLyricsProvider::ExtractLyrics(String content, String& lyrics) {
    static RegExp rx(LyricsPattern(), RegExp::DOTALL | RegExp::MULTILINE | RegExp::UTF8);
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
