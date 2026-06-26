#!/usr/bin/env python3
import sys
from pathlib import Path


def fail(message: str) -> None:
    print(f"validate_identity: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if len(sys.argv) != 2:
        fail("expected repository root argument")

    root = Path(sys.argv[1])
    required = [
        "Croon.upp",
        "Croon.h",
        "Croon.cpp",
        "Croon.iml",
        "AppIdentity.h",
        "AppPaths.cpp",
        "AppPaths.h",
        "ConfigService.cpp",
        "ConfigService.h",
        "DownloadDefaults.h",
        "ProjectSerializer.cpp",
        "ProjectSerializer.h",
        "FfmpegProgressParser.cpp",
        "FfmpegProgressParser.h",
        "GenreCatalog.cpp",
        "GenreCatalog.h",
        "LyricsDownloadService.cpp",
        "LyricsDownloadService.h",
        "LyricsTransformer.cpp",
        "LyricsTransformer.h",
        "RecentProjectService.cpp",
        "RecentProjectService.h",
        "RichTextBuilder.h",
        "SubtitleLineProcessor.cpp",
        "SubtitleLineProcessor.h",
        "SubtitleGenerator.cpp",
        "SubtitleGenerator.h",
        "TextTools.cpp",
        "TextTools.h",
        "TimeFormatter.cpp",
        "TimeFormatter.h",
        "UiScaler.cpp",
        "UiScaler.h",
        "MediaProcessRunner.h",
        "main.cpp",
        "architecture.md",
        "decisions.md",
        "contracts.md",
        "services.md",
        "infra.md",
    ]
    for rel in required:
        if not (root / rel).exists():
            fail(f"missing {rel}")

    upp = (root / "Croon.upp").read_text()
    if (
        "Croon.h" not in upp or
        "Croon.iml" not in upp or
        "AppIdentity.h" not in upp or
        "AppPaths.cpp" not in upp or
        "AppPaths.h" not in upp or
        "ConfigService.cpp" not in upp or
        "ConfigService.h" not in upp or
        "DownloadDefaults.h" not in upp or
        "ProjectSerializer.cpp" not in upp or
        "ProjectSerializer.h" not in upp or
        "FfmpegProgressParser.cpp" not in upp or
        "FfmpegProgressParser.h" not in upp or
        "GenreCatalog.cpp" not in upp or
        "GenreCatalog.h" not in upp or
        "LyricsDownloadService.cpp" not in upp or
        "LyricsDownloadService.h" not in upp or
        "LyricsTransformer.cpp" not in upp or
        "LyricsTransformer.h" not in upp or
        "RecentProjectService.cpp" not in upp or
        "RecentProjectService.h" not in upp or
        "RichTextBuilder.h" not in upp or
        "SubtitleLineProcessor.cpp" not in upp or
        "SubtitleLineProcessor.h" not in upp or
        "SubtitleGenerator.cpp" not in upp or
        "SubtitleGenerator.h" not in upp or
        "TextTools.cpp" not in upp or
        "TextTools.h" not in upp or
        "TimeFormatter.cpp" not in upp or
        "TimeFormatter.h" not in upp or
        "UiScaler.cpp" not in upp or
        "UiScaler.h" not in upp or
        "MediaProcessRunner.h" not in upp
    ):
        fail("Croon.upp does not list package-level Croon files")
    legacy_product = "Mu" + "se"
    if f"{legacy_product}." in upp or f"{legacy_product}Img" in upp:
        fail("Croon.upp still contains legacy package references")
    if 'library(POSIX) "SDL2_mixer";' not in upp:
        fail("Croon.upp should link SDL2_mixer explicitly on POSIX")
    if "pkg_config\n\tsdl2;" not in upp:
        fail("Croon.upp should resolve SDL2 through pkg-config")
    if 'library(POSIX) "SDL2 SDL2_mixer";' in upp:
        fail("Croon.upp links SDL2 twice on POSIX")

    header = (root / "Croon.h").read_text()
    if '#include "AppIdentity.h"' not in header:
        fail("Croon.h does not include AppIdentity.h")
    if '#include "AppPaths.h"' not in header:
        fail("Croon.h does not include AppPaths.h")
    if '#include "ConfigService.h"' not in header:
        fail("Croon.h does not include ConfigService.h")
    if '#include "DownloadDefaults.h"' not in header:
        fail("Croon.h does not include DownloadDefaults.h")
    if '#include "ProjectSerializer.h"' not in header:
        fail("Croon.h does not include ProjectSerializer.h")
    if '#include "FfmpegProgressParser.h"' not in header:
        fail("Croon.h does not include FfmpegProgressParser.h")
    if '#include "GenreCatalog.h"' not in header:
        fail("Croon.h does not include GenreCatalog.h")
    if '#include "LyricsDownloadService.h"' not in header:
        fail("Croon.h does not include LyricsDownloadService.h")
    if '#include "LyricsTransformer.h"' not in header:
        fail("Croon.h does not include LyricsTransformer.h")
    if '#include "RichTextBuilder.h"' not in header:
        fail("Croon.h does not include RichTextBuilder.h")
    if '#include "SubtitleLineProcessor.h"' not in header:
        fail("Croon.h does not include SubtitleLineProcessor.h")
    if '#include "RecentProjectService.h"' not in header:
        fail("Croon.h does not include RecentProjectService.h")
    if '#include "SubtitleGenerator.h"' not in header:
        fail("Croon.h does not include SubtitleGenerator.h")
    if '#include "TextTools.h"' not in header:
        fail("Croon.h does not include TextTools.h")
    if '#include "TimeFormatter.h"' not in header:
        fail("Croon.h does not include TimeFormatter.h")
    if '#include "UiScaler.h"' not in header:
        fail("Croon.h does not include UiScaler.h")
    if '#include "MediaProcessRunner.h"' not in header:
        fail("Croon.h does not include MediaProcessRunner.h")
    if "CroonImg" not in header:
        fail("Croon image class is not declared")
    if "<Croon/Croon.iml>" not in header:
        fail("Croon image file is not declared")
    if "RunCroon" not in header:
        fail("RunCroon entry point is not declared")

    main_cpp = (root / "main.cpp").read_text()
    if "RunCroon();" not in main_cpp:
        fail("main.cpp does not call RunCroon")

    contracts = (root / "contracts.md").read_text()
    for expected in [".croon", "croon.info", "Croon_", "version", "ProjectSerializer"]:
        if expected not in contracts:
            fail(f"contracts.md missing {expected}")

    identity = (root / "AppIdentity.h").read_text()
    for expected in [".croon", "croon.info", "Croon_", "Croon", "1.0"]:
        if expected not in identity:
            fail(f"AppIdentity.h missing {expected}")


if __name__ == "__main__":
    main()
