/*
 * File  : FfmpegAudioCommandBuilder.h
 * Author: Mark Documento
 */

#ifndef _Croon_FfmpegAudioCommandBuilder_h_
#define _Croon_FfmpegAudioCommandBuilder_h_

struct FfmpegAudioCommandBuilder {
    static Vector<String> ConvertToVorbis(String audioPath, String outputPath);
    static Vector<String> Dehiss(String audioPath, String outputPath, int dB=15);
};

#endif
