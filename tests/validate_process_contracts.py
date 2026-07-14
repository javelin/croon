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
        '#include "KarData.h"\n#include "Visualization.h"',
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
        '#include "KarData.h"\n#include "Visualization.h"\n#include "FfmpegAudioCommandBuilder.h"',
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
        "FfmpegAudioCommandBuilder::ConvertToVorbis(audioPath, outputPath)",
        "process.Start(ffmpeg",
    ]:
        if needle not in convert_impl:
            fail(f"ConvertDlg.cpp missing conversion workflow {needle}")

    download_header = (root / "DownloadDlg.h").read_text()
    download_impl = (root / "DownloadDlg.cpp").read_text()
    if "HttpRequest" in download_header:
        fail("DownloadDlg.h exposes HttpRequest storage")
    if "RequestState* request" in download_header:
        fail("DownloadDlg.h uses raw request-state ownership")
    if "ExtractLyrics" in download_header:
        fail("DownloadDlg.h still declares lyrics extraction")
    if "new RequestState" in download_impl:
        fail("DownloadDlg.cpp uses raw request-state allocation")
    if "DownloadDlg::ExtractLyrics" in download_impl:
        fail("DownloadDlg.cpp still implements lyrics extraction")
    for needle in [
        "#include <memory>",
        "struct RequestState",
        "std::unique_ptr<RequestState> request",
        "String GetContent() const",
    ]:
        if needle not in download_header:
            fail(f"DownloadDlg.h missing opaque request contract {needle}")
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
        '#include "KarData.h"\n#include "Visualization.h"',
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
        "struct DownloadDlg::RequestState",
        "HttpRequest request",
        "request(std::make_unique<RequestState>())",
        "DownloadDlg::~DownloadDlg() = default",
        "DownloadDlg::RequestState& DownloadDlg::Request()",
        "String DownloadDlg::GetContent() const",
        "Request().request.Abort()",
        "HttpRequest::FINISHED - HttpRequest::BEGIN",
        "Request().request.Url(url).UserAgent(userAgent).New()",
        "Request().request.Do()",
        "Request().request.IsSuccess()",
        "WhenDownloadSuccess(Request().request.GetContent())",
    ]:
        if needle not in download_impl:
            fail(f"DownloadDlg.cpp missing download workflow {needle}")

    export_impl = (root / "ExportDlg.cpp").read_text()
    export_header = (root / "ExportDlg.h").read_text()
    if "PostCallback([=] { StartNextProcess(); })" in export_header:
        fail("ExportDlg.h still owns export run process kickoff")
    if "AppIdentity::TaggedTempFileName(\"dehissed\", \".ogg\")" in export_header:
        fail("ExportDlg.h still owns export run temp file setup")
    if "int Run(const KarData& karData, String outputPath, int len=1, double thumbnailTS=-1.0f);" not in export_header:
        fail("ExportDlg.h missing export run declaration")
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
        '#include "KarData.h"\n#include "Visualization.h"\n#include "FfmpegAudioCommandBuilder.h"\n#include "FfmpegExportCommandBuilder.h"',
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
        "int ExportDlg::Run(const KarData& karData, String outputPath, int len, double thumbnailTS)",
        "data = &karData",
        "this->outputPath = outputPath",
        "AppIdentity::TaggedTempFileName(\"dehissed\", \".ogg\")",
        "PostCallback([=] { StartNextProcess(); })",
        "return RunDlg(\"Export Video\")",
        "FfmpegProgressParser::ParseTimestamp",
        "AppIdentity::TempFileName(\".ass\")",
        "SubtitleGenerator::ToAss(*data, 4)",
        "FfmpegAudioCommandBuilder::Dehiss",
        "FfmpegExportCommandBuilder::WithVisualization",
        "FfmpegExportCommandBuilder::WithBackgroundVideo",
        "FfmpegExportCommandBuilder::GenerateCoverImage",
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
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "UiScaler.h"',
        '#include "LyricsPartsCtrl.h"',
        '#include "ListCtrl.h"',
        '#include "AppIdentity.h"',
        '#include "KarData.h"\n#include "Visualization.h"',
        '#include "LyricsTransformer.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ProjectLoader.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "ProgressDlg.h"',
        '#include "VideoCatalog.h"',
        '#include "GatherDlg.h"',
    ]:
        if needle not in gather_impl:
            fail(f"GatherDlg.cpp missing direct dependency {needle}")
    for needle in [
        "Config::Get(FFMPEG_LOCATION)",
        "VideoCatalog::FindVideoFiles(videoDir)",
        "VideoCatalog::ThumbnailPath(paths[curPath])",
        "VideoCatalog::HasThumbnail(paths[curPath])",
        "VideoCatalog::LoadThumbnail(paths[curPath])",
        "VideoCatalog::DeleteThumbnail(paths[curPath])",
        "VideoCatalog::BuildThumbnailCommand(paths[curPath])",
        "VideoCatalog::DisplayName(paths[curPath], 60)",
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
        '#include "ProjectSerializer.h"\n#include "Visualization.h"\n#include "FfmpegProjectCommandBuilder.h"\n#include "FfmpegThumbnailCommandBuilder.h"',
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
        "ProjectSerializer::MetadataCompatibility compatibility = ProjectSerializer::ReadCompatibility(metadata)",
        "compatibility == ProjectSerializer::UnsupportedMetadata",
        "ProjectSerializer::ReadVersion(metadata)",
        "compatibility == ProjectSerializer::InvalidMetadata",
        "not valid JSON",
        "KarData temp{metadata}",
        "LyricsTransformer::TimedToRaw(temp.timedLyrics)",
        "Visualization::Thumbnail(data->origVideoFile)",
        "FfmpegProjectCommandBuilder::ExtractAudioAndInfo",
        "FfmpegProjectCommandBuilder::ExtractVideo",
        "FfmpegThumbnailCommandBuilder::Generate",
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
        '#include "KarData.h"\n#include "Visualization.h"\n#include "FfmpegProjectCommandBuilder.h"',
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
        "FfmpegProjectCommandBuilder::SaveWithVisualization(*data, tempFilename)",
        "FfmpegProjectCommandBuilder::SaveWithBackgroundVideo(*data, tempFilename)",
        "process.Start(ffmpeg",
    ]:
        if needle not in save_impl:
            fail(f"SaveProjectDlg.cpp missing save workflow {needle}")

    loader_h = (root / "ProjectLoader.h").read_text()
    if "MediaProcessRunner process;" not in loader_h:
        fail("ProjectLoader does not use MediaProcessRunner")
    if "Config::Get(FFMPEG_LOCATION)" in loader_h:
        fail("ProjectLoader.h performs inline configuration lookup")

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
