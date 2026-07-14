/*
 * File  : UiScaler.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#include "UiScaler.h"

int UiScaler::X(int value) {
    return Zx(value);
}

int UiScaler::Y(int value) {
    return Zy(value);
}

int UiScaler::InverseX(int value) {
    static int multiplier = 0;
    static int divisor = 0;
    if (!multiplier || !divisor) {
        Size m, d;
        Ctrl::GetZoomRatio(m, d);
        multiplier = d.cx;
        divisor = m.cx;
    }
    return value * multiplier / divisor;
}

int UiScaler::InverseY(int value) {
    static int multiplier = 0;
    static int divisor = 0;
    if (!multiplier || !divisor) {
        Size m, d;
        Ctrl::GetZoomRatio(m, d);
        multiplier = d.cy;
        divisor = m.cy;
    }
    return value * multiplier / divisor;
}
