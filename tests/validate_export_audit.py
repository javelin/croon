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
    ffmpeg_h = (root / "Ffmpeg.h").read_text()
    for needle in [
        "ExportWithBackgroundVideo",
        "ExportWithVisualization",
        "AppIdentity::ProjectAttachmentMetadata()",
        "AppIdentity::ProductName()",
        '"libx264"',
        "subtitles=%s[v]",
    ]:
        if needle not in ffmpeg_h:
            fail(f"Ffmpeg.h missing {needle}")

    export_cpp = (root / "ExportDlg.cpp").read_text()
    for needle in [
        'AppIdentity::TempFileName(".ass")',
        "TimedToASS(*data, 4)",
        "Ffmpeg::ExportWithVisualization",
        "Ffmpeg::ExportWithBackgroundVideo",
        "Ffmpeg::GenerateCoverImage",
    ]:
        if needle not in export_cpp:
            fail(f"ExportDlg.cpp missing {needle}")

    util_cpp = (root / "Util.cpp").read_text()
    for needle in [
        "String TimedToASS(",
        "[Script Info]",
        "[V4+ Styles]",
        "[Events]",
        "Dialogue: 0",
        "CountInV1",
        "Grayed",
    ]:
        if needle not in util_cpp:
            fail(f"Util.cpp missing ASS generation token {needle}")

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
