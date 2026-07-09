/*
 * File  : FfmpegAudioCommandBuilder.h
 * Author: Mark Documento
 */

#ifndef _Croon_FfmpegAudioCommandBuilder_h_
#define _Croon_FfmpegAudioCommandBuilder_h_

struct FfmpegAudioCommandBuilder {
    static Vector<String> ConvertToVorbis(String audioPath, String outputPath) {
        return {
            "-i",
            audioPath,
            "-vn",
            "-acodec",
            "libvorbis",
            outputPath
        };
    }
    
    static Vector<String> Dehiss(String audioPath, String outputPath, int factor=30) {
        return {
            "-i",
            audioPath,
            "-af",
            Format("afftdn=nr=%d", factor),
            outputPath
        };
    }
};

#endif
