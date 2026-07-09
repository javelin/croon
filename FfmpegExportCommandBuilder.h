/*
 * File  : FfmpegExportCommandBuilder.h
 * Author: Mark Documento
 */

#ifndef _Croon_FfmpegExportCommandBuilder_h_
#define _Croon_FfmpegExportCommandBuilder_h_

#include "LyricsTransformer.h"
#include "TimeFormatter.h"
#include "Visualization.h"

struct FfmpegExportCommandBuilder {
    static Vector<String> WithBackgroundVideo(const KarData& data, String assFilePath, String outputPath, String audioFilepath="", bool preview=true) {
        String filterComplex;
        auto assFn{assFilePath};
#ifdef PLATFORM_WIN32
        assFn.Replace("\\", "/");
        assFn.Replace(":", "\\\\:");
#endif
        if (preview) {
            filterComplex = Format("[0:v]scale=-2:180,subtitles=%s[v]", assFn);
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
            "libx264",
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
    
    static Vector<String> WithVisualization(const KarData& data, String assFilePath, String outputPath, String audioFilepath, bool preview=true) {
        auto assFn{assFilePath};
#ifdef PLATFORM_WIN32
        assFn.Replace("\\", "/");
        assFn.Replace(":", "\\\\:");
#endif
        auto filter = Visualization::Filter(data.videoFilePath, assFn, preview);
        
        return filter.IsVoid() ? Vector<String>{} : Vector<String>{
            "-f",
            "lavfi",
            "-i",
            Format("color=black:s=%s", Visualization::Reso(preview, 'x')),
            "-i",
            audioFilepath.IsEmpty() ? data.audioFilePath:audioFilepath,
            "-filter_complex",
            filter,
            "-shortest",
            "-c:v",
            "libx264",
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
    
    static Vector<String> GenerateCoverImage(String videoPath, String outputPath, double thumbnailTS) {
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
};

#endif
