/*
 * File  : TextTools.cpp
 * Author: Mark Documento
 */

#include <Core/Core.h>

using namespace Upp;

#include "TextTools.h"

String TextTools::CleanSpacing(const String& text) {
    auto vw = Split(text, ' ');
    for (int i = 0; i < vw.GetCount(); i++) {
        vw[i] = TrimBoth(vw[i]);
    }
    return Join(vw, " ");
}

String TextTools::StripNonAlnum(String text) {
    String ret;
    for (auto c : text) if (IsAlNum(c)) ret += c;
    return ret;
}

String TextTools::ShortenMiddle(String text, int maxLength) {
    String ret = text;
    if (text.GetLength() > maxLength) {
        ret = text.Left(maxLength/2 - 2) + "...";
        ret += text.Right(maxLength/2 - 2);
    }
    return ret;
}
