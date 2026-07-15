/*
 * File  : LyricsProviderTools.cpp
 * Author: Mark Documento
 */

#include <Core/Core.h>

using namespace Upp;

#include "TextTools.h"
#include "LyricsProviderTools.h"

String LyricsProviderTools::NormalizeSlugText(String text) {
    String normalized = ToLower(TrimBoth(text));
    normalized.Replace("&", " ");
    normalized.Replace("'", "");
    normalized.Replace("`", "");
    normalized.Replace("\r", " ");
    normalized.Replace("\n", " ");
    return normalized;
}

String LyricsProviderTools::StripFeatureSuffix(String text) {
    String lower = ToLower(text);
    Vector<String> markers = { " feat.", " feat ", " ft.", " ft ", " featuring ", " with " };
    for (String marker : markers) {
        int pos = lower.Find(marker);
        if (pos >= 0)
            return TrimBoth(text.Left(pos));
    }
    return TrimBoth(text);
}

String LyricsProviderTools::StripVersionSuffixes(String text) {
    String out;
    for (int i = 0; i < text.GetCount(); i++) {
        if (text[i] != '(' && text[i] != '[') {
            out.Cat(text[i]);
            continue;
        }

        char close = text[i] == '(' ? ')' : ']';
        int end = text.Find(close, i + 1);
        if (end < 0) {
            out.Cat(text[i]);
            continue;
        }

        String note = ToLower(text.Mid(i + 1, end - i - 1));
        if (note.Find("live") >= 0 || note.Find("remaster") >= 0 ||
            note.Find("version") >= 0 || note.Find("mono") >= 0 ||
            note.Find("stereo") >= 0 || note.Find("edit") >= 0) {
            i = end;
            continue;
        }

        out.Cat(text.Mid(i, end - i + 1));
        i = end;
    }
    return TrimBoth(out);
}

String LyricsProviderTools::HyphenSlug(String text) {
    text = NormalizeSlugText(StripFeatureSuffix(StripVersionSuffixes(text)));
    String words;
    bool lastDash = true;
    for (int i = 0; i < text.GetCount(); i++) {
        byte c = text[i];
        if (IsAlNum(c)) {
            words.Cat(c);
            lastDash = false;
        }
        else if (!lastDash) {
            words.Cat('-');
            lastDash = true;
        }
    }
    while (words.EndsWith("-"))
        words.Trim(words.GetCount() - 1);
    return words;
}

String LyricsProviderTools::CompactSlug(String text) {
    String slug = HyphenSlug(text);
    return Join(Split(slug, '-'), "");
}

String LyricsProviderTools::DecodeHtmlEntities(String text) {
    text.Replace("&amp;", "&");
    text.Replace("&quot;", "\"");
    text.Replace("&#39;", "'");
    text.Replace("&apos;", "'");
    text.Replace("&nbsp;", " ");
    text.Replace("&lt;", "<");
    text.Replace("&gt;", ">");
    return text;
}

String LyricsProviderTools::StripHtmlTags(String html) {
    String out;
    bool inTag = false;
    for (int i = 0; i < html.GetCount(); i++) {
        char c = html[i];
        if (c == '<') {
            inTag = true;
            continue;
        }
        if (c == '>') {
            inTag = false;
            continue;
        }
        if (!inTag)
            out.Cat(c);
    }
    return out;
}

String LyricsProviderTools::CleanupHtmlLyrics(String html) {
    html.Replace("<br/>", "\n");
    html.Replace("<br />", "\n");
    html.Replace("<br>", "\n");
    html.Replace("</p>", "\n");
    html.Replace("</div>", "\n");

    String text = DecodeHtmlEntities(StripHtmlTags(html));
    Vector<String> lines = Split(text, '\n');
    for (int i = 0; i < lines.GetCount(); i++)
        lines[i] = TrimBoth(lines[i]);
    while (!lines.IsEmpty() && lines[0].IsEmpty())
        lines.Remove(0);
    while (!lines.IsEmpty() && lines.Top().IsEmpty())
        lines.Drop();
    return Join(lines, "\n");
}
