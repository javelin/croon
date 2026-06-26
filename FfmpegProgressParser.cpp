/*
 * File  : FfmpegProgressParser.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

bool FfmpegProgressParser::ParseTimestamp(String output, double& seconds, String& formatted, String key) {
    TrimBoth(output);
    int pos = output.Find(key);
    if (pos > -1) {
        auto tsStr = output.Mid(pos + key.GetLength(), 11);
        auto vs = Split(tsStr, ':');
        if (vs.GetCount() < 3) return false;
        seconds = StrDbl(vs[0])*360.0f;
        seconds += StrDbl(vs[1])*60.0f;
        seconds += StrDbl(vs[2]);
        formatted = output.Mid(pos + 5, 11);
        return true;
    }
    return false;
}
