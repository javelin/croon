#!/usr/bin/env python3
import re
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


def validate_no_production_utility_wrapper_calls(root: Path) -> None:
    wrappers = [
        "CleanSpacing",
        "ComputeFfmpegTs",
        "DownloadLyrics",
        "FormatTime",
        "FormatTime2",
        "FormatTimeASS",
        "FormatTimeSRT",
        "GetDataDirectory",
        "GetGenres",
        "GetPaths",
        "LookaheadVocalPart",
        "RawToUntimedLyrics",
        "ReplaceMetadata",
        "ResolveCountInStyle",
        "ResolveDimStyle",
        "ResolveStyle",
        "ResolveVocalPart",
        "ShortenMiddle",
        "SplitLyrics",
        "StripNonAlnum",
        "TimedLyricsToRaw",
        "TimedToASS",
        "TimedToRichASS",
        "_Zx",
        "_Zy",
    ]
    pattern = re.compile(r"(?<![:A-Za-z0-9_])(" + "|".join(map(re.escape, wrappers)) + r")\s*\(")
    allowed = {
        "SubtitleLineProcessor.cpp",
        "SubtitleLineProcessor.h",
        "TextTools.h",
        "Util.cpp",
        "Util.h",
    }
    for path in sorted(root.glob("*")):
        if path.suffix not in {".cpp", ".h"} or path.name in allowed:
            continue
        text = path.read_text()
        for match in pattern.finditer(text):
            line = text.count("\n", 0, match.start()) + 1
            fail(f"{path.name}:{line} calls compatibility wrapper {match.group(1)}")


def main() -> None:
    if len(sys.argv) != 2:
        fail("expected repository root argument")

    root = Path(sys.argv[1])
    validate_no_production_utility_wrapper_calls(root)

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
    reject(app_paths_cpp, '#include "Croon.h"', "AppPaths app shell dependency")
    require(app_paths_cpp, '#include "AppIdentity.h"', "AppPaths direct identity dependency")
    require(app_paths_cpp, '#include "AppPaths.h"', "AppPaths direct self dependency")
    require(app_paths_cpp, "AppIdentity::PosixDataDirectory", "POSIX app data contract")
    require(app_paths_cpp, "AppIdentity::DataDirectory", "non-POSIX app data contract")
    require(app_paths_cpp, "DirectoryCreate(dataDir)", "AppPaths data directory creation")
    require(app_paths_cpp, "PatternMatchMulti(pattern, ff.GetName())", "AppPaths file discovery")

    config_h = (root / "Config.h").read_text()
    require(config_h, "ConfigService::Get(key, defaultValue)", "Config get delegation")
    require(config_h, "ConfigService::Set(key, value)", "Config set delegation")
    require(config_h, "ConfigService::GetInt(key, defaultValue)", "Config int get delegation")
    require(config_h, "ConfigService::GetFontSize()", "Config font-size delegation")

    config_cpp = (root / "Config.cpp").read_text()
    reject(config_cpp, '#include "Croon.h"', "Config app shell dependency")
    require(config_cpp, '#include "ConfigService.h"\n#include "Config.h"', "Config ordered direct dependencies")
    require(config_cpp, "Config Config::config", "Config singleton storage")
    require(config_cpp, "ConfigService::DefaultFontSize", "Config default font-size delegation")

    config_service_cpp = (root / "ConfigService.cpp").read_text()
    reject(config_service_cpp, '#include "Croon.h"', "ConfigService app shell dependency")
    require(config_service_cpp, '#include "AppPaths.h"', "ConfigService direct app paths dependency")
    require(config_service_cpp, '#include "ConfigService.h"\n#include "Config.h"', "ConfigService ordered config dependencies")
    require(config_service_cpp, "#include <algorithm>", "ConfigService direct clamp dependency")
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

    audio_player_h = (root / "AudioPlayer.h").read_text()
    require(audio_player_h, "bool Reopen() override { return player.Reopen(); }", "AudioPlayer reopen delegation")
    audio_player_base_h = (root / "AudioPlayerBase.h").read_text()
    reject(audio_player_base_h, "AudioPlayerEvent", "AudioPlayerBase dead event wrapper")
    reject(audio_player_base_h, "APE_Type_", "AudioPlayerBase dead event type")
    reject(audio_player_base_h, "bool opened", "AudioPlayerBase dead opened flag")
    reject(audio_player_base_h, "bool playing", "AudioPlayerBase dead playing flag")
    sdl_mixer_audio_player_h = (root / "SDLMixerAudioPlayer.h").read_text()
    sdl_mixer_audio_player_cpp = (root / "SDLMixerAudioPlayer.cpp").read_text()
    reject(sdl_mixer_audio_player_h, "initialized", "SDLMixerAudioPlayer dead initialized flag")
    reject(sdl_mixer_audio_player_h, "GetPosition", "SDLMixerAudioPlayer dead position declaration")
    reject(sdl_mixer_audio_player_cpp, "initialized", "SDLMixerAudioPlayer dead initialized storage")

    croon_h = (root / "Croon.h").read_text()
    reject(croon_h, "typedef struct Visualization VIZ", "Croon.h visualization alias")
    page3_cpp = (root / "Page3.cpp").read_text()
    open_project_dlg_cpp = (root / "OpenProjectDlg.cpp").read_text()
    reject(page3_cpp, "VIZ::", "Page3 visualization alias dependency")
    reject(open_project_dlg_cpp, "VIZ::", "OpenProjectDlg visualization alias dependency")

    recent_project_service_cpp = (root / "RecentProjectService.cpp").read_text()
    reject(recent_project_service_cpp, '#include "Croon.h"', "RecentProjectService app shell dependency")
    require(recent_project_service_cpp, '#include "RecentProjectService.h"', "RecentProjectService direct self dependency")
    require(recent_project_service_cpp, '#include "ConfigService.h"\n#include "Config.h"', "RecentProjectService ordered config dependencies")
    require(recent_project_service_cpp, "ConfigService::Get(PROJECT_LIST)", "RecentProjectService load contract")
    require(recent_project_service_cpp, "ConfigService::Set(PROJECT_LIST", "RecentProjectService save contract")
    require(recent_project_service_cpp, "TrimBoth(path)", "RecentProjectService path normalization")
    require(recent_project_service_cpp, "FindPathIndex(normalized, trimmed)", "RecentProjectService de-duplication")

    project_loader_cpp = (root / "ProjectLoader.cpp").read_text()
    reject(project_loader_cpp, '#include "Croon.h"', "ProjectLoader app shell dependency")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#define IMAGECLASS CroonImg",
        "#define IMAGEFILE <Croon/Croon.iml>",
        "#include <Draw/iml_header.h>",
        '#include "AppIdentity.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "KarData.h"\n#include "Visualization.h"\n#include "FfmpegCommandBuilder.h"',
        '#include "LyricsTransformer.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ProjectLoader.h"',
    ]:
        require(project_loader_cpp, needle, "ProjectLoader direct dependency")
    require(project_loader_cpp, "RecentProjectService::LoadPaths()", "ProjectLoader recent-project load contract")
    require(project_loader_cpp, "AppIdentity::TempFileName", "ProjectLoader temp-file identity contract")
    require(project_loader_cpp, "FfmpegCommandBuilder::DumpAttachmentAndGenerateThumbnail", "ProjectLoader ffmpeg command contract")
    require(project_loader_cpp, "KarData data{LoadFile(infoFilePath)}", "ProjectLoader metadata load contract")
    require(project_loader_cpp, "LyricsTransformer::TimedToRaw", "ProjectLoader lyric preview contract")
    require(project_loader_cpp, "process.Start(ffmpeg", "ProjectLoader media process boundary")

    lyrics_transformer_h = (root / "LyricsTransformer.h").read_text()
    require(lyrics_transformer_h, "SplitDecorations", "LyricsTransformer decoration contract")
    require(lyrics_transformer_h, "RawToUntimed", "LyricsTransformer raw-to-timed contract")
    require(lyrics_transformer_h, "TimedToRaw", "LyricsTransformer timed-to-raw contract")

    lyrics_transformer_cpp = (root / "LyricsTransformer.cpp").read_text()
    reject(lyrics_transformer_cpp, '#include "Croon.h"', "LyricsTransformer app shell dependency")
    require(lyrics_transformer_cpp, "#include <Draw/Draw.h>", "LyricsTransformer direct image type dependency")
    require(lyrics_transformer_cpp, '#include "LyricsTransformer.h"', "LyricsTransformer direct self dependency")
    require(lyrics_transformer_cpp, '#include "KarData.h"', "LyricsTransformer direct data dependency")
    require(lyrics_transformer_cpp, "MaxLineLength", "LyricsTransformer wrapping contract")
    require(lyrics_transformer_cpp, "removeMetadata", "LyricsTransformer metadata removal contract")

    lyrics_download_service_h = (root / "LyricsDownloadService.h").read_text()
    require(lyrics_download_service_h, "BuildAzLyricsUrl", "LyricsDownloadService URL contract")
    require(lyrics_download_service_h, "ExtractAzLyrics", "LyricsDownloadService extraction contract")
    require(lyrics_download_service_h, "Download", "LyricsDownloadService download contract")
    require(lyrics_download_service_h, "AzLyricsUrlFormat", "LyricsDownloadService provider URL boundary")
    require(lyrics_download_service_h, "AzLyricsPattern", "LyricsDownloadService provider pattern boundary")

    lyrics_download_service_cpp = (root / "LyricsDownloadService.cpp").read_text()
    require(lyrics_download_service_cpp, "https://www.azlyrics.com/lyrics/%s/%s.html", "LyricsDownloadService provider URL")
    require(lyrics_download_service_cpp, "<!-- Usage of azlyrics.com content", "LyricsDownloadService extraction pattern")
    require(lyrics_download_service_cpp, "DownloadDlg dlg", "LyricsDownloadService UI download workflow")

    download_defaults_h = (root / "DownloadDefaults.h").read_text()
    require(download_defaults_h, "struct DownloadDefaults", "DownloadDefaults boundary")
    require(download_defaults_h, "UserAgent()", "DownloadDefaults user-agent contract")
    require(download_defaults_h, "Chrome/60.0.3112.90", "DownloadDefaults legacy user-agent value")

    download_dlg_h = (root / "DownloadDlg.h").read_text()
    require(download_dlg_h, "DownloadDefaults::UserAgent()", "DownloadDlg user-agent default")

    constants_h = (root / "Constants.h").read_text()
    reject(constants_h, "AZ_URL", "Constants provider URL extraction")
    reject(constants_h, "AZ_PATTERN", "Constants provider pattern extraction")
    reject(constants_h, "USER_AGENT", "Constants user-agent extraction")
    reject(constants_h, "MaxASSDisplayLines", "Constants dead ASS max cleanup")
    reject(constants_h, "MinASSDisplayLines", "Constants dead ASS min cleanup")

    services_md = (root / "services.md").read_text()
    require(services_md, "## Compatibility Facade", "services compatibility documentation")
    require(services_md, "`Util`: legacy compatibility facade", "Util compatibility documentation")

    croon_h = (root / "Croon.h").read_text()
    reject(croon_h, '#include "Util.h"', "Croon.h compatibility facade exposure")

    subtitle_line_processor_h = (root / "SubtitleLineProcessor.h").read_text()
    require(subtitle_line_processor_h, "ProcessMetadata", "SubtitleLineProcessor metadata contract")
    require(subtitle_line_processor_h, "ResolveVocalPart", "SubtitleLineProcessor vocal-part contract")
    require(subtitle_line_processor_h, "ResolveStyle", "SubtitleLineProcessor style contract")

    subtitle_line_processor_cpp = (root / "SubtitleLineProcessor.cpp").read_text()
    reject(subtitle_line_processor_cpp, '#include "Croon.h"', "SubtitleLineProcessor app shell dependency")
    require(subtitle_line_processor_cpp, "#include <Draw/Draw.h>", "SubtitleLineProcessor direct image type dependency")
    require(subtitle_line_processor_cpp, '#include "SubtitleLineProcessor.h"', "SubtitleLineProcessor direct self dependency")
    require(subtitle_line_processor_cpp, '#include "KarData.h"', "SubtitleLineProcessor direct data dependency")
    require(subtitle_line_processor_cpp, '#include "TimeFormatter.h"', "SubtitleLineProcessor direct time formatter dependency")
    require(subtitle_line_processor_cpp, "TimeFormatter::CountInDuration", "SubtitleLineProcessor count-in contract")
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
    reject(subtitle_generator_cpp, '#include "Croon.h"', "SubtitleGenerator app shell dependency")
    require(subtitle_generator_cpp, '#include "SubtitleGenerator.h"', "SubtitleGenerator direct self dependency")
    require(subtitle_generator_cpp, '#include "SubtitleLineProcessor.h"', "SubtitleGenerator direct line processor dependency")
    require(subtitle_generator_cpp, '#include "TimeFormatter.h"', "SubtitleGenerator direct time formatter dependency")
    require(subtitle_generator_cpp, "SubtitleLineProcessor::ProcessMetadata", "SubtitleGenerator direct metadata dependency")
    require(subtitle_generator_cpp, "SubtitleLineProcessor::ResolveStyle", "SubtitleGenerator direct style dependency")
    require(subtitle_generator_cpp, "TimeFormatter::Ass", "SubtitleGenerator direct ASS time formatting")
    reject(subtitle_generator_cpp, "    ProcessMetadata(data", "SubtitleGenerator utility metadata wrapper dependency")
    reject(subtitle_generator_cpp, "String hilite = ResolveStyle", "SubtitleGenerator utility style wrapper dependency")
    reject(subtitle_generator_cpp, "FormatTimeASS", "SubtitleGenerator utility time wrapper dependency")

    rich_text_builder_h = (root / "RichTextBuilder.h").read_text()
    require(rich_text_builder_h, "struct RTHelper", "RichTextBuilder compatibility type")
    require(rich_text_builder_h, "Fmt(String s)", "RichTextBuilder QTF format contract")
    require(rich_text_builder_h, "EFmt(String s)", "RichTextBuilder QTF format switch contract")
    require(rich_text_builder_h, "DeQtf(s)", "RichTextBuilder text escaping contract")
    require(rich_text_builder_h, "Join(vs", "RichTextBuilder output contract")

    text_tools_h = (root / "TextTools.h").read_text()
    for method in ["CleanSpacing", "StripNonAlnum", "ShortenMiddle"]:
        require(text_tools_h, method, "TextTools contract")

    text_tools_cpp = (root / "TextTools.cpp").read_text()
    reject(text_tools_cpp, '#include "Croon.h"', "TextTools app shell dependency")
    require(text_tools_cpp, '#include "TextTools.h"', "TextTools direct self dependency")
    require(text_tools_cpp, "TrimBoth(vw[i])", "TextTools spacing cleanup")
    require(text_tools_cpp, "IsAlNum(c)", "TextTools alphanumeric filter")
    require(text_tools_cpp, "\"...\"", "TextTools middle shortening")

    time_formatter_h = (root / "TimeFormatter.h").read_text()
    for method in ["CountInDuration", "Format", "Ass", "Srt", "Clock"]:
        require(time_formatter_h, method, "TimeFormatter contract")

    time_formatter_cpp = (root / "TimeFormatter.cpp").read_text()
    reject(time_formatter_cpp, '#include "Croon.h"', "TimeFormatter app shell dependency")
    require(time_formatter_cpp, '#include "TimeFormatter.h"', "TimeFormatter direct self dependency")
    require(time_formatter_cpp, "std::round(dur*100.0f)/100.0f", "TimeFormatter count-in rounding")
    require(time_formatter_cpp, "decimal", "TimeFormatter decimal separator contract")
    require(time_formatter_cpp, "Mid(1)", "TimeFormatter ASS timestamp contract")

    core_tests_cpp = (root / "tests" / "upp" / "CroonCoreTests" / "CroonCoreTests.cpp").read_text()
    core_tests_upp = (root / "tests" / "upp" / "CroonCoreTests" / "CroonCoreTests.upp").read_text()
    core_test_support_cpp = (root / "tests" / "upp" / "CroonCoreTests" / "CroonCoreTestSupport.cpp").read_text()
    require(core_tests_cpp, "TimeFormatter::CountInDuration", "core tests direct time formatter dependency")
    require(core_tests_cpp, "TimeFormatter::Clock", "core tests direct clock formatter dependency")
    require(core_tests_cpp, "TimeFormatter::Ass", "core tests direct ASS formatter dependency")
    require(core_tests_cpp, "TextTools::StripNonAlnum", "core tests direct text tools dependency")
    require(core_tests_cpp, "LyricsTransformer::SplitDecorations", "core tests direct lyric decoration splitter dependency")
    require(core_tests_cpp, "LyricsTransformer::RawToUntimed", "core tests direct raw lyric transformer dependency")
    require(core_tests_cpp, "LyricsTransformer::TimedToRaw", "core tests direct timed lyric transformer dependency")
    require(core_tests_cpp, "SubtitleLineProcessor::ReplaceMetadata", "core tests direct metadata replacement dependency")
    require(core_tests_cpp, "SubtitleLineProcessor::ResolveVocalPart", "core tests direct vocal part dependency")
    require(core_tests_cpp, "SubtitleLineProcessor::ResolveStyle", "core tests direct style dependency")
    require(core_tests_cpp, "SubtitleLineProcessor::ProcessMetadata", "core tests direct metadata processing dependency")
    require(core_tests_cpp, "SubtitleGenerator::ToAss", "core tests direct ASS generator dependency")
    require(core_tests_cpp, "SubtitleGenerator::ToRichAss", "core tests direct rich ASS generator dependency")
    require(core_tests_cpp, 'richAss.Find("@4")', "core tests rich ASS formatting assertion")
    require(core_tests_upp, "LyricsTransformer.cpp", "core tests lyrics transformer implementation")
    require(core_tests_upp, "SubtitleGenerator.cpp", "core tests subtitle generator implementation")
    require(core_tests_upp, "SubtitleLineProcessor.cpp", "core tests subtitle line processor implementation")
    reject(core_tests_upp, "CroonLyricsTestSupport.cpp", "core tests orphan lyric compatibility support")
    reject(core_tests_upp, "CroonAssTestSupport.cpp", "core tests orphan ASS compatibility support")
    reject(core_tests_cpp, "#include <Croon/Util.h>", "core tests compatibility facade include")
    for wrapper in ["Check(CountInDuration(", "Check(FormatTime2(", "Check(FormatTimeASS(", "Check(StripNonAlnum("]:
        reject(core_tests_cpp, wrapper, "core tests time/text compatibility wrapper dependency")
    for wrapper in ["SplitLyrics(", "RawToUntimedLyrics(", "TimedLyricsToRaw("]:
        reject(core_tests_cpp, wrapper, "core tests lyric compatibility wrapper dependency")
    for wrapper in ["TimedToASS(", "TimedToRichASS("]:
        reject(core_tests_cpp, wrapper, "core tests ASS compatibility wrapper dependency")
    for wrapper in ["ReplaceMetadata(", "ResolveVocalPart(", "ResolveCountInStyle(", "ResolveStyle(", "ProcessMetadata("]:
        reject(core_tests_cpp, "Check(" + wrapper, "core tests subtitle-line compatibility wrapper dependency")
        reject(core_tests_cpp, "\t" + wrapper, "core tests subtitle-line compatibility wrapper dependency")
    for rel, text in {
        "CroonCoreTestSupport.cpp": core_test_support_cpp,
    }.items():
        reject(text, "#include <Croon/Util.h>", f"{rel} compatibility facade include")

    ui_scaler_h = (root / "UiScaler.h").read_text()
    require(ui_scaler_h, "X(int value)", "UiScaler horizontal scale contract")
    require(ui_scaler_h, "Y(int value)", "UiScaler vertical scale contract")

    ui_scaler_cpp = (root / "UiScaler.cpp").read_text()
    reject(ui_scaler_cpp, '#include "Croon.h"', "UiScaler app shell dependency")
    require(ui_scaler_cpp, "#include <CtrlLib/CtrlLib.h>", "UiScaler direct CtrlLib dependency")
    require(ui_scaler_cpp, '#include "UiScaler.h"', "UiScaler direct self dependency")
    require(ui_scaler_cpp, "Ctrl::GetZoomRatio", "UiScaler U++ zoom-ratio contract")
    require(ui_scaler_cpp, "return value*dx/mx", "UiScaler horizontal calculation")
    require(ui_scaler_cpp, "return value*dy/my", "UiScaler vertical calculation")

    vid_thumbnail_cpp = (root / "VidThumbnail.cpp").read_text()
    reject(vid_thumbnail_cpp, '#include "Croon.h"', "VidThumbnail app shell dependency")
    require(vid_thumbnail_cpp, "#include <CtrlLib/CtrlLib.h>", "VidThumbnail direct CtrlLib dependency")
    require(vid_thumbnail_cpp, '#include "Constants.h"', "VidThumbnail direct thumbnail constants dependency")
    require(vid_thumbnail_cpp, '#include "VidThumbnail.h"', "VidThumbnail direct self dependency")
    require(vid_thumbnail_cpp, "const int VidThumbnail::Width = ThumbnailDim", "VidThumbnail width contract")
    require(vid_thumbnail_cpp, "imgCtrl.SetImage(Rescale(image, sz))", "VidThumbnail rescale contract")

    util_cpp = (root / "Util.cpp").read_text()
    reject(util_cpp, '#include "Croon.h"', "Util.cpp app shell dependency")
    require(util_cpp, '#include "Util.h"', "Util.cpp direct facade header dependency")
    require(util_cpp, "#include <RichText/RichText.h>", "Util.cpp direct rich text dependency")
    require(util_cpp, '#include "Constants.h"', "Util.cpp direct ASS defaults dependency")
    require(util_cpp, '#include "AppPaths.h"', "Util.cpp direct app paths dependency")
    require(util_cpp, '#include "LyricsTransformer.h"', "Util.cpp direct lyrics transformer dependency")
    require(util_cpp, '#include "SubtitleGenerator.h"', "Util.cpp direct subtitle generator dependency")
    require(util_cpp, '#include "SubtitleLineProcessor.h"', "Util.cpp direct subtitle line processor dependency")
    reject(util_cpp, "SRT_PATTERN", "Util.cpp dead SRT parser cleanup")
    require(util_cpp, "return AppPaths::DataDirectory", "GetDataDirectory compatibility wrapper")
    require(util_cpp, "return AppPaths::FindFiles", "GetPaths compatibility wrapper")
    require(util_cpp, "return LyricsDownloadService::Download", "DownloadLyrics compatibility wrapper")
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
    require(util_cpp, "return UiScaler::X", "_Zx compatibility wrapper")
    require(util_cpp, "return UiScaler::Y", "_Zy compatibility wrapper")

    util_h = (root / "Util.h").read_text()
    require(util_h, '#include "RichTextBuilder.h"', "Util.h rich text dependency")
    reject(util_h, "struct RTHelper", "Util.h rich text helper extraction")
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
    reject(kar_data_cpp, '#include "Croon.h"', "KarData app shell dependency")
    require(kar_data_cpp, '#include "AppIdentity.h"', "KarData direct identity dependency")
    require(kar_data_cpp, '#include "ConfigService.h"\n#include "Config.h"', "KarData ordered config dependencies")
    require(kar_data_cpp, '#include "KarData.h"', "KarData direct self dependency")
    require(kar_data_cpp, '#include "ProjectSerializer.h"', "KarData direct serializer dependency")
    require(kar_data_cpp, "ProjectSerializer::ToJson(*this)", "KarData serialization delegation")
    require(kar_data_cpp, "ProjectSerializer::FromJson(JSONStr)", "KarData deserialization delegation")

    project_serializer_cpp = (root / "ProjectSerializer.cpp").read_text()
    reject(project_serializer_cpp, '#include "Croon.h"', "ProjectSerializer app shell dependency")
    require(project_serializer_cpp, '#include "AppIdentity.h"', "ProjectSerializer direct identity dependency")
    require(project_serializer_cpp, '#include "KarData.h"', "ProjectSerializer direct data dependency")
    require(project_serializer_cpp, '#include "ProjectSerializer.h"', "ProjectSerializer direct self dependency")
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
    reject(ffmpeg_h, "VIZ::", "ffmpeg visualization alias dependency")
    require(ffmpeg_h, '#include "LyricsTransformer.h"', "ffmpeg lyrics transformer dependency")
    require(ffmpeg_h, '#include "TimeFormatter.h"', "ffmpeg time formatter dependency")
    require(ffmpeg_h, "AppIdentity::ProjectAttachmentMetadata()", "project attachment contract")
    require(ffmpeg_h, "AppIdentity::ProductName()", "project metadata contract")
    require(ffmpeg_h, "Vector<String>", "ffmpeg argument-vector contract")
    require(ffmpeg_h, "LyricsTransformer::TimedToRaw", "ffmpeg lyrics metadata serialization")
    require(ffmpeg_h, "TimeFormatter::Clock", "ffmpeg thumbnail timestamp formatting")
    reject(ffmpeg_h, "TimedLyricsToRaw", "ffmpeg utility lyrics wrapper dependency")
    reject(ffmpeg_h, "FormatTime2", "ffmpeg utility time wrapper dependency")
    legacy_ext = ".mu" + "se"
    legacy_name = "Mu" + "se"
    reject(ffmpeg_h, "filename=" + legacy_ext[1:] + ".info", "ffmpeg metadata contract")

    ffmpeg_progress_parser_h = (root / "FfmpegProgressParser.h").read_text()
    require(ffmpeg_progress_parser_h, "ParseTimestamp", "ffmpeg progress parser contract")

    ffmpeg_progress_parser_cpp = (root / "FfmpegProgressParser.cpp").read_text()
    reject(ffmpeg_progress_parser_cpp, '#include "Croon.h"', "ffmpeg progress parser app shell dependency")
    require(ffmpeg_progress_parser_cpp, '#include "FfmpegProgressParser.h"', "ffmpeg progress parser direct self dependency")
    require(ffmpeg_progress_parser_cpp, "output.Find(key)", "ffmpeg progress key lookup")
    require(ffmpeg_progress_parser_cpp, "Split(tsStr, ':')", "ffmpeg progress timestamp split")
    require(ffmpeg_progress_parser_cpp, "formatted = output.Mid", "ffmpeg progress formatted timestamp")

    util_cpp = (root / "Util.cpp").read_text()
    require(util_cpp, "return FfmpegProgressParser::ParseTimestamp", "ComputeFfmpegTs compatibility wrapper")

    genre_catalog_h = (root / "GenreCatalog.h").read_text()
    require(genre_catalog_h, "List()", "GenreCatalog list contract")

    genre_catalog_cpp = (root / "GenreCatalog.cpp").read_text()
    reject(genre_catalog_cpp, '#include "Croon.h"', "GenreCatalog app shell dependency")
    require(genre_catalog_cpp, '#include "GenreCatalog.h"', "GenreCatalog direct self dependency")
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

    direct_lyrics_dependencies = {
        "ExportDlg.cpp": ["SubtitleGenerator::ToAss"],
        "GatherDlg.cpp": ["TextTools::ShortenMiddle"],
        "OpenProjectDlg.cpp": ["LyricsTransformer::TimedToRaw"],
        "Page2.cpp": ["LyricsDownloadService::Download", "TextTools::CleanSpacing"],
        "Page3.cpp": ["LyricsTransformer::RawToUntimed"],
        "Project.cpp": ["LyricsTransformer::RawToUntimed", "SubtitleGenerator::ToRichAss"],
        "ProjectList.cpp": ["LyricsTransformer::TimedToRaw", "TextTools::ShortenMiddle"],
        "ProjectLoader.cpp": ["LyricsTransformer::TimedToRaw"],
        "TimingDlg.cpp": ["LyricsTransformer::TimedToRaw"],
        "TimingLine.cpp": ["LyricsTransformer::SplitDecorations"],
    }
    for rel, expected_calls in direct_lyrics_dependencies.items():
        text = (root / rel).read_text()
        for expected in expected_calls:
            require(text, expected, f"{rel} direct lyric service dependency")
        for wrapper in [
            " DownloadLyrics(",
            " CleanSpacing(",
            " ShortenMiddle(",
            "SplitLyrics(",
            "RawToUntimedLyrics(",
            "TimedLyricsToRaw(",
            "TimedToASS(",
            "TimedToRichASS(",
        ]:
            reject(text, wrapper, f"{rel} lyric utility wrapper dependency")

    direct_time_progress_dependencies = {
        "ConvertDlg.cpp": ["FfmpegProgressParser::ParseTimestamp"],
        "ExportDlg.cpp": ["FfmpegProgressParser::ParseTimestamp"],
        "LyricsEditor.cpp": ["TimeFormatter::Format"],
        "TimingDlg.cpp": ["TimeFormatter::Format"],
        "TimingLine.cpp": ["TimeFormatter::Format"],
    }
    for rel, expected_calls in direct_time_progress_dependencies.items():
        text = (root / rel).read_text()
        for expected in expected_calls:
            require(text, expected, f"{rel} direct time/progress dependency")
        for wrapper in [
            "ComputeFfmpegTs(",
            " FormatTime(",
            "FormatTimeASS(",
            "FormatTimeSRT(",
            "FormatTime2(",
        ]:
            reject(text, wrapper, f"{rel} time/progress utility wrapper dependency")

    direct_path_catalog_dependencies = {
        "ConfigService.cpp": ["AppPaths::DataDirectory"],
        "GatherDlg.cpp": ["AppPaths::DataDirectory", "AppPaths::FindFiles"],
        "Page1.cpp": ["GenreCatalog::List"],
        "Page3.cpp": ["AppPaths::DataDirectory", "AppPaths::FindFiles"],
        "Project.cpp": ["GenreCatalog::List"],
    }
    for rel, expected_calls in direct_path_catalog_dependencies.items():
        text = (root / rel).read_text()
        for expected in expected_calls:
            require(text, expected, f"{rel} direct path/catalog dependency")
        for wrapper in [
            "GetDataDirectory(",
            "GetPaths(",
            "GetGenres(",
        ]:
            reject(text, wrapper, f"{rel} path/catalog utility wrapper dependency")

    direct_ui_scaler_dependencies = [
        "ListCtrl.cpp",
        "ListCtrl.h",
        "LyricsPartsDlg.cpp",
        "MainWindow.cpp",
        "OpenProjectDlg.cpp",
        "Page3.cpp",
        "ProjectList.h",
        "SaveProjectDlg.cpp",
        "TimingDlg.cpp",
        "TimingCtrl.cpp",
        "VideoDlg.cpp",
    ]
    for rel in direct_ui_scaler_dependencies:
        text = (root / rel).read_text()
        require(text, "UiScaler::", f"{rel} direct UI scaler dependency")
        reject(text, "_Zx(", f"{rel} UI scaler utility wrapper dependency")
        reject(text, "_Zy(", f"{rel} UI scaler utility wrapper dependency")
    for rel in ["ListCtrl.cpp", "ListCtrl.h", "LyricsPartsDlg.cpp", "MainWindow.cpp", "OpenProjectDlg.cpp", "ProjectList.h", "SaveProjectDlg.cpp", "TimingDlg.cpp", "TimingCtrl.cpp", "VideoDlg.cpp"]:
        text = (root / rel).read_text()
        reject(text, "Zx(", f"{rel} raw horizontal scaler dependency")
        reject(text, "Zy(", f"{rel} raw vertical scaler dependency")

    list_ctrl_cpp = (root / "ListCtrl.cpp").read_text()
    reject(list_ctrl_cpp, '#include "Croon.h"', "ListCtrl app shell dependency")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#include <cmath>",
        '#include "UiScaler.h"',
        '#include "ListCtrl.h"',
    ]:
        require(list_ctrl_cpp, needle, "ListCtrl direct dependency")
    for needle in [
        "scrollBar.WhenScroll",
        "SetOrientation(Vertical",
        "ListCtrl::Key",
        "ListCtrl::AddChild",
        "ListCtrl::LayCtrlsGrid",
        "ScrollToItemAndCenter",
    ]:
        require(list_ctrl_cpp, needle, "ListCtrl behavior contract")

    lyrics_parts_ctrl_cpp = (root / "LyricsPartsCtrl.cpp").read_text()
    reject(lyrics_parts_ctrl_cpp, '#include "Croon.h"', "LyricsPartsCtrl app shell dependency")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        '#include "LyricsPartsCtrl.h"',
    ]:
        require(lyrics_parts_ctrl_cpp, needle, "LyricsPartsCtrl direct dependency")
    for needle in [
        "void LyricsPartsCtrl::Paint",
        "w.Clip(clipr)",
        "w.DrawText",
        "w.DrawLine",
        "Color(255, 220, 120)",
        "SColorText()",
    ]:
        require(lyrics_parts_ctrl_cpp, needle, "LyricsPartsCtrl paint contract")


if __name__ == "__main__":
    main()
