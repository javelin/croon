/*
 * File  : Ffmpeg.h
 * Author: Mark Documento
 */

#ifndef _Croon_Ffmpeg_h_
#define _Croon_Ffmpeg_h_

struct Ffmpeg {
    static Vector<String> ConvertAudioToVorbis(String audioPath, String outputPath) {
        return {
            "-i",
            audioPath,
            "-vn",
            "-acodec",
            "libvorbis",
            outputPath
        };
    }
    
    static Vector<String> DehissAudio(String audioPath, String outputPath, int factor=30) {
        return {
            "-i",
            audioPath,
            "-af",
            Format("afftdn=nr=%d", factor),
            outputPath
        };
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
            "-metadata", Format("lyrics=%s", TimedLyricsToRaw(data.timedLyrics, true)),
            outputPath
        };
    }
    
    static Vector<String> ExportWithVisualization(const KarData& data, String assFilePath, String outputPath, String audioFilepath, bool preview=true) {
        auto assFn{assFilePath};
#ifdef PLATFORM_WIN32
        assFn.Replace("\\", "/");
        assFn.Replace(":", "\\\\:");
#endif
        auto filter = VIZ::Filter(data.videoFilePath, assFn, preview);
        
        return filter.IsVoid() ? Vector<String>{} : Vector<String>{
            "-f",
            "lavfi",
            "-i",
            Format("color=black:s=%s", VIZ::Reso(preview, 'x')),
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
            "-metadata", Format("lyrics=%s", TimedLyricsToRaw(data.timedLyrics, true)),
            outputPath
        };
    }
    
    static Vector<String> GenerateCoverImage(String videoPath, String outputPath, double thumbnailTS) {
        return {
            "-i",
            videoPath,
            "-ss",
            FormatTime2(thumbnailTS),
            "-frames:v",
            "1",
            outputPath
        };
    }
    
    static Vector<String> GenerateThumbnail(String videoPath, String outputPath, int width, int height) {
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
