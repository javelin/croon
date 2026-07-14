/*
 * File  : VocalPart.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#include "VocalPart.h"

namespace {

constexpr const char *STRAWBERRY_RED_H = "&H005350FA";
constexpr const char *STRAWBERRY_RED_D = "&H004644D9";
constexpr const char *LIGHT_BLUE_H = "&H00FFD590";
constexpr const char *LIGHT_BLUE_D = "&H00FFB957";
constexpr const char *ORCHID_H = "&H00E980ED";
constexpr const char *ORCHID_D = "&H00C66DC9";
constexpr const char *GRAY = "&H00808080";

}

const char *VocalPartStyle::V1PrimaryAss() {
    return STRAWBERRY_RED_H;
}

const char *VocalPartStyle::V1SecondaryAss() {
    return STRAWBERRY_RED_D;
}

const char *VocalPartStyle::V2PrimaryAss() {
    return LIGHT_BLUE_H;
}

const char *VocalPartStyle::V2SecondaryAss() {
    return LIGHT_BLUE_D;
}

const char *VocalPartStyle::BothPrimaryAss() {
    return ORCHID_H;
}

const char *VocalPartStyle::BothSecondaryAss() {
    return ORCHID_D;
}

const char *VocalPartStyle::GrayAss() {
    return GRAY;
}

VocalPart VocalPartStyle::FromParts(bool v1, bool v2) {
    if (v1 && v2) return VP_B;
    if (v2) return VP_V2;
    if (v1) return VP_V1;
    return VP_NONE;
}

Tuple<int, bool, bool, bool> VocalPartStyle::ToParts(int lineIndex, VocalPart part) {
    return MakeTuple(lineIndex, part == VP_V1 || part == VP_B, part == VP_V2 || part == VP_B, IsAssigned(part));
}

VocalPart VocalPartStyle::Next(VocalPart part) {
    switch (part) {
        case VP_V1: return VP_V2;
        case VP_V2: return VP_B;
        case VP_B:  return VP_NONE;
        default:    return VP_V1;
    }
}

bool VocalPartStyle::IsAssigned(VocalPart part) {
    return part == VP_V1 || part == VP_V2 || part == VP_B;
}

String VocalPartStyle::Label(VocalPart part) {
    switch (part) {
        case VP_V1: return "1";
        case VP_V2: return "2";
        case VP_B:  return "B";
        default:    return "-";
    }
}

Color VocalPartStyle::IndicatorColor(VocalPart part) {
    switch (part) {
        case VP_V1: return Color(0xFA, 0x50, 0x53);
        case VP_V2: return Color(0x90, 0xD5, 0xFF);
        case VP_B:  return Color(0xED, 0x80, 0xE9);
        default:    return Color(0x80, 0x80, 0x80);
    }
}

Color VocalPartStyle::IndicatorTextColor(VocalPart part) {
    return part == VP_V2 ? Black() : White();
}
