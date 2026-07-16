/*
 * File  : SubtitleWrapProbeRunner.cpp
 * Author: Mark Documento
 */

#include <Draw/Draw.h>

using namespace Upp;

#include "AppIdentity.h"
#include "Config.h"
#include "FfmpegSubtitleProbeCommandBuilder.h"
#include "KarData.h"
#include "MediaProcessRunner.h"
#include "SubtitleWrapProbe.h"
#include "SubtitleWrapProbeRunner.h"

namespace {

bool RunProcess(String exe, const Vector<String>& args) {
    MediaProcessRunner process;
    if (!process.Start(exe, args))
        return false;

    String output;
    while (process.Read(output) || process.IsRunning())
        Sleep(10);
    return process.GetExitCode() == 0;
}

void SaveDebugProbeFiles(String debugDir,
                         String ffmpegPath,
                         String assPath,
                         int frameCount,
                         int resX,
                         int resY,
                         int cropY,
                         int cropHeight,
                         int effectiveFontSize,
                         bool probeBold) {
    debugDir = TrimBoth(debugDir);
    if (debugDir.IsEmpty())
        return;
    if (!DirectoryExists(debugDir) && !DirectoryCreate(debugDir))
        return;

    String label = Format("wrap_probe_%dx%d_%d_%s",
                          resX,
                          resY,
                          effectiveFontSize,
                          probeBold ? "bold":"regular");
    String debugAssPath = AppendFileName(debugDir, label + ".ass");
    SaveFile(debugAssPath, LoadFile(assPath));

    String outputPattern = AppendFileName(debugDir, label + "_%04d.png");
    RunProcess(ffmpegPath, FfmpegSubtitleProbeCommandBuilder::RenderPngFrames(
        assPath, outputPattern, frameCount, resX, resY, cropY, cropHeight));
}

}

bool SubtitleWrapProbeRunner::Run(const KarData& data,
                                  const Vector<String>& lyrics,
                                  Vector<SubtitleWrapProbeFrame>& frames,
                                  String ffmpegPath,
                                  int resX,
                                  int resY,
                                  int cropY,
                                  int cropHeight,
                                  int probeFontSize,
                                  bool probeBold) {
    frames.Clear();
    if (lyrics.IsEmpty())
        return true;

    if (ffmpegPath.IsVoid())
        ffmpegPath = Config::Get(FFMPEG_LOCATION);
    if (TrimBoth(ffmpegPath).IsEmpty())
        return false;

    String assPath = AppIdentity::TaggedTempFileName("subprobe", ".ass");
    String rgbaPath = AppIdentity::TaggedTempFileName("subprobe", ".rgba");
    auto cleanup = [&] {
        FileDelete(assPath);
        FileDelete(rgbaPath);
    };

    if (!SaveFile(assPath, SubtitleWrapProbe::BuildAss(data, lyrics, resX, resY, probeFontSize, probeBold))) {
        cleanup();
        return false;
    }

    int effectiveFontSize = probeFontSize > 0 ? probeFontSize:data.fontSize;
    SaveDebugProbeFiles(GetEnv("CROON_WRAP_PROBE_DIR"),
                        ffmpegPath,
                        assPath,
                        lyrics.GetCount(),
                        resX,
                        resY,
                        cropY,
                        cropHeight,
                        effectiveFontSize,
                        probeBold);

    Vector<String> args = FfmpegSubtitleProbeCommandBuilder::RenderRgba(
        assPath, rgbaPath, lyrics.GetCount(), resX, resY, cropY, cropHeight);

    if (!RunProcess(ffmpegPath, args)) {
        cleanup();
        return false;
    }

    String rgba = LoadFile(rgbaPath);
    if (rgba.IsVoid()) {
        cleanup();
        return false;
    }

    frames = SubtitleWrapProbe::AnalyzeRgbaFrames(rgba, resX, cropHeight, lyrics.GetCount());
    cleanup();
    return frames.GetCount() == lyrics.GetCount();
}
