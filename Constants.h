/*
 * File  : Constants.h
 * Author: Mark Documento
 */

#ifndef _Croon_Constants_h_
#define _Croon_Constants_h_

static const int DefaultASSDisplayLines = 3;
static const int MaxDecorLength = 10;
static const int MaxLineLength = 80;
static const int MaxASSDisplayLines = 4;
static const int MinASSDisplayLines = 1;
static const int ThumbnailDim = 256;
static const double CountInDelay = 2.0f;

#define AZ_URL          "https://www.azlyrics.com/lyrics/%s/%s.html"
#define AZ_PATTERN      "<!-- Usage of azlyrics.com content by any third-party.+?-->(.+?)</div>"
#define USER_AGENT      "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/60.0.3112.90 Safari/537.36"

#endif
