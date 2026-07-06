#!/usr/bin/env python3
import sys
from pathlib import Path


def fail(message: str) -> None:
    print(f"validate_process_contracts: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if len(sys.argv) != 2:
        fail("expected repository root argument")

    root = Path(sys.argv[1])
    lay = (root / "Croon.lay").read_text()
    for needle in [
        "LAYOUT(CroonProgressLayout",
        "ITEM(Label, monitor",
        "ITEM(ProgressIndicator, progress",
        "ITEM(Button, cancelBtn",
    ]:
        if needle not in lay:
            fail(f"Croon.lay missing {needle}")

    header = (root / "ProgressDlg.h").read_text()
    if "WithCroonProgressLayout<TopWindow>" not in header:
        fail("ProgressDlg is not backed by CroonProgressLayout")
    if "MediaProcessRunner process;" not in header:
        fail("ProgressDlg does not use MediaProcessRunner")
    if "LocalProcess process;" in header:
        fail("ProgressDlg still owns LocalProcess directly")
    for old_member in ["Label monitor;", "ProgressIndicator progress;", "Button cancelBtn;"]:
        if old_member in header:
            fail(f"ProgressDlg still declares layout-owned member {old_member}")

    impl = (root / "ProgressDlg.cpp").read_text()
    if '#include "Croon.h"' in impl:
        fail("ProgressDlg.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#define IMAGECLASS CroonImg",
        "#define IMAGEFILE <Croon/Croon.iml>",
        "#include <Draw/iml_header.h>",
        '#include "Constants.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "UiScaler.h"',
        '#include "LyricsPartsCtrl.h"',
        '#include "ListCtrl.h"',
        '#include "AppIdentity.h"',
        '#include "KarData.h"\n#include "Visualization.h"\n#include "FfmpegCommandBuilder.h"',
        '#include "LyricsTransformer.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ProjectLoader.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "ProgressDlg.h"',
    ]:
        if needle not in impl:
            fail(f"ProgressDlg.cpp missing direct dependency {needle}")
    if "CtrlLayout(*this, \"Progress\")" not in impl:
        fail("ProgressDlg constructor does not apply Designer layout")
    for needle in [
        "process.IsRunning()",
        "process.Kill()",
        "process.Read(output)",
        "process.GetExitCode()",
    ]:
        if needle not in impl:
            fail(f"ProgressDlg.cpp missing process boundary {needle}")
    process_text = "".join((root / rel).read_text() for rel in [
        "ConvertDlg.cpp",
        "ExportDlg.cpp",
        "GatherDlg.cpp",
        "OpenProjectDlg.cpp",
        "SaveProjectDlg.cpp",
    ])
    if "process.Start" not in process_text:
        fail("process dialogs no longer start external processes")

    convert_impl = (root / "ConvertDlg.cpp").read_text()
    if '#include "Croon.h"' in convert_impl:
        fail("ConvertDlg.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#define IMAGECLASS CroonImg",
        "#define IMAGEFILE <Croon/Croon.iml>",
        "#include <Draw/iml_header.h>",
        '#include "Constants.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "UiScaler.h"',
        '#include "LyricsPartsCtrl.h"',
        '#include "ListCtrl.h"',
        '#include "AppIdentity.h"',
        '#include "KarData.h"\n#include "Visualization.h"\n#include "FfmpegCommandBuilder.h"',
        '#include "LyricsTransformer.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ProjectLoader.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "ProgressDlg.h"',
        '#include "FfmpegProgressParser.h"',
        '#include "ConvertDlg.h"',
    ]:
        if needle not in convert_impl:
            fail(f"ConvertDlg.cpp missing direct dependency {needle}")
    for needle in [
        "Config::Get(FFMPEG_LOCATION)",
        "FfmpegProgressParser::ParseTimestamp",
        "AppIdentity::TempFileName(\".ogg\")",
        "FfmpegCommandBuilder::ConvertAudioToVorbis(audioPath, outputPath)",
        "process.Start(ffmpeg",
    ]:
        if needle not in convert_impl:
            fail(f"ConvertDlg.cpp missing conversion workflow {needle}")

    download_impl = (root / "DownloadDlg.cpp").read_text()
    if '#include "Croon.h"' in download_impl:
        fail("DownloadDlg.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#define IMAGECLASS CroonImg",
        "#define IMAGEFILE <Croon/Croon.iml>",
        "#include <Draw/iml_header.h>",
        '#include "Constants.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "UiScaler.h"',
        '#include "LyricsPartsCtrl.h"',
        '#include "ListCtrl.h"',
        '#include "AppIdentity.h"',
        '#include "KarData.h"\n#include "Visualization.h"\n#include "FfmpegCommandBuilder.h"',
        '#include "LyricsTransformer.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ProjectLoader.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "ProgressDlg.h"',
        '#include "DownloadDefaults.h"',
        '#include "DownloadDlg.h"',
    ]:
        if needle not in download_impl:
            fail(f"DownloadDlg.cpp missing direct dependency {needle}")
    for needle in [
        "request.Abort()",
        "HttpRequest::FINISHED - HttpRequest::BEGIN",
        "request.Url(url).UserAgent(userAgent).New()",
        "request.Do()",
        "request.IsSuccess()",
        "WhenDownloadSuccess(request.GetContent())",
    ]:
        if needle not in download_impl:
            fail(f"DownloadDlg.cpp missing download workflow {needle}")

    export_impl = (root / "ExportDlg.cpp").read_text()
    if '#include "Croon.h"' in export_impl:
        fail("ExportDlg.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#define IMAGECLASS CroonImg",
        "#define IMAGEFILE <Croon/Croon.iml>",
        "#include <Draw/iml_header.h>",
        '#include "Constants.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "UiScaler.h"',
        '#include "LyricsPartsCtrl.h"',
        '#include "ListCtrl.h"',
        '#include "AppIdentity.h"',
        '#include "KarData.h"\n#include "Visualization.h"\n#include "FfmpegCommandBuilder.h"',
        '#include "LyricsTransformer.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ProjectLoader.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "ProgressDlg.h"',
        '#include "FfmpegProgressParser.h"',
        '#include "SubtitleGenerator.h"',
        '#include "ExportDlg.h"',
    ]:
        if needle not in export_impl:
            fail(f"ExportDlg.cpp missing direct dependency {needle}")
    for needle in [
        "Config::Get(FFMPEG_LOCATION)",
        "FfmpegProgressParser::ParseTimestamp",
        "AppIdentity::TempFileName(\".ass\")",
        "SubtitleGenerator::ToAss(*data, 4)",
        "FfmpegCommandBuilder::DehissAudio",
        "FfmpegCommandBuilder::ExportWithVisualization",
        "FfmpegCommandBuilder::ExportWithBackgroundVideo",
        "FfmpegCommandBuilder::GenerateCoverImage",
        "process.Start(ffmpeg",
    ]:
        if needle not in export_impl:
            fail(f"ExportDlg.cpp missing export workflow {needle}")

    gather_impl = (root / "GatherDlg.cpp").read_text()
    if '#include "Croon.h"' in gather_impl:
        fail("GatherDlg.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#define IMAGECLASS CroonImg",
        "#define IMAGEFILE <Croon/Croon.iml>",
        "#include <Draw/iml_header.h>",
        '#include "Constants.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "UiScaler.h"',
        '#include "LyricsPartsCtrl.h"',
        '#include "ListCtrl.h"',
        '#include "AppIdentity.h"',
        '#include "KarData.h"\n#include "Visualization.h"\n#include "FfmpegCommandBuilder.h"',
        '#include "LyricsTransformer.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ProjectLoader.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "ProgressDlg.h"',
        '#include "AppPaths.h"',
        '#include "TextTools.h"',
        '#include "GatherDlg.h"',
    ]:
        if needle not in gather_impl:
            fail(f"GatherDlg.cpp missing direct dependency {needle}")
    for needle in [
        "Config::Get(FFMPEG_LOCATION)",
        "AppPaths::DataDirectory()",
        "AppPaths::FindFiles(videoDir, \"*.mp4\")",
        "TextTools::ShortenMiddle(paths[curPath], 60)",
        "FfmpegCommandBuilder::GenerateThumbnail(paths[curPath], tnPath, ThumbnailDim, ThumbnailDim)",
        "process.Start(ffmpeg",
    ]:
        if needle not in gather_impl:
            fail(f"GatherDlg.cpp missing thumbnail discovery workflow {needle}")

    open_impl = (root / "OpenProjectDlg.cpp").read_text()
    if '#include "Croon.h"' in open_impl:
        fail("OpenProjectDlg.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#define IMAGECLASS CroonImg",
        "#define IMAGEFILE <Croon/Croon.iml>",
        "#include <Draw/iml_header.h>",
        '#include "Constants.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "UiScaler.h"',
        '#include "LyricsPartsCtrl.h"',
        '#include "ListCtrl.h"',
        '#include "AppIdentity.h"',
        '#include "KarData.h"\n#include "ProjectSerializer.h"',
        '#include "ProjectSerializer.h"\n#include "Visualization.h"\n#include "FfmpegCommandBuilder.h"',
        '#include "LyricsTransformer.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ProjectLoader.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "ProgressDlg.h"',
        '#include "OpenProjectDlg.h"',
    ]:
        if needle not in open_impl:
            fail(f"OpenProjectDlg.cpp missing direct dependency {needle}")
    for needle in [
        "Config::Get(FFMPEG_LOCATION)",
        "String metadata = LoadFile(data->infoFilePath)",
        "ProjectSerializer::SupportsJson(metadata)",
        "ProjectSerializer::ReadVersion(metadata)",
        "KarData temp{metadata}",
        "LyricsTransformer::TimedToRaw(temp.timedLyrics)",
        "Visualization::Thumbnail(data->origVideoFile)",
        "FfmpegCommandBuilder::ProjectExtractAudioAndInfo",
        "FfmpegCommandBuilder::ProjectExtractVideo",
        "FfmpegCommandBuilder::GenerateThumbnail",
        "StreamRaster::LoadFileAny(data->thumbnailFilePath)",
        "process.Start(ffmpeg",
    ]:
        if needle not in open_impl:
            fail(f"OpenProjectDlg.cpp missing project-open workflow {needle}")

    save_impl = (root / "SaveProjectDlg.cpp").read_text()
    if '#include "Croon.h"' in save_impl:
        fail("SaveProjectDlg.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#include <filesystem>",
        "#define IMAGECLASS CroonImg",
        "#define IMAGEFILE <Croon/Croon.iml>",
        "#include <Draw/iml_header.h>",
        '#include "Constants.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "UiScaler.h"',
        '#include "LyricsPartsCtrl.h"',
        '#include "ListCtrl.h"',
        '#include "AppIdentity.h"',
        '#include "KarData.h"\n#include "Visualization.h"\n#include "FfmpegCommandBuilder.h"',
        '#include "LyricsTransformer.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ProjectLoader.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "ProgressDlg.h"',
        '#include "SaveProjectDlg.h"',
    ]:
        if needle not in save_impl:
            fail(f"SaveProjectDlg.cpp missing direct dependency {needle}")
    for needle in [
        "Config::Get(FFMPEG_LOCATION)",
        "std::filesystem::copy",
        "AppIdentity::ProjectTempFileName()",
        "SaveFile(data->infoFilePath, data->ToJSONStr())",
        "FfmpegCommandBuilder::ProjectSaveWithVisualization(*data, tempFilename)",
        "FfmpegCommandBuilder::ProjectSaveWithBackgroundVideo(*data, tempFilename)",
        "process.Start(ffmpeg",
    ]:
        if needle not in save_impl:
            fail(f"SaveProjectDlg.cpp missing save workflow {needle}")

    loader_h = (root / "ProjectLoader.h").read_text()
    if "MediaProcessRunner process;" not in loader_h:
        fail("ProjectLoader does not use MediaProcessRunner")

    croon_cpp = (root / "Croon.cpp").read_text()
    if "MediaProcessRunner proc;" not in croon_cpp:
        fail("RunCroon does not validate ffmpeg through MediaProcessRunner")

    raw_process_owners = []
    for path in root.glob("*.h"):
        if path.name == "MediaProcessRunner.h":
            continue
        if "LocalProcess" in path.read_text():
            raw_process_owners.append(path.name)
    if raw_process_owners:
        fail(f"headers still own LocalProcess directly: {', '.join(raw_process_owners)}")


if __name__ == "__main__":
    main()
