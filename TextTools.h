/*
 * File  : TextTools.h
 * Author: Mark Documento
 */

#ifndef _Croon_TextTools_h_
#define _Croon_TextTools_h_

struct TextTools {
    static String CleanSpacing(const String& text);
    static String StripNonAlnum(String text);
    static String ShortenMiddle(String text, int maxLength);
};

#endif
