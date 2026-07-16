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

bool SubtitleWrapProbeRunner::Run(const KarData& data,
                                  const Vector<String>& lyrics,
                                  Vector<SubtitleWrapProbeFrame>& frames,
                                  String ffmpegPath,
                                  int resX,
                                  int resY,
                                  int cropY,
                                  int cropHeight) {
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

    if (!SaveFile(assPath, SubtitleWrapProbe::BuildAss(data, lyrics, resX, resY))) {
        cleanup();
        return false;
    }

    Vector<String> args = FfmpegSubtitleProbeCommandBuilder::RenderRgba(
        assPath, rgbaPath, lyrics.GetCount(), resX, resY, cropY, cropHeight);

    MediaProcessRunner process;
    if (!process.Start(ffmpegPath, args)) {
        cleanup();
        return false;
    }

    String output;
    while (process.Read(output) || process.IsRunning())
        Sleep(10);

    if (process.GetExitCode() != 0) {
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
