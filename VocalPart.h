/*
 * File  : VocalPart.h
 * Author: Mark Documento
 */

#ifndef _Croon_VocalPart_h_
#define _Croon_VocalPart_h_

#include <CtrlLib/CtrlLib.h>

enum VocalPart {
    VP_NONE = -1,
    VP_V1,
    VP_V2,
    VP_B
};

struct VocalPartStyle {
    static const char *V1PrimaryAss();
    static const char *V1SecondaryAss();
    static const char *V2PrimaryAss();
    static const char *V2SecondaryAss();
    static const char *BothPrimaryAss();
    static const char *BothSecondaryAss();
    static const char *GrayAss();

    static VocalPart FromParts(bool v1, bool v2);
    static Tuple<int, bool, bool, bool> ToParts(int lineIndex, VocalPart part);
    static VocalPart Next(VocalPart part);
    static bool IsAssigned(VocalPart part);
    static String Label(VocalPart part);
    static Color IndicatorColor(VocalPart part);
    static Color IndicatorTextColor(VocalPart part);
};

#endif
