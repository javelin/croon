/*
 * File  : Visualization.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_header.h>

#include "Visualization.h"

String Visualization::Filter(String viz, String assFn, int height) {
    if (viz == "@@freqs") return ShowFreqs(assFn, height);
    else if (viz == "@@spectrum") return ShowSpectrum(assFn, height);
    else if (viz == "@@vectorscopedot") return AVectorScopeDot(assFn, height);
    else if (viz == "@@vectorscopeline") return AVectorScopeLine(assFn, height);
    else if (viz == "@@waves") return ShowWaves(assFn, height);
    return String::GetVoid();
}

Image Visualization::Thumbnail(String viz) {
    if (viz == "@@freqs") return CroonImg::VizFreqs();
    else if (viz == "@@spectrum") return CroonImg::VizSpectrum();
    else if (viz == "@@vectorscopedot") return CroonImg::VizVectorScopeDot();
    else if (viz == "@@vectorscopeline") return CroonImg::VizVectorScopeLine();
    else if (viz == "@@waves") return CroonImg::VizWaves();
    return CroonImg::GrayMic256();
}

// Canvas width scales from the requested height, keeping the proportions of the
// original 1926x1080 full-size canvas, and is forced even (ffmpeg/yuv420
// requires even dimensions). 180p -> 320, 1080p -> 1926, 2160p -> 3852, etc.
static int VizWidth(int height) {
    int w = height*1926/1080;
    return w - (w % 2);
}

String Visualization::Reso(int height, char sep) {
    return Format("%d%c%d", VizWidth(height), sep, height);
}

// Canvas width paired with an explicit element height (used by the wave sizes).
String Visualization::Reso(int height, int elemHeight, char sep) {
    return Format("%d%c%d", VizWidth(height), sep, elemHeight);
}

String Visualization::ShowFreqs(String assFn, int height) {
    return Format("[1:a]showfreqs=s=%s:mode=line:colors=red[vis]; "
                    "[0:v][vis]overlay=0:0[bg]; "
                    "[bg]subtitles=%s",
                    Reso(height, 'x'),
                    assFn);
}

String Visualization::ShowSpectrum(String assFn, int height) {
    return Format("[1:a]showspectrum=s=%s:mode=combined:color=intensity[vis]; "
                    "[0:v][vis]overlay=0:0[bg]; "
                    "[bg]subtitles=%s",
                    Reso(height, 'x'),
                    assFn);
}

String Visualization::APhaseMeter(String assFn, int height) {
    return Format("[1:a]aphasemeter=s=%s[vis]; "
                    "[0:v][vis]overlay=0:0[bg]; "
                    "[bg]subtitles=%s",
                    Reso(height, 'x'),
                    assFn);
}

String Visualization::AVectorScopeDot(String assFn, int height) {
    return Format("[1:a]avectorscope=s=%s:mode=lissajous_xy:draw=dot[vis]; "
                    "[0:v][vis]overlay=0:0[bg]; "
                    "[bg]subtitles=%s",
                    Reso(height, 'x'),
                    assFn);
}

String Visualization::AVectorScopeLine(String assFn, int height) {
    return Format("[1:a]avectorscope=s=%s:mode=lissajous_xy:draw=line[vis]; "
                    "[0:v][vis]overlay=0:0[bg]; "
                    "[bg]subtitles=%s",
                    Reso(height, 'x'),
                    assFn);
}

String Visualization::ShowWaves(String assFn, int height) {
    return Format("[1:a]showwaves=s=%s:mode=line:colors=cyan[wave]; "
                    "[wave]scale=%s[wave_scaled]; "
                    "[0:v][wave_scaled]overlay=0:(H-h)/2[bg]; "
                    "[bg]subtitles=%s",
                    Reso(height, 300*height/1080, 'x'),
                    Reso(height, 1204*height/1080, ':'),
                    assFn);
}
