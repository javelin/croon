/*
 * File  : Visualization.h
 * Author: Mark Documento
 */

#ifndef _Croon_Visualization_h_
#define _Croon_Visualization_h_

#include <Draw/Draw.h>

struct Visualization {
    static String Filter(String viz, String assFn, bool preview);
    static Image Thumbnail(String viz);
    static String Reso(bool preview, char sep);
    static String Reso(bool preview, int h, char sep);
    static String ShowFreqs(String assFn, bool preview);
    static String ShowSpectrum(String assFn, bool preview);
    static String APhaseMeter(String assFn, bool preview);
    static String AVectorScopeDot(String assFn, bool preview);
    static String AVectorScopeLine(String assFn, bool preview);
    static String ShowWaves(String assFn, bool preview);
};

#endif
