#!/usr/bin/env python3
import sys
from pathlib import Path


def fail(message: str) -> None:
    print(f"validate_core_contracts: {message}", file=sys.stderr)
    raise SystemExit(1)


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        fail(f"{label} missing {needle!r}")


def reject(text: str, needle: str, label: str) -> None:
    if needle in text:
        fail(f"{label} still contains {needle!r}")


def main() -> None:
    if len(sys.argv) != 2:
        fail("expected repository root argument")

    root = Path(sys.argv[1])

    identity_h = (root / "AppIdentity.h").read_text()
    for needle in [
        'ProductName() { return "Croon"; }',
        'Version() { return "1.0"; }',
        'ProjectExtension() { return ".croon"; }',
        'ProjectGlob() { return "*.croon"; }',
        'MetadataAttachmentName() { return "croon.info"; }',
        'TempPrefix() { return "Croon_"; }',
        'PosixDataDirectory() { return ".Croon"; }',
        'DataDirectory() { return "Croon"; }',
    ]:
        require(identity_h, needle, "AppIdentity contract")

    app_paths_cpp = (root / "AppPaths.cpp").read_text()
    require(app_paths_cpp, "AppIdentity::PosixDataDirectory", "POSIX app data contract")
    require(app_paths_cpp, "AppIdentity::DataDirectory", "non-POSIX app data contract")
    require(app_paths_cpp, "DirectoryCreate(dataDir)", "AppPaths data directory creation")
    require(app_paths_cpp, "PatternMatchMulti(pattern, ff.GetName())", "AppPaths file discovery")

    config_h = (root / "Config.h").read_text()
    require(config_h, "ConfigService::Get(key, defaultValue)", "Config get delegation")
    require(config_h, "ConfigService::Set(key, value)", "Config set delegation")
    require(config_h, "ConfigService::GetInt(key, defaultValue)", "Config int get delegation")
    require(config_h, "ConfigService::GetFontSize()", "Config font-size delegation")

    config_service_cpp = (root / "ConfigService.cpp").read_text()
    for key in [
        "FFMPEG_LOCATION",
        "FONT_SIZE",
        "MUSIC_DIR",
        "PROJECT_DIR",
        "PROJECT_LIST",
        "WIN_X",
        "WIN_H",
    ]:
        require(config_service_cpp, key, "ConfigService registered key contract")
    require(config_service_cpp, "SerializeGlobalConfigs", "ConfigService persistence contract")
    require(config_service_cpp, "std::max(MinFontSize, std::min(MaxFontSize", "ConfigService font-size clamp")

    recent_project_service_cpp = (root / "RecentProjectService.cpp").read_text()
    require(recent_project_service_cpp, "ConfigService::Get(PROJECT_LIST)", "RecentProjectService load contract")
    require(recent_project_service_cpp, "ConfigService::Set(PROJECT_LIST", "RecentProjectService save contract")
    require(recent_project_service_cpp, "TrimBoth(path)", "RecentProjectService path normalization")
    require(recent_project_service_cpp, "FindPathIndex(normalized, trimmed)", "RecentProjectService de-duplication")

    lyrics_transformer_h = (root / "LyricsTransformer.h").read_text()
    require(lyrics_transformer_h, "SplitDecorations", "LyricsTransformer decoration contract")
    require(lyrics_transformer_h, "RawToUntimed", "LyricsTransformer raw-to-timed contract")
    require(lyrics_transformer_h, "TimedToRaw", "LyricsTransformer timed-to-raw contract")

    lyrics_transformer_cpp = (root / "LyricsTransformer.cpp").read_text()
    require(lyrics_transformer_cpp, "MaxLineLength", "LyricsTransformer wrapping contract")
    require(lyrics_transformer_cpp, "removeMetadata", "LyricsTransformer metadata removal contract")

    subtitle_line_processor_h = (root / "SubtitleLineProcessor.h").read_text()
    require(subtitle_line_processor_h, "ProcessMetadata", "SubtitleLineProcessor metadata contract")
    require(subtitle_line_processor_h, "ResolveVocalPart", "SubtitleLineProcessor vocal-part contract")
    require(subtitle_line_processor_h, "ResolveStyle", "SubtitleLineProcessor style contract")

    subtitle_line_processor_cpp = (root / "SubtitleLineProcessor.cpp").read_text()
    require(subtitle_line_processor_cpp, "CountInDuration", "SubtitleLineProcessor count-in contract")
    require(subtitle_line_processor_cpp, "ReplaceMetadata", "SubtitleLineProcessor metadata replacement")
    require(subtitle_line_processor_cpp, "LookaheadVocalPart", "SubtitleLineProcessor count-in lookahead")

    subtitle_generator_h = (root / "SubtitleGenerator.h").read_text()
    require(subtitle_generator_h, "ToAss", "SubtitleGenerator ASS contract")
    require(subtitle_generator_h, "ToRichAss", "SubtitleGenerator rich ASS contract")

    media_process_runner_h = (root / "MediaProcessRunner.h").read_text()
    for method in [
        "Start",
        "Read",
        "IsRunning",
        "GetExitCode",
        "Kill",
    ]:
        require(media_process_runner_h, method, "MediaProcessRunner process boundary")
    require(media_process_runner_h, "LocalProcess process", "MediaProcessRunner local process ownership")

    subtitle_generator_cpp = (root / "SubtitleGenerator.cpp").read_text()
    require(subtitle_generator_cpp, "String SubtitleGenerator::ToAss", "SubtitleGenerator ASS implementation")
    require(subtitle_generator_cpp, "String SubtitleGenerator::ToRichAss", "SubtitleGenerator rich ASS implementation")

    text_tools_h = (root / "TextTools.h").read_text()
    for method in ["CleanSpacing", "StripNonAlnum", "ShortenMiddle"]:
        require(text_tools_h, method, "TextTools contract")

    text_tools_cpp = (root / "TextTools.cpp").read_text()
    require(text_tools_cpp, "TrimBoth(vw[i])", "TextTools spacing cleanup")
    require(text_tools_cpp, "IsAlNum(c)", "TextTools alphanumeric filter")
    require(text_tools_cpp, "\"...\"", "TextTools middle shortening")

    time_formatter_h = (root / "TimeFormatter.h").read_text()
    for method in ["CountInDuration", "Format", "Ass", "Srt", "Clock"]:
        require(time_formatter_h, method, "TimeFormatter contract")

    time_formatter_cpp = (root / "TimeFormatter.cpp").read_text()
    require(time_formatter_cpp, "std::round(dur*100.0f)/100.0f", "TimeFormatter count-in rounding")
    require(time_formatter_cpp, "decimal", "TimeFormatter decimal separator contract")
    require(time_formatter_cpp, "Mid(1)", "TimeFormatter ASS timestamp contract")

    util_cpp = (root / "Util.cpp").read_text()
    require(util_cpp, "return AppPaths::DataDirectory", "GetDataDirectory compatibility wrapper")
    require(util_cpp, "return AppPaths::FindFiles", "GetPaths compatibility wrapper")
    require(util_cpp, "return LyricsTransformer::SplitDecorations", "SplitLyrics compatibility wrapper")
    require(util_cpp, "return LyricsTransformer::RawToUntimed", "RawToUntimedLyrics compatibility wrapper")
    require(util_cpp, "return LyricsTransformer::TimedToRaw", "TimedLyricsToRaw compatibility wrapper")
    require(util_cpp, "return SubtitleLineProcessor::ReplaceMetadata", "ReplaceMetadata compatibility wrapper")
    require(util_cpp, "SubtitleLineProcessor::ProcessMetadata", "ProcessMetadata compatibility wrapper")
    require(util_cpp, "return SubtitleLineProcessor::ResolveVocalPart", "ResolveVocalPart compatibility wrapper")
    require(util_cpp, "return SubtitleLineProcessor::ResolveStyle", "ResolveStyle compatibility wrapper")
    require(util_cpp, "return SubtitleLineProcessor::ResolveDimStyle", "ResolveDimStyle compatibility wrapper")
    require(util_cpp, "return SubtitleGenerator::ToAss", "TimedToASS compatibility wrapper")
    require(util_cpp, "return SubtitleGenerator::ToRichAss", "TimedToRichASS compatibility wrapper")

    util_h = (root / "Util.h").read_text()
    require(util_h, '#include "SubtitleLineProcessor.h"', "Util.h subtitle type dependency")
    require(util_h, '#include "TextTools.h"', "Util.h text helper dependency")
    require(util_h, '#include "TimeFormatter.h"', "Util.h time type dependency")
    require(util_h, "return TextTools::CleanSpacing", "CleanSpacing compatibility wrapper")
    require(util_h, "return TextTools::StripNonAlnum", "StripNonAlnum compatibility wrapper")
    require(util_h, "return TextTools::ShortenMiddle", "ShortenMiddle compatibility wrapper")
    require(util_h, "return TimeFormatter::CountInDuration", "CountInDuration compatibility wrapper")
    require(util_h, "return TimeFormatter::Format", "FormatTime compatibility wrapper")
    require(util_h, "return TimeFormatter::Ass", "FormatTimeASS compatibility wrapper")
    require(util_h, "return TimeFormatter::Srt", "FormatTimeSRT compatibility wrapper")
    require(util_h, "return TimeFormatter::Clock", "FormatTime2 compatibility wrapper")

    kar_data_cpp = (root / "KarData.cpp").read_text()
    require(kar_data_cpp, "ProjectSerializer::ToJson(*this)", "KarData serialization delegation")
    require(kar_data_cpp, "ProjectSerializer::FromJson(JSONStr)", "KarData deserialization delegation")

    project_serializer_cpp = (root / "ProjectSerializer.cpp").read_text()
    for key in [
        '"version"',
        '"title"',
        '"artist"',
        '"origVideoFile"',
        '"timedLyrics"',
        '"parts"',
    ]:
        require(project_serializer_cpp, key, "ProjectSerializer JSON contract")
    require(project_serializer_cpp, "if (data.year < 0) data.year = 0", "ProjectSerializer year normalization")

    project_serializer_h = (root / "ProjectSerializer.h").read_text()
    require(project_serializer_h, "FormatVersion()", "ProjectSerializer format-version contract")
    require(project_serializer_h, "SupportsVersion", "ProjectSerializer version-support contract")

    ffmpeg_h = (root / "FfmpegCommandBuilder.h").read_text()
    require(ffmpeg_h, "AppIdentity::ProjectAttachmentMetadata()", "project attachment contract")
    require(ffmpeg_h, "AppIdentity::ProductName()", "project metadata contract")
    require(ffmpeg_h, "Vector<String>", "ffmpeg argument-vector contract")
    legacy_ext = ".mu" + "se"
    legacy_name = "Mu" + "se"
    reject(ffmpeg_h, "filename=" + legacy_ext[1:] + ".info", "ffmpeg metadata contract")

    ffmpeg_progress_parser_h = (root / "FfmpegProgressParser.h").read_text()
    require(ffmpeg_progress_parser_h, "ParseTimestamp", "ffmpeg progress parser contract")

    ffmpeg_progress_parser_cpp = (root / "FfmpegProgressParser.cpp").read_text()
    require(ffmpeg_progress_parser_cpp, "output.Find(key)", "ffmpeg progress key lookup")
    require(ffmpeg_progress_parser_cpp, "Split(tsStr, ':')", "ffmpeg progress timestamp split")
    require(ffmpeg_progress_parser_cpp, "formatted = output.Mid", "ffmpeg progress formatted timestamp")

    util_cpp = (root / "Util.cpp").read_text()
    require(util_cpp, "return FfmpegProgressParser::ParseTimestamp", "ComputeFfmpegTs compatibility wrapper")

    genre_catalog_h = (root / "GenreCatalog.h").read_text()
    require(genre_catalog_h, "List()", "GenreCatalog list contract")

    genre_catalog_cpp = (root / "GenreCatalog.cpp").read_text()
    for genre in ["Ballad", "OPM", "Rock n Roll", "Soft Rock"]:
        require(genre_catalog_cpp, f'"{genre}"', "GenreCatalog reference data")

    util_cpp = (root / "Util.cpp").read_text()
    require(util_cpp, "return GenreCatalog::List", "GetGenres compatibility wrapper")

    for rel in [
        "Page3.cpp",
        "Project.cpp",
        "ProjectList.cpp",
        "SaveProjectDlg.cpp",
    ]:
        text = (root / rel).read_text()
        require(text, "AppIdentity::", f"{rel} project extension contract")
        reject(text, legacy_ext, f"{rel} project extension contract")

    for rel in [
        "ConvertDlg.cpp",
        "ExportDlg.cpp",
        "OpenProjectDlg.cpp",
        "ProjectLoader.cpp",
        "SaveProjectDlg.cpp",
    ]:
        text = (root / rel).read_text()
        require(text, "AppIdentity::", f"{rel} temp-prefix contract")
        reject(text, f"GetTempFileName(\"{legacy_name}_", f"{rel} temp-prefix contract")


if __name__ == "__main__":
    main()
