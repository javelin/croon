/*
 * File  : FfmpegExportCommandBuilder.cpp
 * Author: Mark Documento
 */

#include <Draw/Draw.h>

using namespace Upp;

#include "KarData.h"
#include "LyricsTransformer.h"
#include "TimeFormatter.h"
#include "Visualization.h"
#include "FfmpegExportCommandBuilder.h"

Vector<String> FfmpegExportCommandBuilder::WithBackgroundVideo(const KarData& data, String assFilePath, String outputPath, String audioFilepath, int scaleHeight) {
    String filterComplex;
    auto assFn{assFilePath};
#ifdef PLATFORM_WIN32
    assFn.Replace("\\", "/");
    assFn.Replace(":", "\\\\:");
#endif
    if (scaleHeight > 0) {
        filterComplex = Format("[0:v]scale=-2:%d,subtitles=%s[v]", scaleHeight, assFn);
    }
    else {
        filterComplex = Format("[0:v]subtitles=%s[v]", assFn);
    }

    return {
        "-stream_loop",
        "-1",
        "-i",
        data.videoFilePath,
        "-i",
        audioFilepath.IsEmpty() ? data.audioFilePath:audioFilepath,
        "-map_metadata:s", "-1",
        "-filter_complex", filterComplex,
        "-map",
        "[v]",
        "-map",
        "1:a",
        "-shortest",
        "-c:v",
        "libx265",
        "-c:a",
        "copy",
        "-metadata", Format("title=%s", data.title),
        "-metadata", Format("artist=%s", data.artist),
        "-metadata", Format("composer=%s", data.writer),
        "-metadata", Format("copyright=%s", data.owner),
        "-metadata", Format("genre=%s", data.genre),
        "-metadata", Format("year=%d", data.year),
        "-metadata", Format("comment=Year: %d\nOriginal Video: %s", data.year, data.origVideoFile),
        "-metadata", Format("lyrics=%s", LyricsTransformer::TimedToRaw(data.timedLyrics, true)),
        outputPath
    };
}

Vector<String> FfmpegExportCommandBuilder::WithVisualization(const KarData& data, String assFilePath, String outputPath, String audioFilepath, int canvasHeight) {
    auto assFn{assFilePath};
#ifdef PLATFORM_WIN32
    assFn.Replace("\\", "/");
    assFn.Replace(":", "\\\\:");
#endif
    auto filter = Visualization::Filter(data.videoFilePath, assFn, canvasHeight);

    return filter.IsVoid() ? Vector<String>{} : Vector<String>{
        "-f",
        "lavfi",
        "-i",
        Format("color=black:s=%s", Visualization::Reso(canvasHeight, 'x')),
        "-i",
        audioFilepath.IsEmpty() ? data.audioFilePath:audioFilepath,
        "-filter_complex",
        filter,
        "-shortest",
        "-c:v",
        "libx265",
        "-c:a",
        "copy",
        "-metadata", Format("title=%s", data.title),
        "-metadata", Format("artist=%s", data.artist),
        "-metadata", Format("composer=%s", data.writer),
        "-metadata", Format("copyright=%s", data.owner),
        "-metadata", Format("genre=%s", data.genre),
        "-metadata", Format("year=%d", data.year),
        "-metadata", Format("comment=Year: %d\nOriginal Video: %s", data.year, data.origVideoFile),
        "-metadata", Format("lyrics=%s", LyricsTransformer::TimedToRaw(data.timedLyrics, true)),
        outputPath
    };
}

Vector<String> FfmpegExportCommandBuilder::GenerateCoverImage(String videoPath, String outputPath, double thumbnailTS) {
    return {
        "-i",
        videoPath,
        "-ss",
        TimeFormatter::Clock(thumbnailTS),
        "-frames:v",
        "1",
        outputPath
    };
}
