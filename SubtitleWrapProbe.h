/*
 * File  : SubtitleWrapProbe.h
 * Author: Mark Documento
 */

#ifndef _Croon_SubtitleWrapProbe_h_
#define _Croon_SubtitleWrapProbe_h_

struct SubtitleWrapProbeBand : Moveable<SubtitleWrapProbeBand> {
    int y0 = 0;
    int y1 = 0;
    int width = 0;
};

struct SubtitleWrapProbeFrame : Moveable<SubtitleWrapProbeFrame> {
    Vector<SubtitleWrapProbeBand> bands;
};

struct SubtitleWrapProbe {
    static String BuildAss(const KarData& data,
                           const Vector<String>& lyrics,
                           int resX=1920,
                           int resY=1080);
    static Vector<SubtitleWrapProbeFrame> AnalyzeRgbaFrames(const String& rgba,
                                                            int width,
                                                            int height,
                                                            int frameCount,
                                                            int alphaThreshold=10);
    static bool IsWrappedFrame(const SubtitleWrapProbeFrame& frame, int fontSize);
};

#endif
