/*
 * File  : SubtitleWrapProbeRunner.h
 * Author: Mark Documento
 */

#ifndef _Croon_SubtitleWrapProbeRunner_h_
#define _Croon_SubtitleWrapProbeRunner_h_

#include "SubtitleWrapProbe.h"

struct KarData;

struct SubtitleWrapProbeRunner {
    static bool Run(const KarData& data,
                    const Vector<String>& lyrics,
                    Vector<SubtitleWrapProbeFrame>& frames,
                    String ffmpegPath=String::GetVoid(),
                    int resX=1920,
                    int resY=1080,
                    int cropY=700,
                    int cropHeight=300);
};

#endif
