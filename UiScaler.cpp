/*
 * File  : UiScaler.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

int UiScaler::X(int value) {
    static int mx = 0, dx = 0;
    if (!mx || !dx) {
        Size m, d;
        Ctrl::GetZoomRatio(m, d);
        mx = m.cx;
        dx = d.cx;
    }
    return value*dx/mx;
}

int UiScaler::Y(int value) {
    static int my = 0, dy = 0;
    if (!my || !dy) {
        Size m, d;
        Ctrl::GetZoomRatio(m, d);
        my = m.cy;
        dy = d.cy;
    }
    return value*dy/my;
}
