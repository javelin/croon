/*
 * File  : Visualization.h
 * Author: Mark Documento
 */

#ifndef _Croon_Visualization_h_
#define _Croon_Visualization_h_

#include <Draw/Draw.h>

struct Visualization {
    static String Filter(String viz, String assFn, int height=1080);
    static Image Thumbnail(String viz);
    static String Reso(int height, char sep);
    static String Reso(int height, int elemHeight, char sep);
    static String ShowFreqs(String assFn, int height=1080);
    static String ShowSpectrum(String assFn, int height=1080);
    static String APhaseMeter(String assFn, int height=1080);
    static String AVectorScopeDot(String assFn, int height=1080);
    static String AVectorScopeLine(String assFn, int height=1080);
    static String ShowWaves(String assFn, int height=1080);
};

#endif
