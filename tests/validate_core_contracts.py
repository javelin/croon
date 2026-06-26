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

    util_cpp = (root / "Util.cpp").read_text()
    require(util_cpp, "AppIdentity::PosixDataDirectory", "POSIX app data contract")
    require(util_cpp, "AppIdentity::DataDirectory", "non-POSIX app data contract")

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

    subtitle_generator_h = (root / "SubtitleGenerator.h").read_text()
    require(subtitle_generator_h, "ToAss", "SubtitleGenerator ASS contract")
    require(subtitle_generator_h, "ToRichAss", "SubtitleGenerator rich ASS contract")

    util_cpp = (root / "Util.cpp").read_text()
    require(util_cpp, "String SubtitleGenerator::ToAss", "SubtitleGenerator ASS implementation")
    require(util_cpp, "String SubtitleGenerator::ToRichAss", "SubtitleGenerator rich ASS implementation")
    require(util_cpp, "return SubtitleGenerator::ToAss", "TimedToASS compatibility wrapper")
    require(util_cpp, "return SubtitleGenerator::ToRichAss", "TimedToRichASS compatibility wrapper")

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
