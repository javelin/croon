/*
 * File  : FfmpegProjectCommandBuilder.h
 * Author: Mark Documento
 */

#ifndef _Croon_FfmpegProjectCommandBuilder_h_
#define _Croon_FfmpegProjectCommandBuilder_h_

struct KarData;

struct FfmpegProjectCommandBuilder {
    static Vector<String> DumpAttachmentAndGenerateThumbnail(String projectPath, String infoFilePath, String outputPath, int width, int height);
    static Vector<String> ExtractAudioAndInfo(String projectPath, String audioFilePath, String infoFilePath);
    static Vector<String> ExtractVideo(String projectPath, String videoFilePath);
    static Vector<String> SaveWithBackgroundVideo(const KarData& data, String outputPath);
    static Vector<String> SaveWithVisualization(const KarData& data, String outputPath);
};

#endif
