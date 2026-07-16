/*
 * File  : FfmpegSubtitleProbeCommandBuilder.h
 * Author: Mark Documento
 */

#ifndef _Croon_FfmpegSubtitleProbeCommandBuilder_h_
#define _Croon_FfmpegSubtitleProbeCommandBuilder_h_

struct FfmpegSubtitleProbeCommandBuilder {
    static Vector<String> RenderRgba(String assFilePath,
                                     String outputPath,
                                     int frameCount,
                                     int width=1920,
                                     int height=1080,
                                     int cropY=700,
                                     int cropHeight=300);
    static Vector<String> RenderPngFrames(String assFilePath,
                                          String outputPattern,
                                          int frameCount,
                                          int width=1920,
                                          int height=1080,
                                          int cropY=700,
                                          int cropHeight=300);
};

#endif
