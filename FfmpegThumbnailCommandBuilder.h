/*
 * File  : FfmpegThumbnailCommandBuilder.h
 * Author: Mark Documento
 */

#ifndef _Croon_FfmpegThumbnailCommandBuilder_h_
#define _Croon_FfmpegThumbnailCommandBuilder_h_

struct FfmpegThumbnailCommandBuilder {
    static Vector<String> Generate(String videoPath, String outputPath, int width, int height);
};

#endif
