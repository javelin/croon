/*
 * File  : FfmpegExportCommandBuilder.h
 * Author: Mark Documento
 */

#ifndef _Croon_FfmpegExportCommandBuilder_h_
#define _Croon_FfmpegExportCommandBuilder_h_

struct KarData;

struct FfmpegExportCommandBuilder {
    static Vector<String> WithBackgroundVideo(const KarData& data, String assFilePath, String outputPath, String audioFilepath="", int scaleHeight=0);
    static Vector<String> WithVisualization(const KarData& data, String assFilePath, String outputPath, String audioFilepath, int canvasHeight=1080);
    static Vector<String> GenerateCoverImage(String videoPath, String outputPath, double thumbnailTS);
};

#endif
