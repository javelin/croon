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
