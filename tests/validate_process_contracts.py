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
    for inline_body in [
        "Title(title).Run()",
        "WhenProcessAborted(false)",
        "WhenProcessAborted(true)",
        "return WhenProcessEnded(code)",
        "WhenProcessOutput(output)",
        "WhenStartNextProcess()",
        "WhenUpdateProgress()",
    ]:
        if inline_body in header:
            fail(f"ProgressDlg.h still owns inline process wrapper {inline_body}")

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
        "int ProgressDlg::RunDlg(const char* title)",
        "Title(title).Run()",
        "void ProgressDlg::ProcessAborted()",
        "WhenProcessAborted(false)",
        "void ProgressDlg::ProcessAbortedByUser()",
        "WhenProcessAborted(true)",
        "bool ProgressDlg::ProcessEnded(int code)",
        "return WhenProcessEnded(code)",
        "void ProgressDlg::ProcessOutput(String output)",
        "WhenProcessOutput(output)",
        "void ProgressDlg::StartNextProcess()",
        "WhenStartNextProcess()",
        "void ProgressDlg::UpdateProgress()",
        "WhenUpdateProgress()",
    ]:
        if needle not in impl:
            fail(f"ProgressDlg.cpp missing process wrapper {needle}")
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
        '#include "SubtitleWrapProbeRunner.h"',
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
        "ReadVideoSizeWithFfprobe(ffmpegPath, data.videoFilePath, videoSize)",
        "ReadVideoSizeWithFfmpeg(ffmpegPath, data.videoFilePath, videoSize)",
        "AppIdentity::TempFileName(\".ass\")",
        "SubtitleGenerator::HighlightProbeLyrics(*data, data->subtitleLines)",
        "Size probeCanvas = ProbeCanvasForExport(*data, ffmpeg)",
        "SubtitleWrapProbeRunner::Run(*data, probeLyrics, probeFrames, ffmpeg,",
        "SubtitleWrapProbeRunner::Run(*data, probeLyrics, incomingProbeFrames, ffmpeg",
        "data->subtitleLines, probeCanvas.cx, probeCanvas.cy)",
        "FfmpegAudioCommandBuilder::Dehiss",
        "FfmpegExportCommandBuilder::WithVisualization",
        "FfmpegExportCommandBuilder::WithBackgroundVideo",
        "FfmpegExportCommandBuilder::GenerateCoverImage",
        "process.Start(ffmpeg",
    ]:
        if needle not in export_impl:
            fail(f"ExportDlg.cpp missing export workflow {needle}")
    if "ReadVideoSizeFromDecodedFrame" in export_impl:
        fail("ExportDlg should not decode a PNG frame for video size probing")
    if "ProgressDlg::Run(\"Export Video\")" in export_impl:
        fail("ExportDlg nests modal progress runs across export phases")
    if "runCode" in export_header or "runCode" in export_impl:
        fail("ExportDlg still tracks obsolete nested run state")

    for retired in ["GatherDlg.cpp", "GatherDlg.h"]:
        if (root / retired).exists():
            fail(f"{retired} should remain retired")

    open_impl = (root / "OpenProjectDlg.cpp").read_text()
    open_header = (root / "OpenProjectDlg.h").read_text()
    if "void Close() override {}" in open_header:
        fail("OpenProjectDlg.h still owns inline Close override")
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
        "void OpenProjectDlg::Close()",
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
    save_header = (root / "SaveProjectDlg.h").read_text()
    if "return Run(data.projectPath, data)" in save_header:
        fail("SaveProjectDlg.h still owns inline Run(KarData&) forwarding")
    if "void Close() override {}" in save_header:
        fail("SaveProjectDlg.h still owns inline Close override")
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
        "int SaveProjectDlg::Run(KarData& karData)",
        "return Run(karData.projectPath, karData)",
        "void SaveProjectDlg::Close()",
        "std::filesystem::copy",
        "AppIdentity::ProjectTempFileName()",
        "SaveFile(data->infoFilePath, data->ToJSONStr())",
        "FfmpegProjectCommandBuilder::SaveWithVisualization(*data, tempFilename)",
        "FfmpegProjectCommandBuilder::SaveWithBackgroundVideo(*data, tempFilename)",
        "process.Start(ffmpeg",
        "Exclamation(\"Unable to save project!\");\n        Break(IDCANCEL);",
    ]:
        if needle not in save_impl:
            fail(f"SaveProjectDlg.cpp missing save workflow {needle}")

    loader_h = (root / "ProjectLoader.h").read_text()
    if "MediaProcessRunner process;" not in loader_h:
        fail("ProjectLoader does not use MediaProcessRunner")
    if "void StopLoading();" not in loader_h:
        fail("ProjectLoader does not expose background loading stop")
    if "Config::Get(FFMPEG_LOCATION)" in loader_h:
        fail("ProjectLoader.h performs inline configuration lookup")

    croon_cpp = (root / "Croon.cpp").read_text()
    if "MediaProcessRunner proc;" not in croon_cpp:
        fail("RunCroon does not validate ffmpeg through MediaProcessRunner")

    runner_h = (root / "MediaProcessRunner.h").read_text()
    if "return process.Start" in runner_h or "return process.Read" in runner_h or "process.Kill();" in runner_h:
        fail("MediaProcessRunner.h still owns LocalProcess forwarding bodies")

    runner_cpp = (root / "MediaProcessRunner.cpp").read_text()
    for needle in [
        "process.Start(command)",
        "process.Start(executable, args)",
        "process.Start(executable, args, envptr, dir)",
        "process.Read(output)",
        "process.IsRunning()",
        "process.GetExitCode()",
        "process.Kill()",
    ]:
        if needle not in runner_cpp:
            fail(f"MediaProcessRunner.cpp missing forwarding body {needle}")

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
