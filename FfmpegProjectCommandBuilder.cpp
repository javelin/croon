/*
 * File  : FfmpegProjectCommandBuilder.cpp
 * Author: Mark Documento
 */

#include <Draw/Draw.h>

using namespace Upp;

#include "AppIdentity.h"
#include "KarData.h"
#include "FfmpegProjectCommandBuilder.h"

Vector<String> FfmpegProjectCommandBuilder::DumpAttachmentAndGenerateThumbnail(String projectPath, String infoFilePath, String outputPath, int width, int height) {
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

Vector<String> FfmpegProjectCommandBuilder::ExtractAudioAndInfo(String projectPath, String audioFilePath, String infoFilePath) {
    return {
        "-dump_attachment:t:0", infoFilePath,
        "-i", projectPath,
        "-map", "0:a:0",
        "-c:a", "copy",
        audioFilePath
    };
}

Vector<String> FfmpegProjectCommandBuilder::ExtractVideo(String projectPath, String videoFilePath) {
    return {
        "-i", projectPath,
        "-map", "0:v:0",
        "-c:v", "copy",
        videoFilePath
    };
}

Vector<String> FfmpegProjectCommandBuilder::SaveWithBackgroundVideo(const KarData& data, String outputPath) {
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

Vector<String> FfmpegProjectCommandBuilder::SaveWithVisualization(const KarData& data, String outputPath) {
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
