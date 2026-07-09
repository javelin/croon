/*
 * File  : FfmpegCommandBuilder.h
 * Author: Mark Documento
 */

#ifndef _Croon_FfmpegCommandBuilder_h_
#define _Croon_FfmpegCommandBuilder_h_

#include "FfmpegAudioCommandBuilder.h"
#include "FfmpegExportCommandBuilder.h"
#include "FfmpegThumbnailCommandBuilder.h"

struct FfmpegCommandBuilder {
    static Vector<String> ConvertAudioToVorbis(String audioPath, String outputPath) {
        return FfmpegAudioCommandBuilder::ConvertToVorbis(audioPath, outputPath);
    }
    
    static Vector<String> DehissAudio(String audioPath, String outputPath, int factor=30) {
        return FfmpegAudioCommandBuilder::Dehiss(audioPath, outputPath, factor);
    }
    
    static Vector<String> DumpAttachmentAndGenerateThumbnail(String projectPath, String infoFilePath, String outputPath, int width, int height) {
        return {
            "-dump_attachment:t:0", infoFilePath,
            "-i", projectPath,
            "-map", "v:0",
            "-ss", "00:00:01",
            "-vframes", "1",
            "-vf",
            Format("crop='min(iw,ih)':'min(iw,ih)',scale=%d:%d",
                    width, height),
            outputPath
        };
    }
    
    static Vector<String> ExportWithBackgroundVideo(const KarData& data, String assFilePath, String outputPath, String audioFilepath="", bool preview=true) {
        return FfmpegExportCommandBuilder::WithBackgroundVideo(data, assFilePath, outputPath, audioFilepath, preview);
    }
    
    static Vector<String> ExportWithVisualization(const KarData& data, String assFilePath, String outputPath, String audioFilepath, bool preview=true) {
        return FfmpegExportCommandBuilder::WithVisualization(data, assFilePath, outputPath, audioFilepath, preview);
    }
    
    static Vector<String> GenerateCoverImage(String videoPath, String outputPath, double thumbnailTS) {
        return FfmpegExportCommandBuilder::GenerateCoverImage(videoPath, outputPath, thumbnailTS);
    }
    
    static Vector<String> GenerateThumbnail(String videoPath, String outputPath, int width, int height) {
        return FfmpegThumbnailCommandBuilder::Generate(videoPath, outputPath, width, height);
    }
    
    static Vector<String> ProjectExtractAudioAndInfo(String projectPath, String audioFilePath, String infoFilePath) {
        return {
            "-dump_attachment:t:0", infoFilePath,
            "-i", projectPath,
            "-map", "0:a:0",
            "-c:a", "copy",
            audioFilePath
        };
    }
    
    static Vector<String> ProjectExtractVideo(String projectPath, String videoFilePath) {
        return {
            "-i", projectPath,
            "-map", "0:v:0",
            "-c:v", "copy",
            videoFilePath
        };
    }
    
    static Vector<String> ProjectSaveWithBackgroundVideo(const KarData& data, String outputPath) {
        return {
            "-i", data.videoFilePath,
            "-i", data.audioFilePath,
            "-map", "0:v:0",
            "-map", "1:a:0",
            "-map_metadata:s", "-1",
            "-attach", data.infoFilePath,
            "-metadata:s:2", AppIdentity::ProjectAttachmentMetadata(),
            "-metadata:s:2", "mimetype=application/json",
            "-c", "copy",
            "-metadata", Format("APPLICATION=%s v%s", AppIdentity::ProductName(), data.version),
            "-f", "matroska",
            outputPath
        };
    }
    
    static Vector<String> ProjectSaveWithVisualization(const KarData& data, String outputPath) {
        return {
            "-i", data.audioFilePath,
            "-map_metadata:s", "-1",
            "-attach", data.infoFilePath,
            "-metadata:s:1", AppIdentity::ProjectAttachmentMetadata(),
            "-metadata:s:1", "mimetype=application/json",
            "-c", "copy",
            "-metadata", Format("APPLICATION=%s v%s", AppIdentity::ProductName(), data.version),
            "-f", "matroska",
            outputPath
        };
    }
};

#endif
