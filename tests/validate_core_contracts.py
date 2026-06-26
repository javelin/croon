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

    kar_data_cpp = (root / "KarData.cpp").read_text()
    for key in [
        '"version"',
        '"title"',
        '"artist"',
        '"origVideoFile"',
        '"timedLyrics"',
        '"parts"',
    ]:
        require(kar_data_cpp, key, "KarData JSON contract")

    ffmpeg_h = (root / "Ffmpeg.h").read_text()
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
