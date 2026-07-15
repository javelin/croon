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
        "Croon.cpp",
        "Croon.iml",
        "AppIdentity.h",
        "AppPaths.cpp",
        "AppPaths.h",
        "AppAudioPlayer.cpp",
        "AppAudioPlayer.h",
        "AzLyricsProvider.cpp",
        "AzLyricsProvider.h",
        "GeniusLyricsProvider.cpp",
        "GeniusLyricsProvider.h",
        "ConfigService.cpp",
        "ConfigService.h",
        "DownloadDefaults.h",
        "ProjectSerializer.cpp",
        "ProjectSerializer.h",
        "FfmpegAudioCommandBuilder.h",
        "FfmpegExportCommandBuilder.h",
        "FfmpegProgressParser.cpp",
        "FfmpegProgressParser.h",
        "FfmpegProjectCommandBuilder.h",
        "FfmpegThumbnailCommandBuilder.h",
        "GenreCatalog.cpp",
        "GenreCatalog.h",
        "LyricsDownloadService.cpp",
        "LyricsDownloadService.h",
        "LyricsProviderTools.cpp",
        "LyricsProviderTools.h",
        "LyricsTransformer.cpp",
        "LyricsTransformer.h",
        "SongLyricsProvider.cpp",
        "SongLyricsProvider.h",
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
        "VocalPart.cpp",
        "VocalPart.h",
        "VideoCatalog.cpp",
        "VideoCatalog.h",
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
        "Croon.iml" not in upp or
        "AppIdentity.cpp" not in upp or
        "AppIdentity.h" not in upp or
        "AppPaths.cpp" not in upp or
        "AppPaths.h" not in upp or
        "AppAudioPlayer.cpp" not in upp or
        "AppAudioPlayer.h" not in upp or
        "AzLyricsProvider.cpp" not in upp or
        "AzLyricsProvider.h" not in upp or
        "GeniusLyricsProvider.cpp" not in upp or
        "GeniusLyricsProvider.h" not in upp or
        "ConfigService.cpp" not in upp or
        "ConfigService.h" not in upp or
        "DownloadDefaults.cpp" not in upp or
        "DownloadDefaults.h" not in upp or
        "ProjectSerializer.cpp" not in upp or
        "ProjectSerializer.h" not in upp or
        "FfmpegAudioCommandBuilder.cpp" not in upp or
        "FfmpegAudioCommandBuilder.h" not in upp or
        "FfmpegExportCommandBuilder.cpp" not in upp or
        "FfmpegExportCommandBuilder.h" not in upp or
        "FfmpegProgressParser.cpp" not in upp or
        "FfmpegProgressParser.h" not in upp or
        "FfmpegProjectCommandBuilder.cpp" not in upp or
        "FfmpegProjectCommandBuilder.h" not in upp or
        "FfmpegThumbnailCommandBuilder.cpp" not in upp or
        "FfmpegThumbnailCommandBuilder.h" not in upp or
        "GenreCatalog.cpp" not in upp or
        "GenreCatalog.h" not in upp or
        "LyricsDownloadService.cpp" not in upp or
        "LyricsDownloadService.h" not in upp or
        "LyricsProviderTools.cpp" not in upp or
        "LyricsProviderTools.h" not in upp or
        "LrcGenerator.cpp" not in upp or
        "LrcGenerator.h" not in upp or
        "LyricsTransformer.cpp" not in upp or
        "LyricsTransformer.h" not in upp or
        "SongLyricsProvider.cpp" not in upp or
        "SongLyricsProvider.h" not in upp or
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
        "VocalPart.cpp" not in upp or
        "VocalPart.h" not in upp or
        "VideoCatalog.cpp" not in upp or
        "VideoCatalog.h" not in upp or
        "Visualization.cpp" not in upp or
        "Visualization.h" not in upp or
        "MediaProcessRunner.cpp" not in upp or
        "MediaProcessRunner.h" not in upp
    ):
        fail("Croon.upp does not list package-level Croon files")
    if "Croon.h" in upp:
        fail("Croon.upp still lists obsolete Croon.h umbrella header")
    legacy_product = "Mu" + "se"
    if f"{legacy_product}." in upp or f"{legacy_product}Img" in upp:
        fail("Croon.upp still contains legacy package references")
    if 'library(POSIX) "SDL2_mixer";' not in upp:
        fail("Croon.upp should link SDL2_mixer explicitly on POSIX")
    if "pkg_config\n\tsdl2;" not in upp:
        fail("Croon.upp should resolve SDL2 through pkg-config")
    if 'library(POSIX) "SDL2 SDL2_mixer";' in upp:
        fail("Croon.upp links SDL2 twice on POSIX")
    if "\tFfmpeg.h," in upp:
        fail("Croon.upp should not list obsolete Ffmpeg.h alias")
    if "\tFfmpegCommandBuilder.h," in upp:
        fail("Croon.upp should not list obsolete FfmpegCommandBuilder facade")
    if "\tMusicPlayer.h," in upp:
        fail("Croon.upp should not list obsolete MusicPlayer facade")
    if "\tAudioPlayer.h," in upp:
        fail("Croon.upp should not list obsolete generic AudioPlayer wrapper")
    if "\tAudioPlayerBase.h," in upp:
        fail("Croon.upp should not list obsolete AudioPlayerBase wrapper")

    if (root / "Croon.h").exists():
        fail("obsolete Croon.h umbrella header still exists")
    if (root / "FfmpegCommandBuilder.h").exists():
        fail("obsolete FfmpegCommandBuilder compatibility facade still exists")
    if (root / "MusicPlayer.h").exists():
        fail("obsolete MusicPlayer compatibility facade still exists")
    if (root / "AudioPlayer.h").exists():
        fail("obsolete generic AudioPlayer wrapper still exists")
    if (root / "AudioPlayerBase.h").exists():
        fail("obsolete AudioPlayerBase wrapper still exists")

    croon_cpp = (root / "Croon.cpp").read_text()
    if "CroonImg" not in croon_cpp:
        fail("Croon image class is not declared")
    if "<Croon/Croon.iml>" not in croon_cpp:
        fail("Croon image file is not declared")
    if "void RunCroon()" not in croon_cpp:
        fail("RunCroon entry point is not defined")

    main_cpp = (root / "main.cpp").read_text()
    if "RunCroon();" not in main_cpp:
        fail("main.cpp does not call RunCroon")

    contracts = (root / "contracts.md").read_text()
    for expected in [".croon", "croon.info", "Croon_", "version", "ProjectSerializer"]:
        if expected not in contracts:
            fail(f"contracts.md missing {expected}")

    identity = (root / "AppIdentity.cpp").read_text()
    for expected in [".croon", "croon.info", "Croon_", "Croon", "1.1"]:
        if expected not in identity:
            fail(f"AppIdentity.cpp missing {expected}")


if __name__ == "__main__":
    main()
