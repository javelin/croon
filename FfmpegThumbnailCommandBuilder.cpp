/*
 * File  : FfmpegThumbnailCommandBuilder.cpp
 * Author: Mark Documento
 */

#include <Core/Core.h>

using namespace Upp;

#include "FfmpegThumbnailCommandBuilder.h"

Vector<String> FfmpegThumbnailCommandBuilder::Generate(String videoPath, String outputPath, int width, int height) {
    return {
        "-i",
        videoPath,
        "-ss",
        "00:00:06",
        "-vframes",
        "1",
        "-vf",
        Format("crop='min(iw,ih)':'min(iw,ih)',scale=%d:%d",
                width, height),
        outputPath
    };
}
