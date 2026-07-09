/*
 * File  : FfmpegThumbnailCommandBuilder.h
 * Author: Mark Documento
 */

#ifndef _Croon_FfmpegThumbnailCommandBuilder_h_
#define _Croon_FfmpegThumbnailCommandBuilder_h_

struct FfmpegThumbnailCommandBuilder {
    static Vector<String> Generate(String videoPath, String outputPath, int width, int height) {
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
};

#endif
