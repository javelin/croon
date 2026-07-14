/*
 * File  : UiScaler.h
 * Author: Mark Documento
 */

#ifndef _Croon_UiScaler_h_
#define _Croon_UiScaler_h_

struct UiScaler {
    static int X(int value);
    static int Y(int value);
    static int InverseX(int value);
    static int InverseY(int value);
};

#endif
