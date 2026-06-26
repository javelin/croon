/*
 * File  : FfmpegProgressParser.h
 * Author: Mark Documento
 */

#ifndef _Croon_FfmpegProgressParser_h_
#define _Croon_FfmpegProgressParser_h_

struct FfmpegProgressParser {
    static bool ParseTimestamp(String output, double& seconds, String& formatted, String key="time=");
};

#endif
