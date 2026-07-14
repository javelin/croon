/*
 * File  : FfmpegAudioCommandBuilder.cpp
 * Author: Mark Documento
 */

#include <Core/Core.h>

using namespace Upp;

#include "FfmpegAudioCommandBuilder.h"

Vector<String> FfmpegAudioCommandBuilder::ConvertToVorbis(String audioPath, String outputPath) {
    return {
        "-i",
        audioPath,
        "-vn",
        "-acodec",
        "libvorbis",
        outputPath
    };
}

Vector<String> FfmpegAudioCommandBuilder::Dehiss(String audioPath, String outputPath, int dB) {
    return {
        "-i",
        audioPath,
        "-af",
        Format("afftdn=nr=%d:nf=-45:bn=1", dB),
        outputPath
    };
}
