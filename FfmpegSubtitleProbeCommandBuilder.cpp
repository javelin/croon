/*
 * File  : FfmpegSubtitleProbeCommandBuilder.cpp
 * Author: Mark Documento
 */

#include <Core/Core.h>

using namespace Upp;

#include "FfmpegSubtitleProbeCommandBuilder.h"

namespace {

String EscapeSubtitlePath(String assFilePath) {
#ifdef PLATFORM_WIN32
    assFilePath.Replace("\\", "/");
    assFilePath.Replace(":", "\\\\:");
#endif
    return assFilePath;
}

}

Vector<String> FfmpegSubtitleProbeCommandBuilder::RenderRgba(String assFilePath,
                                                             String outputPath,
                                                             int frameCount,
                                                             int width,
                                                             int height,
                                                             int cropY,
                                                             int cropHeight) {
    return {
        "-f",
        "lavfi",
        "-i",
        Format("color=c=black@0.0:s=%dx%d:r=1", width, height),
        "-t",
        AsString(max(0, frameCount)),
        "-vf",
        Format("subtitles=%s,crop=%d:%d:0:%d", EscapeSubtitlePath(assFilePath), width, cropHeight, cropY),
        "-pix_fmt",
        "rgba",
        "-f",
        "rawvideo",
        outputPath
    };
}

Vector<String> FfmpegSubtitleProbeCommandBuilder::RenderPngFrames(String assFilePath,
                                                                  String outputPattern,
                                                                  int frameCount,
                                                                  int width,
                                                                  int height,
                                                                  int cropY,
                                                                  int cropHeight) {
    return {
        "-y",
        "-f",
        "lavfi",
        "-i",
        Format("color=c=black@0.0:s=%dx%d:r=1", width, height),
        "-t",
        AsString(max(0, frameCount)),
        "-vf",
        Format("subtitles=%s,crop=%d:%d:0:%d", EscapeSubtitlePath(assFilePath), width, cropHeight, cropY),
        outputPattern
    };
}
