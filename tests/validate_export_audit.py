#!/usr/bin/env python3
import sys
from pathlib import Path


def fail(message: str) -> None:
    print(f"validate_export_audit: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if len(sys.argv) != 2:
        fail("expected repository root argument")

    root = Path(sys.argv[1])
    ffmpeg_export_h = (root / "FfmpegExportCommandBuilder.h").read_text()
    ffmpeg_export_cpp = (root / "FfmpegExportCommandBuilder.cpp").read_text()
    ffmpeg_project_h = (root / "FfmpegProjectCommandBuilder.h").read_text()
    ffmpeg_project_cpp = (root / "FfmpegProjectCommandBuilder.cpp").read_text()
    if (root / "FfmpegCommandBuilder.h").exists():
        fail("obsolete FfmpegCommandBuilder compatibility facade still exists")
    for needle in [
        "AppIdentity::ProjectAttachmentMetadata()",
        "AppIdentity::ProductName()",
    ]:
        if needle not in ffmpeg_project_cpp:
            fail(f"FfmpegProjectCommandBuilder.cpp missing {needle}")
    for needle in [
        "WithBackgroundVideo",
        "WithVisualization",
    ]:
        if needle not in ffmpeg_export_h:
            fail(f"FfmpegExportCommandBuilder.h missing {needle}")
    for needle in [
        '"libx264"',
        "subtitles=%s[v]",
    ]:
        if needle not in ffmpeg_export_cpp:
            fail(f"FfmpegExportCommandBuilder.cpp missing {needle}")

    export_cpp = (root / "ExportDlg.cpp").read_text()
    for needle in [
        'AppIdentity::TempFileName(".ass")',
        "SubtitleGenerator::ToAss(*data, 4)",
        "FfmpegAudioCommandBuilder::Dehiss",
        "FfmpegExportCommandBuilder::WithVisualization",
        "FfmpegExportCommandBuilder::WithBackgroundVideo",
        "FfmpegExportCommandBuilder::GenerateCoverImage",
    ]:
        if needle not in export_cpp:
            fail(f"ExportDlg.cpp missing {needle}")
    if "FfmpegCommandBuilder::" in export_cpp:
        fail("ExportDlg.cpp still calls broad ffmpeg command builder")

    if (root / "Util.cpp").exists() or (root / "Util.h").exists():
        fail("Util compatibility facade should not exist")

    subtitle_generator_cpp = (root / "SubtitleGenerator.cpp").read_text()
    for needle in [
        "[Script Info]",
        "[V4+ Styles]",
        "[Events]",
        "Dialogue: 0",
        "CountInV1",
        "Grayed",
        "SubtitleMoveTag",
        "\\move(",
    ]:
        if needle not in subtitle_generator_cpp:
            fail(f"SubtitleGenerator.cpp missing ASS generation token {needle}")

    legacy_product = "Mu" + "se"
    legacy_lower = "mu" + "se"
    disallowed = [legacy_product, legacy_lower, legacy_product.upper(), f".{legacy_lower}"]
    ignored_dirs = {".git", "build"}
    for path in root.rglob("*"):
        if any(part in ignored_dirs for part in path.parts):
            continue
        if not path.is_file():
            continue
        try:
            text = path.read_text()
        except UnicodeDecodeError:
            continue
        for term in disallowed:
            if term in text:
                fail(f"{path.relative_to(root)} contains legacy product reference {term!r}")


if __name__ == "__main__":
    main()
