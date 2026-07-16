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
        "DownloadLyricsWithStatus",
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
        "LyricsDownloadService.cpp",
        "LyricsDownloadService.h",
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
    identity_cpp = (root / "AppIdentity.cpp").read_text()
    for needle in [
        "ProductName()",
        "Version()",
        "ProjectExtension()",
        "ProjectGlob()",
        "MetadataAttachmentName()",
        "TempPrefix()",
        "PosixDataDirectory()",
        "DataDirectory()",
    ]:
        require(identity_h, needle, "AppIdentity contract")
    for needle in [
        'return "Croon"',
        'return "1.1"',
        'return ".croon"',
        'return "*.croon"',
        'return "croon.info"',
        'return "Croon_"',
        'return ".Croon"',
        "GetTempFileName(TempPrefix())",
    ]:
        require(identity_cpp, needle, "AppIdentity implementation contract")
    reject(identity_h, 'return "Croon"', "AppIdentity inline implementation")

    app_paths_cpp = (root / "AppPaths.cpp").read_text()
    reject(app_paths_cpp, '#include "Croon.h"', "AppPaths app shell dependency")
    require(app_paths_cpp, '#include "AppIdentity.h"', "AppPaths direct identity dependency")
    require(app_paths_cpp, '#include "AppPaths.h"', "AppPaths direct self dependency")
    require(app_paths_cpp, "AppIdentity::PosixDataDirectory", "POSIX app data contract")
    require(app_paths_cpp, "AppIdentity::DataDirectory", "non-POSIX app data contract")
    require(app_paths_cpp, "DirectoryCreate(dataDir)", "AppPaths data directory creation")
    require(app_paths_cpp, "PatternMatchMulti(pattern, ff.GetName())", "AppPaths file discovery")

    config_h = (root / "Config.h").read_text()
    require(config_h, "static String Get", "Config get declaration")
    require(config_h, "static Config& Set", "Config set declaration")
    require(config_h, "static int GetInt", "Config int get declaration")
    require(config_h, "static int GetFontSize", "Config font-size declaration")
    reject(config_h, "ConfigService::", "Config inline service delegation")

    config_cpp = (root / "Config.cpp").read_text()
    reject(config_cpp, '#include "Croon.h"', "Config app shell dependency")
    require(config_cpp, '#include "ConfigService.h"\n#include "Config.h"', "Config ordered direct dependencies")
    require(config_cpp, "Config Config::config", "Config singleton storage")
    require(config_cpp, "ConfigService::DefaultFontSize", "Config default font-size delegation")
    require(config_cpp, "ConfigService::Get(key, defaultValue)", "Config get delegation")
    require(config_cpp, "ConfigService::Set(key, value)", "Config set delegation")
    require(config_cpp, "ConfigService::GetInt(key, defaultValue)", "Config int get delegation")
    require(config_cpp, "ConfigService::GetFontSize()", "Config font-size delegation")

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
        "LRC_EXPORT_DIR",
        "PROJECT_LIST",
        "WIN_X",
        "WIN_H",
    ]:
        require(config_service_cpp, key, "ConfigService registered key contract")
    require(config_service_cpp, "SerializeGlobalConfigs", "ConfigService persistence contract")
    require(config_service_cpp, "std::max(MinFontSize, std::min(MaxFontSize", "ConfigService font-size clamp")

    app_audio_player_h = (root / "AppAudioPlayer.h").read_text()
    app_audio_player_cpp = (root / "AppAudioPlayer.cpp").read_text()
    require(app_audio_player_h, "struct AppAudioPlayer", "AppAudioPlayer explicit boundary declaration")
    reject(app_audio_player_h, '#include "SDLMixerAudioPlayer.h"', "AppAudioPlayer backend header dependency")
    reject(app_audio_player_h, "SDLMixerAudioPlayer::", "AppAudioPlayer inline backend delegation")
    reject(app_audio_player_h, "static SDLMixerAudioPlayer& Player()", "AppAudioPlayer private backend accessor")
    require(app_audio_player_h, "#include <Core/Core.h>", "AppAudioPlayer String dependency")
    require(app_audio_player_h, "static bool Open(const Upp::String& filename)", "AppAudioPlayer open contract")
    require(app_audio_player_h, "static bool Close()", "AppAudioPlayer close contract")
    require(app_audio_player_h, "static bool Reopen()", "AppAudioPlayer reopen contract")
    require(app_audio_player_h, "static bool Seek(double seconds)", "AppAudioPlayer seek contract")
    require(app_audio_player_h, "static bool IsPlaying()", "AppAudioPlayer playing-state contract")
    require(app_audio_player_h, "static double Duration()", "AppAudioPlayer duration contract")
    require(app_audio_player_cpp, '#include "AppAudioPlayer.h"', "AppAudioPlayer implementation facade dependency")
    require(app_audio_player_cpp, '#include "SDLMixerAudioPlayer.h"', "AppAudioPlayer implementation backend dependency")
    for needle in [
        "SDLMixerAudioPlayer::InitPlayer()",
        "SDLMixerAudioPlayer::DeInitPlayer()",
        "SDLMixerAudioPlayer::GetPlayer().Open(filename)",
        "SDLMixerAudioPlayer::GetPlayer().Close()",
        "SDLMixerAudioPlayer::GetPlayer().Reopen()",
        "SDLMixerAudioPlayer::GetPlayer().Pause()",
        "SDLMixerAudioPlayer::GetPlayer().Play()",
        "SDLMixerAudioPlayer::GetPlayer().Seek(seconds)",
        "SDLMixerAudioPlayer::GetPlayer().IsPlaying()",
        "SDLMixerAudioPlayer::GetPlayer().IsOpen()",
        "SDLMixerAudioPlayer::GetPlayer().Duration()",
        "SDLMixerAudioPlayer::GetPlayer().Position()",
    ]:
        require(app_audio_player_cpp, needle, "AppAudioPlayer implementation backend delegation")
    if (root / "AudioPlayerBase.h").exists():
        fail("obsolete AudioPlayerBase wrapper still exists")
    sdl_mixer_audio_player_h = (root / "SDLMixerAudioPlayer.h").read_text()
    sdl_mixer_audio_player_cpp = (root / "SDLMixerAudioPlayer.cpp").read_text()
    reject(sdl_mixer_audio_player_h, '#include "AudioPlayerBase.h"', "SDLMixerAudioPlayer obsolete audio base dependency")
    reject(sdl_mixer_audio_player_h, "public AudioPlayerBase", "SDLMixerAudioPlayer obsolete audio base inheritance")
    reject(sdl_mixer_audio_player_h, "initialized", "SDLMixerAudioPlayer dead initialized flag")
    reject(sdl_mixer_audio_player_h, "GetPosition", "SDLMixerAudioPlayer dead position declaration")
    reject(sdl_mixer_audio_player_h, " override", "SDLMixerAudioPlayer unused override marker")
    reject(sdl_mixer_audio_player_h, "State() override", "SDLMixerAudioPlayer unused state accessor")
    reject(sdl_mixer_audio_player_h, "GetPath() const override", "SDLMixerAudioPlayer unused path accessor")
    reject(sdl_mixer_audio_player_h, "LastError", "SDLMixerAudioPlayer unused last-error API")
    reject(sdl_mixer_audio_player_h, "WhenError", "SDLMixerAudioPlayer unused error event")
    reject(sdl_mixer_audio_player_h, "lastError", "SDLMixerAudioPlayer unused error storage")
    reject(sdl_mixer_audio_player_h, "#include <SDL2/SDL.h>", "SDLMixerAudioPlayer public SDL dependency")
    reject(sdl_mixer_audio_player_h, "#include <SDL2/SDL_mixer.h>", "SDLMixerAudioPlayer public SDL_mixer dependency")
    reject(sdl_mixer_audio_player_h, "#include <SDL.h>", "SDLMixerAudioPlayer public SDL dependency")
    reject(sdl_mixer_audio_player_h, "#include <SDL_mixer.h>", "SDLMixerAudioPlayer public SDL_mixer dependency")
    reject(sdl_mixer_audio_player_h, "SDL_Quit()", "SDLMixerAudioPlayer inline SDL shutdown")
    reject(sdl_mixer_audio_player_h, "Mix_FreeMusic", "SDLMixerAudioPlayer inline SDL cleanup")
    reject(sdl_mixer_audio_player_h, "Mix_PlayingMusic", "SDLMixerAudioPlayer inline SDL state check")
    reject(sdl_mixer_audio_player_h, "static SDLMixerAudioPlayer player", "SDLMixerAudioPlayer header-owned singleton storage")
    reject(sdl_mixer_audio_player_cpp, "SDLMixerAudioPlayer SDLMixerAudioPlayer::player", "SDLMixerAudioPlayer global singleton storage")
    reject(sdl_mixer_audio_player_cpp, "initialized", "SDLMixerAudioPlayer dead initialized storage")
    reject(sdl_mixer_audio_player_cpp, "LastError", "SDLMixerAudioPlayer unused last-error implementation")
    reject(sdl_mixer_audio_player_cpp, "WhenError", "SDLMixerAudioPlayer unused error event usage")
    reject(sdl_mixer_audio_player_cpp, '#include "Croon.h"', "SDLMixerAudioPlayer app shell dependency")
    require(sdl_mixer_audio_player_h, "enum AudioPlayerState", "SDLMixerAudioPlayer state contract")
    require(sdl_mixer_audio_player_h, "#include <Core/Core.h>", "SDLMixerAudioPlayer public String dependency")
    require(sdl_mixer_audio_player_h, "typedef struct Mix_Music Mix_Music", "SDLMixerAudioPlayer opaque music declaration")
    require(sdl_mixer_audio_player_h, "static SDLMixerAudioPlayer& GetPlayer()", "SDLMixerAudioPlayer singleton accessor contract")
    require(sdl_mixer_audio_player_h, "bool Open(const Upp::String& filename)", "SDLMixerAudioPlayer open contract")
    require(sdl_mixer_audio_player_h, "virtual ~SDLMixerAudioPlayer()", "SDLMixerAudioPlayer cleanup declaration")
    require(sdl_mixer_audio_player_h, "bool IsPlaying()", "SDLMixerAudioPlayer state check declaration")
    require(sdl_mixer_audio_player_h, "bool IsOpen()", "SDLMixerAudioPlayer open-state declaration")
    require(sdl_mixer_audio_player_h, "bool Reopen()", "SDLMixerAudioPlayer reopen declaration")
    require(sdl_mixer_audio_player_h, "void ReportError(const Upp::String& error)", "SDLMixerAudioPlayer private error reporting")
    for needle in [
        "#include <atomic>",
    ]:
        require(sdl_mixer_audio_player_h, needle, "SDLMixerAudioPlayer backend header dependency")
    for needle in [
        "#include <Core/Core.h>",
        "#include <SDL2/SDL.h>",
        "#include <SDL2/SDL_mixer.h>",
        '#include "SDLMixerAudioPlayer.h"',
    ]:
        require(sdl_mixer_audio_player_cpp, needle, "SDLMixerAudioPlayer direct dependency")
    for needle in [
        "SDL_Init(SDL_INIT_AUDIO)",
        "Mix_OpenAudio",
        "Mix_LoadMUS",
        "Mix_PlayMusic",
        "Mix_SetMusicPosition",
        "Mix_HaltMusic",
        "Mix_PlayingMusic",
        "void SDLMixerAudioPlayer::DeInitPlayer()",
        "SDLMixerAudioPlayer& SDLMixerAudioPlayer::GetPlayer()",
        "static SDLMixerAudioPlayer player",
        "SDLMixerAudioPlayer::~SDLMixerAudioPlayer()",
        "void SDLMixerAudioPlayer::ReportError",
        "ReportError(String(",
    ]:
        require(sdl_mixer_audio_player_cpp, needle, "SDLMixerAudioPlayer playback contract")

    if (root / "Croon.h").exists():
        fail("obsolete Croon.h umbrella header still exists")
    if (root / "MusicPlayer.h").exists():
        fail("obsolete MusicPlayer compatibility facade still exists")
    if (root / "AudioPlayer.h").exists():
        fail("obsolete generic AudioPlayer wrapper still exists")
    croon_upp = (root / "Croon.upp").read_text()
    reject(croon_upp, "Croon.h", "Croon.upp obsolete umbrella header")
    reject(croon_upp, "MusicPlayer.h", "Croon.upp obsolete music player facade")
    reject(croon_upp, "\tAudioPlayer.h,", "Croon.upp obsolete generic audio wrapper")
    reject(croon_upp, "\tAudioPlayerBase.h,", "Croon.upp obsolete audio base wrapper")
    require(croon_upp, "AppAudioPlayer.cpp", "Croon.upp app audio facade implementation")
    for path in sorted(root.glob("*")):
        if path.suffix not in {".cpp", ".h"}:
            continue
        reject(path.read_text(), '#include "Croon.h"', f"{path.name} umbrella dependency")

    croon_cpp = (root / "Croon.cpp").read_text()
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#include <ctime>",
        "#include <filesystem>",
        "#define IMAGECLASS CroonImg",
        "#define IMAGEFILE <Croon/Croon.iml>",
        "#include <Draw/iml_header.h>",
        "#include <Draw/iml_source.h>",
        '#include "Constants.h"',
        '#include "AppIdentity.h"',
        '#include "AppPaths.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "KarData.h"',
        '#include "LyricsTransformer.h"',
        '#include "SubtitleGenerator.h"',
        '#include "TimeFormatter.h"',
        '#include "UiScaler.h"',
        '#include "GenreCatalog.h"',
        '#include "RichTextBuilder.h"',
        '#include "TextTools.h"',
        '#include "Visualization.h"',
        '#include "FfmpegProgressParser.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ListCtrl.h"',
        '#include "LyricsPartsCtrl.h"',
        '#include "ProjectLoader.h"',
        '#include "Page.h"',
        '#include "LyricsDownloadService.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "ProgressDlg.h"',
        '#include "ConvertDlg.h"',
        '#include "OpenProjectDlg.h"',
        '#include "SaveProjectDlg.h"',
        '#include "ExportDlg.h"',
        '#include "LyricsPartsDlg.h"',
        '#include "TimingLine.h"',
        '#include "TimingCtrl.h"',
        "#define LAYOUTFILE <Croon/CroonTimingDlg.lay>",
        '#include "AppAudioPlayer.h"',
        '#include "TimingDlg.h"',
        '#include "VidThumbnail.h"',
        '#include "Page1.h"',
        '#include "Page2.h"',
        '#include "Page3.h"',
        "#define LAYOUTFILE <Croon/CroonWizardShell.lay>",
        "#define LAYOUTFILE <Croon/CroonVideoDlg.lay>",
        '#include "VideoDlg.h"',
        '#include "WizardDlg.h"',
        '#include "Project.h"',
        '#include "ProjectList.h"',
        "#define LAYOUTFILE <Croon/CroonMainWindow.lay>",
        '#include "MainWindow.h"',
    ]:
        require(croon_cpp, needle, "Croon app shell direct dependency")
    for needle in ["#include <atomic>", "#include <SDL2/SDL.h>", "#include <SDL2/SDL_mixer.h>", "#include <SDL.h>", "#include <SDL_mixer.h>"]:
        reject(croon_cpp, needle, "Croon app shell direct SDL/audio dependency")
    for needle in [
        "Config::Get(FFMPEG_LOCATION)",
        "MediaProcessRunner proc",
        "proc.Start(ffmpegLoc)",
        "Config::Set(FFMPEG_LOCATION, ffmpegLoc)",
        "AppAudioPlayer::InitPlayer()",
        "KarData data;",
        "MainWindow(data).Run()",
        "AppAudioPlayer::DeInitPlayer()",
    ]:
        require(croon_cpp, needle, "Croon app launch workflow")
    reject(croon_cpp, "MainWindow(KarData::GetGlobal()).Run()", "Croon launch global data wiring")
    reject(croon_cpp, "GatherDlg& GetGatherDlg()", "Croon app shell global gather accessor")
    reject(croon_cpp, "VideoDlg& GetVideoDlg()", "Croon app shell global video accessor")
    reject(croon_cpp, "WizardDlg& GetWizardDlg()", "Croon app shell global wizard accessor")
    reject(croon_cpp, "(void)GetVideoDlg()", "Croon launch global video dialog prewarm")
    reject(croon_cpp, "(void)GetWizardDlg()", "Croon launch global wizard dialog prewarm")
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

    decisions_md = (root / "decisions.md").read_text()
    require(decisions_md, "### Croon Metadata Compatibility", "Croon metadata compatibility decision")
    require(decisions_md, "Croon must keep `.croon`", "Croon project artifact compatibility decision")
    require(decisions_md, "ProjectSerializer", "Croon metadata serializer decision")
    require(decisions_md, "legacy `1.0` and unversioned `.croon` metadata are treated as the current readable format", "legacy .croon metadata compatibility decision")
    require(decisions_md, "unsupported explicit metadata versions are rejected by load gates", "unsupported .croon metadata compatibility decision")
    require(decisions_md, "### Explicit Runtime Project State", "explicit runtime project state decision")
    require(decisions_md, "legacy global `KarData` accessor has been removed", "removed global data decision")
    require(decisions_md, "### VideoCatalog Owns Video Discovery", "VideoCatalog discovery decision")
    require(decisions_md, "Configured video directory enumeration, cached video listing, thumbnail file path construction, and cached thumbnail image loading belong behind `VideoCatalog`", "VideoCatalog enumeration ownership decision")
    require(decisions_md, "### No Legacy Product Artifact Import", "legacy product artifact import decision")
    require(decisions_md, "will not add import support for older legacy product metadata", "legacy product metadata import decision")
    reject(decisions_md, "Whether to remove global project state after the first functional migration.", "resolved global project state deferred decision")
    require(decisions_md, "expand `VideoCatalog` into a `VideoLibraryCache`", "video library cache deferred decision")
    require(decisions_md, "Incremental video picker loading handles the current responsiveness problem", "video library cache performance decision")

    architecture_md = (root / "architecture.md").read_text()
    require(architecture_md, "`MainWindow` is the normal composition root", "MainWindow composition root documentation")
    require(architecture_md, "`Project`, `ProjectList`, `VideoDlg`, and `WizardDlg`", "MainWindow runtime dialog ownership documentation")
    require(architecture_md, "legacy global data and dialog accessors have been retired", "global accessor retirement documentation")

    project_loader_cpp = (root / "ProjectLoader.cpp").read_text()
    project_loader_h = (root / "ProjectLoader.h").read_text()
    reject(project_loader_h, "Config::Get(FFMPEG_LOCATION)", "ProjectLoader inline config lookup")
    require(project_loader_h, "ProjectLoader();", "ProjectLoader constructor declaration")
    require(project_loader_h, "void StopLoading();", "ProjectLoader stop loading declaration")
    require(project_loader_h, "const Vector<String>& ProjectPaths() const", "ProjectLoader recent path access")
    reject(project_loader_cpp, '#include "Croon.h"', "ProjectLoader app shell dependency")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#define IMAGECLASS CroonImg",
        "#define IMAGEFILE <Croon/Croon.iml>",
        "#include <Draw/iml_header.h>",
        '#include "AppIdentity.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "KarData.h"\n#include "ProjectSerializer.h"',
        '#include "ProjectSerializer.h"\n#include "Visualization.h"\n#include "FfmpegProjectCommandBuilder.h"',
        '#include "LyricsTransformer.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ProjectLoader.h"',
    ]:
        require(project_loader_cpp, needle, "ProjectLoader direct dependency")
    require(project_loader_cpp, "RecentProjectService::LoadPaths()", "ProjectLoader recent-project load contract")
    require(project_loader_cpp, "ProjectLoader::ProjectLoader()", "ProjectLoader constructor implementation")
    require(project_loader_cpp, "void ProjectLoader::StopLoading()", "ProjectLoader stop loading implementation")
    require(project_loader_cpp, "process.Kill()", "ProjectLoader kill active background process")
    require(project_loader_cpp, "ffmpeg = Config::Get(FFMPEG_LOCATION)", "ProjectLoader ffmpeg config lookup")
    require(project_loader_cpp, "AppIdentity::TempFileName", "ProjectLoader temp-file identity contract")
    require(project_loader_cpp, "FfmpegProjectCommandBuilder::DumpAttachmentAndGenerateThumbnail", "ProjectLoader ffmpeg command contract")
    require(project_loader_cpp, "String metadata = LoadFile(infoFilePath)", "ProjectLoader metadata read contract")
    require(project_loader_cpp, "ProjectSerializer::SupportsJson(metadata)", "ProjectLoader metadata compatibility gate")
    require(project_loader_cpp, "KarData data{metadata}", "ProjectLoader metadata load contract")
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

    azlyrics_provider_h = (root / "AzLyricsProvider.h").read_text()
    require(azlyrics_provider_h, "struct AzLyricsProvider", "AzLyricsProvider boundary")
    require(azlyrics_provider_h, "Name()", "AzLyricsProvider name contract")
    require(azlyrics_provider_h, "BuildUrl", "AzLyricsProvider URL contract")
    require(azlyrics_provider_h, "ExtractLyrics", "AzLyricsProvider extraction contract")
    require(azlyrics_provider_h, "UrlFormat", "AzLyricsProvider URL format boundary")
    require(azlyrics_provider_h, "LyricsPattern", "AzLyricsProvider pattern boundary")

    azlyrics_provider_cpp = (root / "AzLyricsProvider.cpp").read_text()
    reject(azlyrics_provider_cpp, '#include "Croon.h"', "AzLyricsProvider app shell dependency")
    require(azlyrics_provider_cpp, "#include <Core/Core.h>", "AzLyricsProvider direct Core dependency")
    require(azlyrics_provider_cpp, "#include <plugin/pcre/Pcre.h>", "AzLyricsProvider regex dependency")
    require(azlyrics_provider_cpp, '#include "TextTools.h"', "AzLyricsProvider text helper dependency")
    require(azlyrics_provider_cpp, '#include "AzLyricsProvider.h"', "AzLyricsProvider direct self dependency")
    require(azlyrics_provider_cpp, 'return "AZLyrics"', "AzLyricsProvider active provider name")
    require(azlyrics_provider_cpp, "https://www.azlyrics.com/lyrics/%s/%s.html", "AzLyricsProvider provider URL")
    require(azlyrics_provider_cpp, "<!-- Usage of azlyrics.com content", "AzLyricsProvider extraction pattern")
    require(azlyrics_provider_cpp, "TextTools::CleanSpacing", "AzLyricsProvider spacing cleanup")
    require(azlyrics_provider_cpp, "TextTools::StripNonAlnum", "AzLyricsProvider URL sanitization")
    require(azlyrics_provider_cpp, 'a.TrimStart("the ")', "AzLyricsProvider leading artist prefix cleanup")
    require(azlyrics_provider_cpp, "RegExp rx", "AzLyricsProvider extraction regex")

    lyrics_provider_tools_h = (root / "LyricsProviderTools.h").read_text()
    lyrics_provider_tools_cpp = (root / "LyricsProviderTools.cpp").read_text()
    require(lyrics_provider_tools_h, "struct LyricsProviderTools", "LyricsProviderTools boundary")
    require(lyrics_provider_tools_h, "HyphenSlug", "LyricsProviderTools hyphen slug contract")
    require(lyrics_provider_tools_h, "CompactSlug", "LyricsProviderTools compact slug contract")
    require(lyrics_provider_tools_h, "CleanupHtmlLyrics", "LyricsProviderTools HTML cleanup contract")
    reject(lyrics_provider_tools_cpp, '#include "Croon.h"', "LyricsProviderTools app shell dependency")
    require(lyrics_provider_tools_cpp, '#include "TextTools.h"', "LyricsProviderTools text helper dependency")
    require(lyrics_provider_tools_cpp, "StripVersionSuffixes", "LyricsProviderTools version suffix cleanup")
    require(lyrics_provider_tools_cpp, "StripFeatureSuffix", "LyricsProviderTools feature suffix cleanup")
    require(lyrics_provider_tools_cpp, "DecodeHtmlEntities", "LyricsProviderTools entity cleanup")

    genius_provider_h = (root / "GeniusLyricsProvider.h").read_text()
    genius_provider_cpp = (root / "GeniusLyricsProvider.cpp").read_text()
    require(genius_provider_h, "struct GeniusLyricsProvider", "GeniusLyricsProvider boundary")
    require(genius_provider_h, "BuildUrl", "GeniusLyricsProvider URL contract")
    require(genius_provider_h, "ExtractLyrics", "GeniusLyricsProvider extraction contract")
    reject(genius_provider_cpp, '#include "Croon.h"', "GeniusLyricsProvider app shell dependency")
    require(genius_provider_cpp, "https://genius.com/%s-%s-lyrics", "GeniusLyricsProvider URL format")
    require(genius_provider_cpp, "LyricsProviderTools::HyphenSlug", "GeniusLyricsProvider slug helper")
    require(genius_provider_cpp, "data-lyrics-container", "GeniusLyricsProvider lyric container extraction")

    songlyrics_provider_h = (root / "SongLyricsProvider.h").read_text()
    songlyrics_provider_cpp = (root / "SongLyricsProvider.cpp").read_text()
    require(songlyrics_provider_h, "struct SongLyricsProvider", "SongLyricsProvider boundary")
    require(songlyrics_provider_h, "BuildUrl", "SongLyricsProvider URL contract")
    require(songlyrics_provider_h, "ExtractLyrics", "SongLyricsProvider extraction contract")
    reject(songlyrics_provider_cpp, '#include "Croon.h"', "SongLyricsProvider app shell dependency")
    require(songlyrics_provider_cpp, "https://www.songlyrics.com/%s/%s-lyrics/", "SongLyricsProvider URL format")
    require(songlyrics_provider_cpp, "LyricsProviderTools::HyphenSlug", "SongLyricsProvider slug helper")
    require(songlyrics_provider_cpp, "songLyricsDiv", "SongLyricsProvider lyric block extraction")

    lyrics_download_service_h = (root / "LyricsDownloadService.h").read_text()
    require(lyrics_download_service_h, "DownloadStatus", "LyricsDownloadService status contract")
    require(lyrics_download_service_h, "DownloadOk", "LyricsDownloadService success status")
    require(lyrics_download_service_h, "DownloadCancelled", "LyricsDownloadService cancelled status")
    require(lyrics_download_service_h, "ExtractionFailed", "LyricsDownloadService extraction failure status")
    require(lyrics_download_service_h, "DownloadStatusLabel", "LyricsDownloadService status label contract")
    require(lyrics_download_service_h, "BuildProviderUrl", "LyricsDownloadService provider-neutral URL contract")
    require(lyrics_download_service_h, "ExtractProviderLyrics", "LyricsDownloadService provider-neutral extraction contract")
    reject(lyrics_download_service_h, "BuildAzLyricsUrl", "LyricsDownloadService AZ URL alias")
    reject(lyrics_download_service_h, "ExtractAzLyrics", "LyricsDownloadService AZ extraction alias")
    require(lyrics_download_service_h, "DownloadWithStatus", "LyricsDownloadService status download contract")
    require(lyrics_download_service_h, "Download", "LyricsDownloadService download contract")
    require(lyrics_download_service_h, "ProviderName", "LyricsDownloadService provider name contract")
    reject(lyrics_download_service_h, "AzLyricsUrlFormat", "LyricsDownloadService provider URL extraction")
    reject(lyrics_download_service_h, "AzLyricsPattern", "LyricsDownloadService provider pattern extraction")

    lyrics_download_service_cpp = (root / "LyricsDownloadService.cpp").read_text()
    reject(lyrics_download_service_cpp, '#include "Croon.h"', "LyricsDownloadService app shell dependency")
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
        '#include "AzLyricsProvider.h"',
        '#include "GeniusLyricsProvider.h"',
        '#include "SongLyricsProvider.h"',
        '#include "LyricsDownloadService.h"',
    ]:
        require(lyrics_download_service_cpp, needle, "LyricsDownloadService direct dependency")
    require(lyrics_download_service_cpp, "struct LyricsProvider", "LyricsDownloadService provider-chain entry")
    require(lyrics_download_service_cpp, "Vector<LyricsProvider> GetProviders()", "LyricsDownloadService provider-chain construction")
    require(lyrics_download_service_cpp, "AzLyricsProvider::BuildUrl", "LyricsDownloadService AZ provider URL")
    require(lyrics_download_service_cpp, "GeniusLyricsProvider::BuildUrl", "LyricsDownloadService Genius provider URL")
    require(lyrics_download_service_cpp, "SongLyricsProvider::BuildUrl", "LyricsDownloadService SongLyrics provider URL")
    reject(lyrics_download_service_cpp, "BuildAzLyricsUrl", "LyricsDownloadService AZ URL alias implementation")
    reject(lyrics_download_service_cpp, "ExtractAzLyrics", "LyricsDownloadService AZ extraction alias implementation")
    require(lyrics_download_service_cpp, "extractLyrics(content, *output)", "LyricsDownloadService provider-neutral download extraction")
    require(lyrics_download_service_cpp, "dlg.Run(provider.buildUrl(title, artist)", "LyricsDownloadService provider-neutral download URL")
    require(lyrics_download_service_cpp, "if (result == IDABORT)", "LyricsDownloadService user abort stops fallback chain")
    require(lyrics_download_service_cpp, '"Downloading lyrics"', "LyricsDownloadService opaque download prompt")
    reject(lyrics_download_service_cpp, 'Format("Downloading lyrics from %s", ProviderName())', "LyricsDownloadService user-facing provider prompt")
    require(lyrics_download_service_cpp, 'return "ok"', "LyricsDownloadService success status label")
    require(lyrics_download_service_cpp, 'return "cancelled"', "LyricsDownloadService cancelled status label")
    require(lyrics_download_service_cpp, 'return "extraction-failed"', "LyricsDownloadService extraction failure status label")
    reject(lyrics_download_service_cpp, "https://www.azlyrics.com/lyrics/%s/%s.html", "LyricsDownloadService provider URL extraction")
    reject(lyrics_download_service_cpp, "<!-- Usage of azlyrics.com content", "LyricsDownloadService extraction pattern extraction")
    reject(lyrics_download_service_cpp, "TextTools::CleanSpacing", "LyricsDownloadService spacing cleanup extraction")
    reject(lyrics_download_service_cpp, "TextTools::StripNonAlnum", "LyricsDownloadService URL sanitization extraction")
    reject(lyrics_download_service_cpp, "RegExp rx", "LyricsDownloadService extraction regex extraction")
    require(lyrics_download_service_cpp, "DownloadDlg dlg", "LyricsDownloadService UI download workflow")
    require(lyrics_download_service_cpp, "bool extracted = false", "LyricsDownloadService extraction status tracking")
    require(lyrics_download_service_cpp, "DownloadWithStatus(title, artist, lyrics) == DownloadOk", "LyricsDownloadService boolean compatibility wrapper")

    download_defaults_h = (root / "DownloadDefaults.h").read_text()
    download_defaults_cpp = (root / "DownloadDefaults.cpp").read_text()
    require(download_defaults_h, "struct DownloadDefaults", "DownloadDefaults boundary")
    require(download_defaults_h, "UserAgent()", "DownloadDefaults user-agent contract")
    reject(download_defaults_h, "Chrome/60.0.3112.90", "DownloadDefaults inline user-agent value")
    require(download_defaults_cpp, "Chrome/60.0.3112.90", "DownloadDefaults legacy user-agent value")

    download_dlg_h = (root / "DownloadDlg.h").read_text()
    download_dlg_cpp = (root / "DownloadDlg.cpp").read_text()
    require(download_dlg_h, "DownloadDefaults::UserAgent()", "DownloadDlg user-agent default")
    reject(download_dlg_h, "HttpRequest", "DownloadDlg header HTTP request dependency")
    reject(download_dlg_h, "ExtractLyrics", "DownloadDlg lyrics extraction dependency")
    reject(download_dlg_cpp, "DownloadDlg::ExtractLyrics", "DownloadDlg lyrics extraction implementation")
    reject(download_dlg_h, "RequestState* request", "DownloadDlg raw request-state ownership")
    reject(download_dlg_cpp, "new RequestState", "DownloadDlg raw request-state allocation")
    require(download_dlg_h, "#include <memory>", "DownloadDlg request ownership dependency")
    require(download_dlg_h, "struct RequestState", "DownloadDlg opaque request state")
    require(download_dlg_h, "std::unique_ptr<RequestState> request", "DownloadDlg opaque request storage")
    require(download_dlg_cpp, "request(std::make_unique<RequestState>())", "DownloadDlg request-state construction")

    decisions_md = (root / "decisions.md").read_text()
    require(decisions_md, "scraped providers remain internal implementation details behind `LyricsDownloadService`", "internal lyrics provider decision")
    require(decisions_md, "Genius and SongLyrics are fallback candidates", "future lyrics provider infrastructure decision")
    require(decisions_md, "user-facing download workflow stays opaque", "opaque lyrics provider workflow decision")
    services_md = (root / "services.md").read_text()
    require(services_md, "`AzLyricsProvider`", "AzLyricsProvider service documentation")
    require(services_md, "`GeniusLyricsProvider`", "GeniusLyricsProvider service documentation")
    require(services_md, "`SongLyricsProvider`", "SongLyricsProvider service documentation")
    require(services_md, "`LyricsProviderTools`", "LyricsProviderTools service documentation")
    require(services_md, "internal lyrics provider chain", "LyricsDownloadService internal provider chain documentation")
    require(services_md, "provider-neutral URL and extraction delegation", "LyricsDownloadService future provider delegation documentation")
    require(services_md, "opaque download workflow", "LyricsDownloadService opaque workflow documentation")
    require(services_md, "download status reporting", "LyricsDownloadService status documentation")
    require(services_md, "Provider-specific naming stays inside individual provider classes", "LyricsDownloadService provider-specific naming documentation")
    require(services_md, "`VideoCatalog`: video file discovery, cached video listing, thumbnail path, and cached thumbnail loading boundary for configured video directories", "VideoCatalog service documentation")
    require(services_md, "`VideoLibraryCache`: optional future service", "VideoLibraryCache planned service documentation")
    require(services_md, "streams video picker thumbnails incrementally", "VideoCatalog picker responsiveness documentation")
    require(services_md, "`GatherDlg`: removed modal video gathering dialog", "GatherDlg retirement documentation")
    require(services_md, "`LrcGenerator`: LRC lyric export after metadata/count-in processing", "LrcGenerator service documentation")
    require(services_md, "`FfmpegSubtitleProbeCommandBuilder`: deterministic ffmpeg arguments for raw RGBA subtitle probe rendering", "FfmpegSubtitleProbeCommandBuilder service documentation")
    require(services_md, "`SubtitleWrapProbe`: internal highlighted-line ASS probe generation", "SubtitleWrapProbe service documentation")
    require(services_md, "`SubtitleWrapProbeRunner`: internal ffmpeg/libass probe execution", "SubtitleWrapProbeRunner service documentation")

    constants_h = (root / "Constants.h").read_text()
    reject(constants_h, "AZ_URL", "Constants provider URL extraction")
    reject(constants_h, "AZ_PATTERN", "Constants provider pattern extraction")
    reject(constants_h, "USER_AGENT", "Constants user-agent extraction")
    reject(constants_h, "MaxASSDisplayLines", "Constants dead ASS max cleanup")
    reject(constants_h, "MinASSDisplayLines", "Constants dead ASS min cleanup")

    services_md = (root / "services.md").read_text()
    require(services_md, "## Retired Compatibility Facades", "services retired compatibility documentation")
    require(services_md, "`Util`: removed legacy facade", "Util retirement documentation")

    subtitle_line_processor_h = (root / "SubtitleLineProcessor.h").read_text()
    require(subtitle_line_processor_h, '#include "VocalPart.h"', "SubtitleLineProcessor vocal part dependency")
    require(subtitle_line_processor_h, "ProcessMetadata", "SubtitleLineProcessor metadata contract")
    require(subtitle_line_processor_h, "ResolveVocalPart", "SubtitleLineProcessor vocal-part contract")
    require(subtitle_line_processor_h, "ResolveStyle", "SubtitleLineProcessor style contract")

    subtitle_line_processor_cpp = (root / "SubtitleLineProcessor.cpp").read_text()
    reject(subtitle_line_processor_cpp, '#include "Croon.h"', "SubtitleLineProcessor app shell dependency")
    require(subtitle_line_processor_cpp, "#include <Draw/Draw.h>", "SubtitleLineProcessor direct image type dependency")
    require(subtitle_line_processor_cpp, '#include "SubtitleLineProcessor.h"', "SubtitleLineProcessor direct self dependency")
    require(subtitle_line_processor_cpp, '#include "KarData.h"', "SubtitleLineProcessor direct data dependency")
    require(subtitle_line_processor_cpp, '#include "TimeFormatter.h"', "SubtitleLineProcessor direct time formatter dependency")
    require(subtitle_line_processor_cpp, '#include "VocalPart.h"', "SubtitleLineProcessor direct vocal part dependency")
    require(subtitle_line_processor_cpp, "TimeFormatter::CountInDuration", "SubtitleLineProcessor count-in contract")
    require(subtitle_line_processor_cpp, "ReplaceMetadata", "SubtitleLineProcessor metadata replacement")
    require(subtitle_line_processor_cpp, "LookaheadVocalPart", "SubtitleLineProcessor count-in lookahead")

    subtitle_generator_h = (root / "SubtitleGenerator.h").read_text()
    require(subtitle_generator_h, "ToAss", "SubtitleGenerator ASS contract")
    require(subtitle_generator_h, "HighlightProbeLyrics", "SubtitleGenerator highlighted probe lyrics contract")
    require(subtitle_generator_h, "const Vector<bool>& wrappedHighlights", "SubtitleGenerator wrap-aware ASS contract")
    require(subtitle_generator_h, "ToRichAss", "SubtitleGenerator rich ASS contract")

    lrc_generator_h = (root / "LrcGenerator.h").read_text()
    require(lrc_generator_h, "ToLrc", "LrcGenerator LRC contract")

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
    reject(media_process_runner_h, "return process.", "MediaProcessRunner inline forwarding")
    reject(media_process_runner_h, "process.Kill();", "MediaProcessRunner inline forwarding")

    media_process_runner_cpp = (root / "MediaProcessRunner.cpp").read_text()
    reject(media_process_runner_cpp, '#include "Croon.h"', "MediaProcessRunner app shell dependency")
    require(media_process_runner_cpp, '#include "MediaProcessRunner.h"', "MediaProcessRunner direct self dependency")
    for method in [
        "bool MediaProcessRunner::Start(const String& command)",
        "bool MediaProcessRunner::Start(const String& executable, const Vector<String>& args)",
        "bool MediaProcessRunner::Start(const String& executable, const Vector<String>& args, const char *envptr, const char *dir)",
        "bool MediaProcessRunner::Read(String& output)",
        "bool MediaProcessRunner::IsRunning()",
        "int MediaProcessRunner::GetExitCode()",
        "void MediaProcessRunner::Kill()",
    ]:
        require(media_process_runner_cpp, method, "MediaProcessRunner forwarding implementation")

    subtitle_generator_cpp = (root / "SubtitleGenerator.cpp").read_text()
    require(subtitle_generator_cpp, "String SubtitleGenerator::ToAss", "SubtitleGenerator ASS implementation")
    require(subtitle_generator_cpp, "Vector<String> SubtitleGenerator::HighlightProbeLyrics", "SubtitleGenerator highlighted probe lyrics implementation")
    require(subtitle_generator_cpp, "IsWrappedHighlight", "SubtitleGenerator wrap flag helper")
    require(subtitle_generator_cpp, "SubtitleLineMoveTag", "SubtitleGenerator per-line wrap movement helper")
    require(subtitle_generator_cpp, "targetSlot - (wrapped ? 1:0)", "SubtitleGenerator raises only wrapped displayed rows")
    require(subtitle_generator_cpp, "lastLineWrapped", "SubtitleGenerator previous grayed wrap tracking")
    require(subtitle_generator_cpp, "nextWrapped", "SubtitleGenerator incoming wrap tracking")
    require(subtitle_generator_cpp, "String SubtitleGenerator::ToRichAss", "SubtitleGenerator rich ASS implementation")
    reject(subtitle_generator_cpp, '#include "Croon.h"', "SubtitleGenerator app shell dependency")
    require(subtitle_generator_cpp, '#include "SubtitleGenerator.h"', "SubtitleGenerator direct self dependency")
    require(subtitle_generator_cpp, '#include "SubtitleLineProcessor.h"', "SubtitleGenerator direct line processor dependency")
    require(subtitle_generator_cpp, '#include "TimeFormatter.h"', "SubtitleGenerator direct time formatter dependency")
    require(subtitle_generator_cpp, '#include "VocalPart.h"', "SubtitleGenerator direct vocal part dependency")
    require(subtitle_generator_cpp, "VocalPartStyle::V1PrimaryAss()", "SubtitleGenerator shared vocal color dependency")
    require(subtitle_generator_cpp, "SubtitleLineProcessor::ProcessMetadata", "SubtitleGenerator direct metadata dependency")
    require(subtitle_generator_cpp, "SubtitleLineProcessor::ResolveStyle", "SubtitleGenerator direct style dependency")
    require(subtitle_generator_cpp, "TimeFormatter::Ass", "SubtitleGenerator direct ASS time formatting")
    require(subtitle_generator_cpp, "SubtitleMoveTag", "SubtitleGenerator scrolling movement helper")
    require(subtitle_generator_cpp, "\\move(", "SubtitleGenerator ASS movement tag")
    require(subtitle_generator_cpp, "lastLine", "SubtitleGenerator previous grayed line tracking")
    reject(subtitle_generator_cpp, "olderLine", "SubtitleGenerator no extra outgoing grayed line")
    reject(subtitle_generator_cpp, "    ProcessMetadata(data", "SubtitleGenerator utility metadata wrapper dependency")
    reject(subtitle_generator_cpp, "String hilite = ResolveStyle", "SubtitleGenerator utility style wrapper dependency")
    reject(subtitle_generator_cpp, "FormatTimeASS", "SubtitleGenerator utility time wrapper dependency")

    subtitle_wrap_probe_h = (root / "SubtitleWrapProbe.h").read_text()
    subtitle_wrap_probe_cpp = (root / "SubtitleWrapProbe.cpp").read_text()
    require(subtitle_wrap_probe_h, "struct SubtitleWrapProbe", "SubtitleWrapProbe boundary")
    require(subtitle_wrap_probe_h, "BuildAss", "SubtitleWrapProbe ASS generation contract")
    require(subtitle_wrap_probe_h, "AnalyzeRgbaFrames", "SubtitleWrapProbe RGBA analysis contract")
    require(subtitle_wrap_probe_h, "IsWrappedFrame", "SubtitleWrapProbe wrapped decision contract")
    reject(subtitle_wrap_probe_cpp, '#include "Croon.h"', "SubtitleWrapProbe app shell dependency")
    require(subtitle_wrap_probe_cpp, "Style: V1,Arial", "SubtitleWrapProbe highlighted style contract")
    require(subtitle_wrap_probe_cpp, "TimeFormatter::Ass((double)i)", "SubtitleWrapProbe one-frame timestamp contract")
    require(subtitle_wrap_probe_cpp, "isTextPixel", "SubtitleWrapProbe text pixel scan contract")
    require(subtitle_wrap_probe_cpp, "max((int)red, max((int)green, (int)blue))", "SubtitleWrapProbe ignores opaque black background")
    require(subtitle_wrap_probe_cpp, "rowHasText", "SubtitleWrapProbe row band grouping")
    require(subtitle_wrap_probe_cpp, "CountTextLineGroups", "SubtitleWrapProbe text line grouping")
    require(subtitle_wrap_probe_cpp, "fontSize / 12", "SubtitleWrapProbe fragment gap tolerance")

    subtitle_wrap_probe_runner_h = (root / "SubtitleWrapProbeRunner.h").read_text()
    subtitle_wrap_probe_runner_cpp = (root / "SubtitleWrapProbeRunner.cpp").read_text()
    require(subtitle_wrap_probe_runner_h, "struct SubtitleWrapProbeRunner", "SubtitleWrapProbeRunner boundary")
    require(subtitle_wrap_probe_runner_h, "Run(const KarData& data", "SubtitleWrapProbeRunner run contract")
    reject(subtitle_wrap_probe_runner_cpp, '#include "Croon.h"', "SubtitleWrapProbeRunner app shell dependency")
    require(subtitle_wrap_probe_runner_cpp, '#include "AppIdentity.h"', "SubtitleWrapProbeRunner temp file dependency")
    require(subtitle_wrap_probe_runner_cpp, '#include "Config.h"', "SubtitleWrapProbeRunner ffmpeg config dependency")
    require(subtitle_wrap_probe_runner_cpp, '#include "MediaProcessRunner.h"', "SubtitleWrapProbeRunner process dependency")
    require(subtitle_wrap_probe_runner_cpp, "AppIdentity::TaggedTempFileName(\"subprobe\", \".ass\")", "SubtitleWrapProbeRunner ASS temp file")
    require(subtitle_wrap_probe_runner_cpp, "AppIdentity::TaggedTempFileName(\"subprobe\", \".rgba\")", "SubtitleWrapProbeRunner RGBA temp file")
    require(subtitle_wrap_probe_runner_cpp, "SaveFile(assPath, SubtitleWrapProbe::BuildAss", "SubtitleWrapProbeRunner ASS persistence")
    require(subtitle_wrap_probe_runner_cpp, "FfmpegSubtitleProbeCommandBuilder::RenderRgba", "SubtitleWrapProbeRunner ffmpeg command")
    require(subtitle_wrap_probe_runner_cpp, "process.Start(ffmpegPath, args)", "SubtitleWrapProbeRunner process start")
    require(subtitle_wrap_probe_runner_cpp, "LoadFile(rgbaPath)", "SubtitleWrapProbeRunner raw RGBA load")
    require(subtitle_wrap_probe_runner_cpp, "SubtitleWrapProbe::AnalyzeRgbaFrames", "SubtitleWrapProbeRunner analysis delegation")
    require(subtitle_wrap_probe_runner_cpp, "FileDelete(assPath)", "SubtitleWrapProbeRunner ASS cleanup")
    require(subtitle_wrap_probe_runner_cpp, "FileDelete(rgbaPath)", "SubtitleWrapProbeRunner RGBA cleanup")

    lrc_generator_cpp = (root / "LrcGenerator.cpp").read_text()
    reject(lrc_generator_cpp, '#include "Croon.h"', "LrcGenerator app shell dependency")
    require(lrc_generator_cpp, "#include <Draw/Draw.h>", "LrcGenerator direct image type dependency")
    require(lrc_generator_cpp, '#include "LrcGenerator.h"', "LrcGenerator direct self dependency")
    require(lrc_generator_cpp, '#include "SubtitleLineProcessor.h"', "LrcGenerator metadata processing dependency")
    require(lrc_generator_cpp, '#include "VocalPart.h"', "LrcGenerator vocal part dependency")
    require(lrc_generator_cpp, "SubtitleLineProcessor::ProcessMetadata", "LrcGenerator processed metadata export")
    require(lrc_generator_cpp, "std::round(seconds * 100.0)", "LrcGenerator centisecond timestamp rounding")
    require(lrc_generator_cpp, "************", "LrcGenerator count-in stars")
    require(lrc_generator_cpp, "VocalPartPrefix", "LrcGenerator vocal part prefix helper")
    require(lrc_generator_cpp, "AssignedVocalPart", "LrcGenerator explicit assignment helper")

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
    require(core_tests_cpp, "SubtitleGenerator::HighlightProbeLyrics", "core tests highlighted probe lyrics")
    require(core_tests_cpp, "wrappedHighlights[nextLineIdx] = true", "core tests wrapped highlight spacing flag")
    require(core_tests_cpp, "SubtitleGenerator raises highlighted slot when that line wraps", "core tests adaptive highlighted spacing")
    require(core_tests_cpp, "SubtitleGenerator raises grayed slot when the grayed line wraps", "core tests adaptive grayed spacing")
    require(core_tests_cpp, "SubtitleGenerator raises incoming slot when that incoming line wraps", "core tests adaptive incoming spacing")
    require(core_tests_cpp, "SubtitleWrapProbe::BuildAss", "core tests subtitle wrap probe generation")
    require(core_tests_cpp, "SubtitleWrapProbe::AnalyzeRgbaFrames", "core tests subtitle wrap probe analysis")
    require(core_tests_cpp, "OpaqueBackgroundProbeRgbaFixture", "core tests opaque background probe fixture")
    require(core_tests_cpp, "FragmentedSingleLineProbeRgbaFixture", "core tests fragmented single-line probe fixture")
    require(core_tests_cpp, "WrappedTwoLineProbeRgbaFixture", "core tests wrapped two-line probe fixture")
    require(core_tests_cpp, "SubtitleWrapProbe::IsWrappedFrame", "core tests wrapped frame decision")
    require(core_tests_cpp, "SubtitleWrapProbeRunner::Run", "core tests subtitle wrap probe runner")
    require(core_tests_cpp, "FfmpegSubtitleProbeCommandBuilder::RenderRgba", "core tests subtitle probe command builder")
    require(core_tests_cpp, "LrcGenerator::ToLrc", "core tests direct LRC generator dependency")
    require(core_tests_cpp, "VocalPartStyle::Next", "core tests direct vocal part helper dependency")
    require(core_tests_cpp, "VocalPartStyle::ToParts", "core tests vocal part tuple contract")
    require(core_tests_cpp, 'richAss.Find("@4")', "core tests rich ASS formatting assertion")
    require(core_tests_cpp, 'CountOccurrences(ass, "Dialogue: 0,0:00:05.00") == 4',
            "core tests 4-line subtitle page limit")
    require(core_tests_cpp, 'CountOccurrences(SubtitleGenerator::ToAss(exportData, 3), "Dialogue: 0,0:00:05.00") == 3',
            "core tests 3-line subtitle page limit")
    require(core_tests_cpp, '"[00:23.15][1]: Lately all my thoughts have gone to you"',
            "core tests LRC vocal prefix assertion")
    require(core_tests_cpp, '"[00:32.00](Young love)"',
            "core tests LRC backup vocal assertion")
    require(core_tests_cpp, "GeniusLyricsProvider::BuildUrl", "core tests Genius provider URL assertion")
    require(core_tests_cpp, "SongLyricsProvider::BuildUrl", "core tests SongLyrics provider URL assertion")
    require(core_tests_cpp, "LyricsProviderTools::HyphenSlug", "core tests lyrics provider slug assertion")
    require(core_tests_upp, "GeniusLyricsProvider.cpp", "core tests Genius provider implementation")
    require(core_tests_upp, "LyricsProviderTools.cpp", "core tests lyrics provider tools implementation")
    require(core_tests_upp, "SongLyricsProvider.cpp", "core tests SongLyrics provider implementation")
    require(core_tests_upp, "LyricsTransformer.cpp", "core tests lyrics transformer implementation")
    require(core_tests_upp, "LrcGenerator.cpp", "core tests LRC generator implementation")
    require(core_tests_upp, "MediaProcessRunner.cpp", "core tests media process runner implementation")
    require(core_tests_upp, "SubtitleGenerator.cpp", "core tests subtitle generator implementation")
    require(core_tests_upp, "SubtitleLineProcessor.cpp", "core tests subtitle line processor implementation")
    require(core_tests_upp, "SubtitleWrapProbe.cpp", "core tests subtitle wrap probe implementation")
    require(core_tests_upp, "SubtitleWrapProbeRunner.cpp", "core tests subtitle wrap probe runner implementation")
    require(core_tests_upp, "FfmpegSubtitleProbeCommandBuilder.cpp", "core tests subtitle probe command implementation")
    require(core_tests_upp, "VocalPart.cpp", "core tests vocal part implementation")
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
    for fixture in [
        "current-project-metadata.json",
        "legacy-unversioned-project-metadata.json",
        "unsupported-project-metadata.json",
        "invalid-project-metadata.json",
    ]:
        if not (root / "tests" / "upp" / "CroonCoreTests" / "fixtures" / fixture).exists():
            fail(f"core tests missing metadata fixture {fixture}")
        require(core_tests_cpp, fixture, f"core tests metadata fixture usage {fixture}")
    for rel, text in {
        "CroonCoreTestSupport.cpp": core_test_support_cpp,
    }.items():
        reject(text, "#include <Croon/Util.h>", f"{rel} compatibility facade include")

    ui_scaler_h = (root / "UiScaler.h").read_text()
    require(ui_scaler_h, "X(int value)", "UiScaler horizontal scale contract")
    require(ui_scaler_h, "Y(int value)", "UiScaler vertical scale contract")
    require(ui_scaler_h, "InverseX(int value)", "UiScaler inverse horizontal scale contract")
    require(ui_scaler_h, "InverseY(int value)", "UiScaler inverse vertical scale contract")

    ui_scaler_cpp = (root / "UiScaler.cpp").read_text()
    reject(ui_scaler_cpp, '#include "Croon.h"', "UiScaler app shell dependency")
    require(ui_scaler_cpp, "#include <CtrlLib/CtrlLib.h>", "UiScaler direct CtrlLib dependency")
    require(ui_scaler_cpp, '#include "UiScaler.h"', "UiScaler direct self dependency")
    require(ui_scaler_cpp, "return Zx(value)", "UiScaler horizontal U++ layout zoom delegation")
    require(ui_scaler_cpp, "return Zy(value)", "UiScaler vertical U++ layout zoom delegation")
    require(ui_scaler_cpp, "Ctrl::GetZoomRatio", "UiScaler inverse zoom-ratio calculation")
    require(ui_scaler_cpp, "return value * multiplier / divisor", "UiScaler inverse scaling calculation")

    vid_thumbnail_cpp = (root / "VidThumbnail.cpp").read_text()
    reject(vid_thumbnail_cpp, '#include "Croon.h"', "VidThumbnail app shell dependency")
    require(vid_thumbnail_cpp, "#include <CtrlLib/CtrlLib.h>", "VidThumbnail direct CtrlLib dependency")
    require(vid_thumbnail_cpp, '#include "Constants.h"', "VidThumbnail direct thumbnail constants dependency")
    require(vid_thumbnail_cpp, '#include "VidThumbnail.h"', "VidThumbnail direct self dependency")
    require(vid_thumbnail_cpp, "const int VidThumbnail::Width = ThumbnailDim", "VidThumbnail width contract")
    require(vid_thumbnail_cpp, "imgCtrl.SetImage(Rescale(image, sz))", "VidThumbnail rescale contract")

    if (root / "Util.cpp").exists():
        fail("Util.cpp compatibility facade still exists")
    if (root / "Util.h").exists():
        fail("Util.h compatibility facade still exists")
    croon_upp = (root / "Croon.upp").read_text()
    reject(croon_upp, "Util.cpp", "Croon.upp compatibility facade implementation")
    reject(croon_upp, "Util.h", "Croon.upp compatibility facade header")

    kar_data_cpp = (root / "KarData.cpp").read_text()
    reject(kar_data_cpp, '#include "Croon.h"', "KarData app shell dependency")
    require(kar_data_cpp, '#include "AppIdentity.h"', "KarData direct identity dependency")
    require(kar_data_cpp, '#include "ConfigService.h"\n#include "Config.h"', "KarData ordered config dependencies")
    require(kar_data_cpp, '#include "KarData.h"', "KarData direct self dependency")
    require(kar_data_cpp, '#include "ProjectSerializer.h"', "KarData direct serializer dependency")
    reject(kar_data_cpp, "KarData& GlobalKarData()", "KarData internal global accessor")
    reject(kar_data_cpp, "static KarData data", "KarData internal singleton storage")
    reject(kar_data_cpp, "KarData::GetGlobal()", "KarData global accessor implementation")
    reject(kar_data_cpp, "KarData _karData", "KarData exposed global storage")
    kar_data_h = (root / "KarData.h").read_text()
    reject(kar_data_h, "static KarData& GetGlobal()", "KarData global accessor declaration")
    require(kar_data_cpp, "ProjectSerializer::ToJson(*this)", "KarData serialization delegation")
    require(kar_data_cpp, "ProjectSerializer::FromJson(JSONStr)", "KarData deserialization delegation")

    project_serializer_cpp = (root / "ProjectSerializer.cpp").read_text()
    reject(project_serializer_cpp, '#include "Croon.h"', "ProjectSerializer app shell dependency")
    require(project_serializer_cpp, '#include "AppIdentity.h"', "ProjectSerializer direct identity dependency")
    require(project_serializer_cpp, '#include "KarData.h"', "ProjectSerializer direct data dependency")
    require(project_serializer_cpp, '#include "ProjectSerializer.h"', "ProjectSerializer direct self dependency")
    require(project_serializer_cpp, "bool ParseMetadataObject(const String& json, Value& js)", "ProjectSerializer shared metadata parse helper")
    require(project_serializer_cpp, "return !js.IsError() && js.Is<ValueMap>()", "ProjectSerializer parse helper object-shape contract")
    require(project_serializer_cpp, "String ProjectSerializer::ReadVersion(const String& json)", "ProjectSerializer direct version-read implementation")
    require(project_serializer_cpp, "if (!ParseMetadataObject(json, js)) return String::GetVoid()", "ProjectSerializer safe version-read invalid metadata contract")
    require(project_serializer_cpp, 'return NormalizeReadVersion(js.GetAdd("version"))', "ProjectSerializer direct version-read normalization")
    require(project_serializer_cpp, "ProjectSerializer::MetadataCompatibility ProjectSerializer::ReadCompatibility", "ProjectSerializer compatibility-status implementation")
    require(project_serializer_cpp, "catch (CParser::Error&)", "ProjectSerializer invalid JSON classification")
    require(project_serializer_cpp, "return InvalidMetadata", "ProjectSerializer invalid metadata status")
    require(project_serializer_cpp, "if (!ParseMetadataObject(json, js)) return InvalidMetadata", "ProjectSerializer metadata object-shape classification")
    require(project_serializer_cpp, "if (version.IsEmpty()) return LegacyUnversionedMetadata", "ProjectSerializer legacy compatibility classification")
    require(project_serializer_cpp, "return SupportsVersion(version) ? CurrentMetadata : UnsupportedMetadata", "ProjectSerializer explicit compatibility classification")
    require(project_serializer_cpp, "bool ProjectSerializer::SupportsJson(const String& json)", "ProjectSerializer JSON support implementation")
    require(project_serializer_cpp, "compatibility == CurrentMetadata || compatibility == LegacyUnversionedMetadata", "ProjectSerializer JSON support status contract")
    require(project_serializer_cpp, "String ProjectSerializer::CompatibilityLabel", "ProjectSerializer compatibility label implementation")
    for label in ['return "current"', 'return "legacy-unversioned"', 'return "unsupported"', 'return "invalid"']:
        require(project_serializer_cpp, label, "ProjectSerializer compatibility label contract")
    for key in [
        '"version"',
        '"title"',
        '"artist"',
        '"origVideoFile"',
        '"subtitleLines"',
        '"timedLyrics"',
        '"parts"',
    ]:
        require(project_serializer_cpp, key, "ProjectSerializer JSON contract")
    require(project_serializer_cpp, 'js("version", FormatVersion())', "ProjectSerializer save format-version stamping")
    require(project_serializer_cpp, 'version == "1.0"', "ProjectSerializer legacy 1.0 version normalization")
    require(project_serializer_cpp, 'data.version = NormalizeReadVersion(js.GetAdd("version"))', "ProjectSerializer read-version normalization")
    require(project_serializer_cpp, 'data.SetSubtitleLines(js.GetAdd("subtitleLines"))', "ProjectSerializer subtitle line-count hydration")
    require(project_serializer_cpp, "if (!ParseMetadataObject(json, js))", "ProjectSerializer invalid metadata hydration guard")
    require(project_serializer_cpp, "data.version = String::GetVoid()", "ProjectSerializer invalid metadata version marker")
    require(project_serializer_cpp, "if (data.year < 0) data.year = 0", "ProjectSerializer year normalization")

    project_serializer_h = (root / "ProjectSerializer.h").read_text()
    require(project_serializer_h, "FormatVersion()", "ProjectSerializer format-version contract")
    require(project_serializer_h, "NormalizeReadVersion", "ProjectSerializer legacy read-version normalization contract")
    require(project_serializer_h, "SupportsVersion", "ProjectSerializer version-support contract")
    require(project_serializer_h, "ReadVersion", "ProjectSerializer direct version-read contract")
    require(project_serializer_h, "MetadataCompatibility", "ProjectSerializer compatibility-status contract")
    require(project_serializer_h, "LegacyUnversionedMetadata", "ProjectSerializer legacy compatibility-status contract")
    require(project_serializer_h, "InvalidMetadata", "ProjectSerializer invalid compatibility-status contract")
    require(project_serializer_h, "CompatibilityLabel", "ProjectSerializer compatibility label contract")
    require(project_serializer_h, "SupportsJson", "ProjectSerializer JSON support contract")
    reject(project_serializer_h, "AppIdentity::Version()", "ProjectSerializer inline identity dependency")

    legacy_ext = ".mu" + "se"
    legacy_name = "Mu" + "se"
    if (root / "FfmpegCommandBuilder.h").exists():
        fail("obsolete FfmpegCommandBuilder compatibility facade still exists")

    croon_upp = (root / "Croon.upp").read_text()
    reject(croon_upp, "FfmpegCommandBuilder.h", "obsolete ffmpeg command builder facade package entry")

    ffmpeg_audio_h = (root / "FfmpegAudioCommandBuilder.h").read_text()
    ffmpeg_audio_cpp = (root / "FfmpegAudioCommandBuilder.cpp").read_text()
    require(ffmpeg_audio_h, "struct FfmpegAudioCommandBuilder", "ffmpeg audio builder declaration")
    require(ffmpeg_audio_h, "ConvertToVorbis(String audioPath, String outputPath)", "ffmpeg audio conversion declaration")
    require(ffmpeg_audio_h, "Dehiss(String audioPath, String outputPath, int dB=15)", "ffmpeg dehiss declaration")
    reject(ffmpeg_audio_h, '"libvorbis"', "ffmpeg audio inline conversion codec")
    require(ffmpeg_audio_cpp, '"libvorbis"', "ffmpeg audio conversion codec")
    require(ffmpeg_audio_cpp, 'Format("afftdn=nr=%d:nf=-45:bn=1", dB)', "ffmpeg dehiss filter contract")

    ffmpeg_export_h = (root / "FfmpegExportCommandBuilder.h").read_text()
    ffmpeg_export_cpp = (root / "FfmpegExportCommandBuilder.cpp").read_text()
    require(ffmpeg_export_h, "struct FfmpegExportCommandBuilder", "ffmpeg export builder declaration")
    require(ffmpeg_export_h, "WithBackgroundVideo(const KarData& data", "ffmpeg background export declaration")
    require(ffmpeg_export_h, "WithVisualization(const KarData& data", "ffmpeg visualization export declaration")
    require(ffmpeg_export_h, "GenerateCoverImage(String videoPath, String outputPath, double thumbnailTS)", "ffmpeg cover image declaration")
    reject(ffmpeg_export_h, "LyricsTransformer::TimedToRaw", "ffmpeg export inline lyrics metadata serialization")
    require(ffmpeg_export_cpp, '#include "LyricsTransformer.h"', "ffmpeg export lyrics transformer dependency")
    require(ffmpeg_export_cpp, '#include "TimeFormatter.h"', "ffmpeg export time formatter dependency")
    require(ffmpeg_export_cpp, '#include "Visualization.h"', "ffmpeg export visualization dependency")
    require(ffmpeg_export_cpp, "LyricsTransformer::TimedToRaw", "ffmpeg export lyrics metadata serialization")
    require(ffmpeg_export_cpp, "TimeFormatter::Clock", "ffmpeg export cover timestamp formatting")
    require(ffmpeg_export_cpp, '"libx265"', "ffmpeg export video codec")
    require(ffmpeg_export_cpp, "subtitles=%s[v]", "ffmpeg background subtitle filter")

    ffmpeg_project_h = (root / "FfmpegProjectCommandBuilder.h").read_text()
    ffmpeg_project_cpp = (root / "FfmpegProjectCommandBuilder.cpp").read_text()
    require(ffmpeg_project_h, "struct FfmpegProjectCommandBuilder", "ffmpeg project builder declaration")
    require(ffmpeg_project_h, "DumpAttachmentAndGenerateThumbnail(String projectPath", "ffmpeg project listing declaration")
    require(ffmpeg_project_h, "ExtractAudioAndInfo(String projectPath", "ffmpeg project audio extraction declaration")
    require(ffmpeg_project_h, "ExtractVideo(String projectPath", "ffmpeg project video extraction declaration")
    require(ffmpeg_project_h, "SaveWithBackgroundVideo(const KarData& data", "ffmpeg project background save declaration")
    require(ffmpeg_project_h, "SaveWithVisualization(const KarData& data", "ffmpeg project visualization save declaration")
    reject(ffmpeg_project_h, "AppIdentity::ProjectAttachmentMetadata()", "ffmpeg project inline attachment contract")
    require(ffmpeg_project_cpp, '#include "AppIdentity.h"', "ffmpeg project identity dependency")
    require(ffmpeg_project_cpp, '#include "KarData.h"', "ffmpeg project data dependency")
    require(ffmpeg_project_cpp, "AppIdentity::ProjectAttachmentMetadata()", "project attachment contract")
    require(ffmpeg_project_cpp, "AppIdentity::ProductName()", "project metadata contract")
    require(ffmpeg_project_cpp, '"00:00:01"', "ffmpeg project listing thumbnail seek contract")
    reject(ffmpeg_project_h, "filename=" + legacy_ext[1:] + ".info", "ffmpeg metadata contract")

    ffmpeg_subtitle_probe_h = (root / "FfmpegSubtitleProbeCommandBuilder.h").read_text()
    ffmpeg_subtitle_probe_cpp = (root / "FfmpegSubtitleProbeCommandBuilder.cpp").read_text()
    require(ffmpeg_subtitle_probe_h, "struct FfmpegSubtitleProbeCommandBuilder", "ffmpeg subtitle probe builder declaration")
    require(ffmpeg_subtitle_probe_h, "RenderRgba", "ffmpeg subtitle probe raw RGBA render declaration")
    reject(ffmpeg_subtitle_probe_cpp, '#include "Croon.h"', "ffmpeg subtitle probe builder app shell dependency")
    require(ffmpeg_subtitle_probe_cpp, "color=c=black@0.0:s=%dx%d:r=1", "ffmpeg subtitle probe transparent source")
    require(ffmpeg_subtitle_probe_cpp, "subtitles=%s,crop=%d:%d:0:%d", "ffmpeg subtitle probe subtitles crop filter")
    require(ffmpeg_subtitle_probe_cpp, '"rgba"', "ffmpeg subtitle probe RGBA pixel format")
    require(ffmpeg_subtitle_probe_cpp, '"rawvideo"', "ffmpeg subtitle probe rawvideo output")

    ffmpeg_thumbnail_h = (root / "FfmpegThumbnailCommandBuilder.h").read_text()
    ffmpeg_thumbnail_cpp = (root / "FfmpegThumbnailCommandBuilder.cpp").read_text()
    require(ffmpeg_thumbnail_h, "struct FfmpegThumbnailCommandBuilder", "ffmpeg thumbnail builder declaration")
    require(ffmpeg_thumbnail_h, "Generate(String videoPath, String outputPath, int width, int height)", "ffmpeg thumbnail builder generate contract")
    reject(ffmpeg_thumbnail_h, '"00:00:06"', "ffmpeg thumbnail inline seek contract")
    require(ffmpeg_thumbnail_cpp, '"00:00:06"', "ffmpeg thumbnail seek contract")
    require(ffmpeg_thumbnail_cpp, '"-vframes"', "ffmpeg thumbnail frame contract")
    require(ffmpeg_thumbnail_cpp, "Format(\"crop='min(iw,ih)':'min(iw,ih)',scale=%d:%d\"", "ffmpeg thumbnail crop-scale contract")
    require(ffmpeg_thumbnail_cpp, "outputPath", "ffmpeg thumbnail output path contract")

    ffmpeg_progress_parser_h = (root / "FfmpegProgressParser.h").read_text()
    require(ffmpeg_progress_parser_h, "ParseTimestamp", "ffmpeg progress parser contract")

    ffmpeg_progress_parser_cpp = (root / "FfmpegProgressParser.cpp").read_text()
    reject(ffmpeg_progress_parser_cpp, '#include "Croon.h"', "ffmpeg progress parser app shell dependency")
    require(ffmpeg_progress_parser_cpp, '#include "FfmpegProgressParser.h"', "ffmpeg progress parser direct self dependency")
    require(ffmpeg_progress_parser_cpp, "output.Find(key)", "ffmpeg progress key lookup")
    require(ffmpeg_progress_parser_cpp, "Split(tsStr, ':')", "ffmpeg progress timestamp split")
    require(ffmpeg_progress_parser_cpp, "formatted = output.Mid", "ffmpeg progress formatted timestamp")

    genre_catalog_h = (root / "GenreCatalog.h").read_text()
    require(genre_catalog_h, "List()", "GenreCatalog list contract")

    genre_catalog_cpp = (root / "GenreCatalog.cpp").read_text()
    reject(genre_catalog_cpp, '#include "Croon.h"', "GenreCatalog app shell dependency")
    require(genre_catalog_cpp, '#include "GenreCatalog.h"', "GenreCatalog direct self dependency")
    for genre in ["Ballad", "OPM", "Rock n Roll", "Soft Rock"]:
        require(genre_catalog_cpp, f'"{genre}"', "GenreCatalog reference data")

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
        "OpenProjectDlg.cpp": ["LyricsTransformer::TimedToRaw"],
        "Page2.cpp": ["LyricsDownloadService::Download", "TextTools::CleanSpacing"],
        "Page3.cpp": ["LyricsTransformer::RawToUntimed"],
        "Project.cpp": ["LyricsTransformer::RawToUntimed", "LrcGenerator::ToLrc"],
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
        "Page1.cpp": ["GenreCatalog::List"],
        "Page3.cpp": ["VideoCatalog::FindCachedThumbnails"],
        "Project.cpp": ["GenreCatalog::List"],
        "VideoCatalog.cpp": ["AppPaths::DataDirectory", "AppPaths::FindFiles", "TextTools::ShortenMiddle"],
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
    for rel in ["ListCtrl.cpp", "Page3.cpp", "ProjectList.h"]:
        text = (root / rel).read_text()
        require(text, "UiScaler::Inverse", f"{rel} inverse UI scaler dependency")
    for rel in ["ListCtrl.cpp", "ListCtrl.h", "LyricsPartsDlg.cpp", "MainWindow.cpp", "OpenProjectDlg.cpp", "ProjectList.h", "SaveProjectDlg.cpp", "TimingDlg.cpp", "TimingCtrl.cpp", "VideoDlg.cpp"]:
        text = (root / rel).read_text()
        reject(text, "Zx(", f"{rel} raw horizontal scaler dependency")
        reject(text, "Zy(", f"{rel} raw vertical scaler dependency")

    project_cpp = (root / "Project.cpp").read_text()
    reject(project_cpp, "Project::Project() : Project(KarData::GetGlobal())", "Project default global data wiring")
    reject(project_cpp, "VideoDlg& CompatibilityVideoDlg()", "Project compatibility video helper")
    reject(project_cpp, "Project::Project(KarData& projectData) : Project(projectData, CompatibilityVideoDlg())", "Project compatibility video wiring")
    require(project_cpp, "Project::Project(KarData& projectData, VideoDlg& videoDialog) : videoPath(\"\"), data(projectData), videoDlg(videoDialog)", "Project injected video constructor")
    require(project_cpp, "videoDlg.Run()", "Project injected video dialog workflow")
    reject(project_cpp, "SubtitleGenerator::ToRichAss(data)", "Project live ASS preview refresh")
    reject(project_cpp, "previewRT", "Project live ASS preview control")
    require(project_cpp, "SaveProjectDlg().Run(data) == IDOK", "Project save clears dirty only after successful save")
    require(project_cpp, "saveDlg.Run(savePath, data)", "Project injected data save-as")
    require(project_cpp, "LyricsTransformer::RawToUntimed(data)", "Project injected data lyrics update")
    require(project_cpp, "expDlg.Run(data, outputPath, length)", "Project injected data export")
    require(project_cpp, "void Project::ExportLrc()", "Project LRC export implementation")
    require(project_cpp, "Config::Get(LRC_EXPORT_DIR)", "Project LRC export directory lookup")
    require(project_cpp, "Config::Set(LRC_EXPORT_DIR, GetFileDirectory(outputPath))", "Project LRC export directory persistence")
    require(project_cpp, "fsel.Type(\"LRC Lyrics (*.lrc)\", \"*.lrc\")", "Project LRC save type")
    require(project_cpp, "SaveFile(outputPath, LrcGenerator::ToLrc(data))", "Project LRC file write")
    require(project_cpp, "sub.Add(\"LRC File...\"", "Project LRC menu entry")
    require(project_cpp, "SetDirty(dirty || tDlg.IsDirty())", "Project timing preserves existing dirty state")
    require(project_cpp, "data.origAudioFilePath = ~fsel;\n            data.duration = conDlg.GetDurationn();\n            SetDirty();",
            "Project replacement audio marks project dirty")
    require(project_cpp, "Project::~Project()", "Project destructor implementation")
    require(project_cpp, "Project::~Project() {\n    CleanUp();\n}", "Project destructor cleanup")
    reject(project_cpp, "auto& data = KarData::GetGlobal()", "Project direct global data alias")
    reject(project_cpp, "KarData::GetGlobal().", "Project direct global field access")
    reject(project_cpp, "SaveProjectDlg().Run(KarData::GetGlobal()", "Project direct global save access")
    reject(project_cpp, "SubtitleGenerator::ToRichAss(KarData::GetGlobal()", "Project direct global preview access")
    project_h = (root / "Project.h").read_text()
    reject(project_h, "RichTextCtrl previewRT;", "Project live ASS preview member")
    reject(project_h, "    Project();", "Project default global data declaration")
    reject(project_h, "    Project(KarData& data);", "Project compatibility video declaration")
    reject(project_h, "~Project() { CleanUp(); }", "Project inline cleanup destructor")
    require(project_h, "Project(KarData& data, VideoDlg& videoDlg)", "Project injected video declaration")
    require(project_h, "void ExportLrc();", "Project LRC export declaration")
    require(project_h, "virtual ~Project();", "Project destructor declaration")
    decisions_md = (root / "decisions.md").read_text()
    require(decisions_md, "### Disable Live RichText ASS Preview", "disabled live ASS preview decision")
    require(decisions_md, "future lightweight preview can reuse the tab area for LRC-formatted lyrics", "future LRC preview decision")
    require(project_h, "KarData& data", "Project injected data member")
    require(project_h, "VideoDlg& videoDlg", "Project injected video member")

    video_dlg_cpp = (root / "VideoDlg.cpp").read_text()
    reject(video_dlg_cpp, "VideoDlg::VideoDlg() : VideoDlg(KarData::GetGlobal())", "VideoDlg default global data wiring")
    require(video_dlg_cpp, "VideoDlg::VideoDlg(KarData& data) : page3(data)", "VideoDlg injected data constructor")
    video_dlg_h = (root / "VideoDlg.h").read_text()
    reject(video_dlg_h, "    VideoDlg();", "VideoDlg default global data declaration")
    require(video_dlg_h, "VideoDlg(KarData& data)", "VideoDlg injected data declaration")

    main_window_cpp = (root / "MainWindow.cpp").read_text()
    reject(main_window_cpp, "MainWindow::MainWindow() : MainWindow(KarData::GetGlobal())", "MainWindow default global data wiring")
    require(main_window_cpp, "MainWindow::MainWindow(KarData& data) : videoDlg(data), wizardDlg(data), project(data, videoDlg), projects(data, wizardDlg)", "MainWindow injected data constructor")
    require(main_window_cpp, "Add(projects.HSizePos().VSizePos())", "MainWindow runtime project-list mount")
    require(main_window_cpp, "Add(project.HSizePos().VSizePos())", "MainWindow runtime project mount")
    require(main_window_cpp, "project.Populate()", "MainWindow project load workflow")
    require(main_window_cpp, "projects.NewProject()", "MainWindow project-list workflow")
    main_window_h = (root / "MainWindow.h").read_text()
    require(main_window_h, "MainWindow(KarData& data)", "MainWindow injected data declaration")
    reject(main_window_h, "    MainWindow();", "MainWindow default global data declaration")
    reject(main_window_h, "KarData& data;", "MainWindow unused data member")
    require(main_window_h, "VideoDlg videoDlg", "MainWindow owned VideoDlg member")
    require(main_window_h, "WizardDlg wizardDlg", "MainWindow owned WizardDlg member")
    require(main_window_h, "Project project", "MainWindow owned Project member")
    require(main_window_h, "ProjectList projects", "MainWindow owned ProjectList member")
    main_window_lay = (root / "CroonMainWindow.lay").read_text()
    reject(main_window_lay, "ITEM(ProjectList, projects", "MainWindow layout default ProjectList construction")
    reject(main_window_lay, "ITEM(Project, project", "MainWindow layout default Project construction")

    page1_cpp = (root / "Page1.cpp").read_text()
    reject(page1_cpp, '#include "Croon.h"', "Page1 app shell dependency")
    require(page1_cpp, '#include "KarData.h"', "Page1 direct data dependency")
    require(page1_cpp, '#include "GenreCatalog.h"', "Page1 direct genre catalog dependency")
    require(page1_cpp, "GenreCatalog::List()", "Page1 genre list contract")
    reject(page1_cpp, "Page1::Page1() : Page1(KarData::GetGlobal())", "Page1 default global data wiring")
    require(page1_cpp, "Page1::Page1(KarData& data) : data(data)", "Page1 injected data constructor")
    require(page1_cpp, "titleEd.SetData(data.title)", "Page1 injected data population")
    reject(page1_cpp, "auto& data = KarData::GetGlobal()", "Page1 direct global data access")
    page1_h = (root / "Page1.h").read_text()
    reject(page1_h, "    Page1();", "Page1 default global data declaration")
    require(page1_h, "Page1(KarData& data)", "Page1 injected data declaration")
    require(page1_h, "KarData& data", "Page1 injected data member")

    page2_cpp = (root / "Page2.cpp").read_text()
    reject(page2_cpp, '#include "Croon.h"', "Page2 app shell dependency")
    require(page2_cpp, '#include "ConfigService.h"\n#include "Config.h"', "Page2 ordered config dependency")
    require(page2_cpp, '#include "KarData.h"', "Page2 direct data dependency")
    require(page2_cpp, '#include "LyricsDownloadService.h"', "Page2 direct download dependency")
    require(page2_cpp, '#include "TextTools.h"', "Page2 direct text helper dependency")
    reject(page2_cpp, "Page2::Page2() : Page2(KarData::GetGlobal())", "Page2 default global data wiring")
    require(page2_cpp, "Page2::Page2(KarData& data)", "Page2 injected data constructor")
    require(page2_cpp, "data.timedLyrics.IsEmpty()", "Page2 injected timed lyrics access")
    require(page2_cpp, "data.rawLyrics", "Page2 injected raw lyrics access")
    reject(page2_cpp, "auto& karData = KarData::GetGlobal()", "Page2 direct global data access")
    require(page2_cpp, "LyricsDownloadService::DownloadWithStatus", "Page2 lyric download status contract")
    require(page2_cpp, "LyricsDownloadService::DownloadOk", "Page2 lyric download success contract")
    require(page2_cpp, "LyricsDownloadService::ExtractionFailed", "Page2 lyric extraction failure contract")
    require(page2_cpp, "Downloaded lyrics could not be read", "Page2 lyric extraction failure message")
    require(page2_cpp, "TextTools::CleanSpacing", "Page2 lyric cleanup contract")
    require(page2_cpp, "Config::Get(LYRICS_PREFIX)", "Page2 lyrics prefix contract")
    page2_h = (root / "Page2.h").read_text()
    reject(page2_h, "    Page2();", "Page2 default global data declaration")
    require(page2_h, "Page2(KarData& data)", "Page2 injected data declaration")
    require(page2_h, "KarData& data", "Page2 injected data member")

    page3_cpp = (root / "Page3.cpp").read_text()
    reject(page3_cpp, "Page3::Page3(String gatherKey) : Page3(KarData::GetGlobal(), gatherKey)", "Page3 default global data wiring")
    reject(page3_cpp, "GatherDlg& CompatibilityGatherDlg()", "Page3 compatibility gather helper")
    reject(page3_cpp, "Page3::Page3(KarData& data, String gatherKey) : Page3(data, CompatibilityGatherDlg(), gatherKey)", "Page3 compatibility gather wiring")
    require(page3_cpp, "Page3::Page3(KarData& data, String gatherKey)", "Page3 injected data constructor")
    require(page3_cpp, "Page3::~Page3()", "Page3 stops incremental gather on destruction")
    require(page3_cpp, "void Page3::StopGathering()", "Page3 exposes incremental gather stop")
    require(page3_cpp, "StartGather(~fsel)", "Page3 incremental gather entry point")
    require(page3_cpp, "VideoCatalog::FindVideoFiles(videoDir)", "Page3 incremental video discovery")
    require(page3_cpp, "VideoCatalog::BuildThumbnailCommand(path)", "Page3 incremental thumbnail command")
    require(page3_cpp, "gatherProcess.Start(ffmpeg, args, nullptr, nullptr)", "Page3 incremental thumbnail process")
    require(page3_cpp, "PollGatherProcess()", "Page3 incremental thumbnail polling")
    require(page3_cpp, "AddGatheredVideo(gatherPaths[gatherIndex])", "Page3 incremental list population")
    require(page3_cpp, "AddVideoItem(&videoLst, path, tnPath, img, &vizLst)", "Page3 adds discovered videos directly")
    reject(page3_cpp, "gatherDlg.WhenVideoAdded", "Page3 no longer waits on GatherDlg events")
    reject(page3_cpp, "gatherDlg.Run(~fsel)", "Page3 no longer opens GatherDlg while picking videos")
    require(page3_cpp, "LyricsTransformer::RawToUntimed(this->data)", "Page3 injected data timing conversion")
    require(page3_cpp, "VideoCatalog::FindCachedThumbnails(videoDir)", "Page3 cached video listing service")
    require(page3_cpp, "cachedVideos[i].videoPath", "Page3 cached video path binding")
    require(page3_cpp, "cachedVideos[i].thumbnailPath", "Page3 cached thumbnail path binding")
    require(page3_cpp, "cachedVideos[i].thumbnail", "Page3 cached thumbnail image binding")
    reject(page3_cpp, "VideoCatalog::ThumbnailPath(paths[i])", "Page3 raw thumbnail path dependency")
    reject(page3_cpp, "VideoCatalog::LoadThumbnail(paths[i])", "Page3 raw thumbnail loading dependency")
    reject(page3_cpp, "AppPaths::DataDirectory()", "Page3 raw thumbnail directory dependency")
    reject(page3_cpp, "StreamRaster::LoadFileAny(tnPath)", "Page3 raw thumbnail loading dependency")
    require(page3_cpp, "saveDlg.Run(savePath, this->data)", "Page3 injected data save contract")
    require(page3_cpp, "this->data.videoFilePath = path", "Page3 injected video path write")
    require(page3_cpp, "enableNext = !data.videoFilePath.IsEmpty()", "Page3 injected data population")
    reject(page3_cpp, "auto& data = KarData::GetGlobal()", "Page3 direct global data access")
    reject(page3_cpp, "SaveProjectDlg().Run(KarData::GetGlobal()", "Page3 direct global save access")
    page3_h = (root / "Page3.h").read_text()
    reject(page3_h, "    Page3(String gatherKey", "Page3 default global data declaration")
    require(page3_h, "Page3(KarData& data, String gatherKey", "Page3 injected data declaration")
    require(page3_h, "virtual ~Page3();", "Page3 destructor declaration")
    require(page3_h, "void StopGathering();", "Page3 gather stop declaration")
    require(page3_h, "KarData& data", "Page3 injected data member")
    require(page3_h, "MediaProcessRunner gatherProcess", "Page3 owns incremental gather process")
    require(page3_h, "Vector<String> gatherPaths", "Page3 owns incremental gather paths")
    reject(page3_h, "GatherDlg& gatherDlg", "Page3 no longer owns GatherDlg reference")

    video_catalog_h = (root / "VideoCatalog.h").read_text()
    require(video_catalog_h, "struct VideoCatalogItem", "VideoCatalog item declaration")
    require(video_catalog_h, "String videoPath", "VideoCatalog item video path")
    require(video_catalog_h, "String thumbnailPath", "VideoCatalog item thumbnail path")
    require(video_catalog_h, "Image thumbnail", "VideoCatalog item thumbnail image")
    require(video_catalog_h, "FindVideoFiles(String videoDir)", "VideoCatalog discovery declaration")
    require(video_catalog_h, "FindCachedThumbnails(String videoDir)", "VideoCatalog cached thumbnail declaration")
    require(video_catalog_h, "ThumbnailPath(String videoPath)", "VideoCatalog thumbnail path declaration")
    require(video_catalog_h, "HasThumbnail(String videoPath)", "VideoCatalog thumbnail existence declaration")
    require(video_catalog_h, "LoadThumbnail(String videoPath)", "VideoCatalog thumbnail loading declaration")
    require(video_catalog_h, "DeleteThumbnail(String videoPath)", "VideoCatalog thumbnail deletion declaration")
    require(video_catalog_h, "BuildThumbnailCommand(String videoPath)", "VideoCatalog thumbnail command declaration")
    require(video_catalog_h, "DisplayName(String videoPath, int maxLength)", "VideoCatalog display name declaration")
    video_catalog_cpp = (root / "VideoCatalog.cpp").read_text()
    reject(video_catalog_cpp, '#include "Croon.h"', "VideoCatalog app shell dependency")
    require(video_catalog_cpp, "#include <Draw/Draw.h>", "VideoCatalog image loading dependency")
    require(video_catalog_cpp, '#include "Constants.h"\n#include "AppPaths.h"\n#include "FfmpegThumbnailCommandBuilder.h"\n#include "TextTools.h"\n#include "VideoCatalog.h"', "VideoCatalog direct dependencies")
    require(video_catalog_cpp, 'AppPaths::FindFiles(videoDir, "*.mp4")', "VideoCatalog mp4 discovery contract")
    require(video_catalog_cpp, "Vector<VideoCatalogItem> VideoCatalog::FindCachedThumbnails(String videoDir)", "VideoCatalog cached thumbnail implementation")
    require(video_catalog_cpp, "Vector<String> paths = FindVideoFiles(videoDir)", "VideoCatalog cached listing discovery")
    require(video_catalog_cpp, "Image thumbnail = LoadThumbnail(paths[i])", "VideoCatalog cached listing thumbnail load")
    require(video_catalog_cpp, "item.videoPath = paths[i]", "VideoCatalog cached listing video path")
    require(video_catalog_cpp, "item.thumbnailPath = ThumbnailPath(paths[i])", "VideoCatalog cached listing thumbnail path")
    require(video_catalog_cpp, "item.thumbnail = thumbnail", "VideoCatalog cached listing thumbnail image")
    require(video_catalog_cpp, "AppendFileName(AppPaths::DataDirectory(), GetFileName(videoPath))", "VideoCatalog thumbnail data path contract")
    require(video_catalog_cpp, 'tnPath.Replace(".mp4", ".thumbnail.png")', "VideoCatalog thumbnail extension contract")
    require(video_catalog_cpp, "FileExists(ThumbnailPath(videoPath))", "VideoCatalog thumbnail existence contract")
    require(video_catalog_cpp, "if (!HasThumbnail(videoPath)) return Image()", "VideoCatalog thumbnail loading existence gate")
    require(video_catalog_cpp, "StreamRaster::LoadFileAny(tnPath)", "VideoCatalog thumbnail loading contract")
    require(video_catalog_cpp, "FileDelete(ThumbnailPath(videoPath))", "VideoCatalog thumbnail deletion contract")
    require(video_catalog_cpp, "Vector<String> VideoCatalog::BuildThumbnailCommand(String videoPath)", "VideoCatalog thumbnail command implementation")
    require(video_catalog_cpp, "FfmpegThumbnailCommandBuilder::Generate(videoPath, ThumbnailPath(videoPath), ThumbnailDim, ThumbnailDim)", "VideoCatalog thumbnail command delegation")
    require(video_catalog_cpp, "String VideoCatalog::DisplayName(String videoPath, int maxLength)", "VideoCatalog display name implementation")
    require(video_catalog_cpp, "TextTools::ShortenMiddle(videoPath, maxLength)", "VideoCatalog display name formatting")

    for retired in ["GatherDlg.cpp", "GatherDlg.h"]:
        if (root / retired).exists():
            fail(f"{retired} should remain retired")

    wizard_cpp = (root / "WizardDlg.cpp").read_text()
    reject(wizard_cpp, "WizardDlg::WizardDlg() : WizardDlg(KarData::GetGlobal())", "WizardDlg default global data wiring")
    require(wizard_cpp, "WizardDlg::WizardDlg(KarData& data) : data(data), page1(data), page2(data), page3(data)", "WizardDlg injected data constructor")
    require(wizard_cpp, "Add(page->HSizePosZ(5, 5).VSizePosZ(32, 45))", "WizardDlg runtime page mount")
    require(wizard_cpp, "page3.StopGathering();\n    TopWindow::Close();", "WizardDlg stops incremental gather after run")
    require(wizard_cpp, "data.Reset()", "WizardDlg injected data reset")
    reject(wizard_cpp, "auto& data = KarData::GetGlobal()", "WizardDlg direct global data access")
    wizard_h = (root / "WizardDlg.h").read_text()
    reject(wizard_h, "    WizardDlg();", "WizardDlg default global data declaration")
    require(wizard_h, "WizardDlg(KarData& data)", "WizardDlg injected data declaration")
    require(wizard_h, "KarData& data", "WizardDlg injected data member")
    require(wizard_h, "Page1 page1;", "WizardDlg owned Page1 member")
    require(wizard_h, "Page2 page2;", "WizardDlg owned Page2 member")
    reject(wizard_h, "GatherDlg gatherDlg;", "WizardDlg no longer owns GatherDlg")
    require(wizard_h, "Page3 page3;", "WizardDlg owned Page3 member")

    project_list_cpp = (root / "ProjectList.cpp").read_text()
    reject(project_list_cpp, "ProjectList::ProjectList() : ProjectList(KarData::GetGlobal())", "ProjectList default global data wiring")
    reject(project_list_cpp, "WizardDlg& CompatibilityWizardDlg()", "ProjectList compatibility wizard helper")
    reject(project_list_cpp, "ProjectList::ProjectList(KarData& data) : ProjectList(data, CompatibilityWizardDlg())", "ProjectList compatibility wizard wiring")
    require(project_list_cpp, "ProjectList::ProjectList(KarData& data, WizardDlg& wizardDlg) : data(data), wizardDlg(wizardDlg)", "ProjectList injected wizard constructor")
    require(project_list_cpp, "InstallListHandlers();", "ProjectList installs handlers before background load completes")
    require(project_list_cpp, "AddLoadedProject(path, title, artist, lyrics, thumbnail)", "ProjectList incremental project insertion")
    require(project_list_cpp, "projectLst.AddChild(*(new ProjectItemCtrl(projects.back())))", "ProjectList streamed items consume mouse events immediately")
    reject(project_list_cpp, "projectLst.AddChild(*(new ProjectItemCtrl(projects.back())), false)", "ProjectList streamed items waiting for loader completion")
    require(project_list_cpp, "loader.StopLoading();", "ProjectList can stop background loading")
    require(project_list_cpp, "loader.ProjectPaths()", "ProjectList preserves unloaded recent paths")
    require(project_list_cpp, "ContainsProject(path)", "ProjectList avoids duplicate background items")
    require(project_list_cpp, "recentProjectsCleared", "ProjectList preserves explicit clear action")
    require(project_list_cpp, "removedProjectPaths", "ProjectList preserves explicit delete action")
    require(project_list_cpp, "data = pick(tdata)", "ProjectList injected data load")
    require(project_list_cpp, "wizardDlg.Run(conDlg.GetConvertedFile()", "ProjectList injected wizard workflow")
    require(project_list_cpp, "AppAudioPlayer::Open(data.audioFilePath)", "ProjectList injected data audio open")
    reject(project_list_cpp, "AppAudioPlayer::GetPlayer()", "ProjectList audio singleton reach-through")
    require(project_list_cpp, "LyricsTransformer::TimedToRaw(data.timedLyrics, true)", "ProjectList injected data list refresh")
    reject(project_list_cpp, "KarData& data = KarData::GetGlobal()", "ProjectList direct global data alias")
    reject(project_list_cpp, "KarData::GetGlobal().audioFilePath", "ProjectList direct global audio access")
    project_list_h = (root / "ProjectList.h").read_text()
    reject(project_list_h, "    ProjectList();", "ProjectList default global data declaration")
    reject(project_list_h, "    ProjectList(KarData& data);", "ProjectList compatibility wizard declaration")
    require(project_list_h, "ProjectList(KarData& data, WizardDlg& wizardDlg)", "ProjectList injected wizard declaration")
    require(project_list_h, "void InstallListHandlers();", "ProjectList handler helper declaration")
    require(project_list_h, "void AddLoadedProject", "ProjectList incremental add helper declaration")
    require(project_list_h, "Vector<String> removedProjectPaths", "ProjectList deleted path tracking")
    require(project_list_h, "bool recentProjectsCleared", "ProjectList clear state tracking")
    require(project_list_h, "KarData& data", "ProjectList injected data member")
    require(project_list_h, "WizardDlg& wizardDlg", "ProjectList injected wizard member")

    list_ctrl_cpp = (root / "ListCtrl.cpp").read_text()
    list_ctrl_h = (root / "ListCtrl.h").read_text()
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
        "ListCtrl::ItemWidth",
        "OverLayCtrl::MouseEvent",
    ]:
        require(list_ctrl_cpp, needle, "ListCtrl behavior contract")
    for needle in ["UiScaler::", "scrollBar.Wheel", "return ParentCtrl::MouseEvent", "focusFrame.SetColor"]:
        reject(list_ctrl_h, needle, "ListCtrl inline behavior")

    lyrics_parts_ctrl_cpp = (root / "LyricsPartsCtrl.cpp").read_text()
    lyrics_parts_ctrl_h = (root / "LyricsPartsCtrl.h").read_text()
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
        "LyricsPartsCtrl::SetLyricsAndParts",
        "LyricsPartsCtrl::ToggleAt",
    ]:
        require(lyrics_parts_ctrl_cpp, needle, "LyricsPartsCtrl paint contract")
    for needle in ["sb.WhenScroll", "TrimBoth(lines[row])", "ToggleAt(p)", "SetCapture()"]:
        reject(lyrics_parts_ctrl_h, needle, "LyricsPartsCtrl inline behavior")

    lyrics_editor_cpp = (root / "LyricsEditor.cpp").read_text()
    reject(lyrics_editor_cpp, '#include "Croon.h"', "LyricsEditor app shell dependency")
    require(lyrics_editor_cpp, '#include "KarData.h"', "LyricsEditor direct data dependency")
    reject(lyrics_editor_cpp, "LyricsEditor::LyricsEditor() : LyricsEditor(KarData::GetGlobal())", "LyricsEditor default global data wiring")
    require(lyrics_editor_cpp, "LyricsEditor::LyricsEditor(KarData& data) : data(data)", "LyricsEditor injected data constructor")
    require(lyrics_editor_cpp, "for (auto& tl : data.timedLyrics)", "LyricsEditor injected timed lyrics read")
    reject(lyrics_editor_cpp, "auto& data = KarData::GetGlobal()", "LyricsEditor direct global data access")
    lyrics_editor_h = (root / "LyricsEditor.h").read_text()
    reject(lyrics_editor_h, "    LyricsEditor();", "LyricsEditor default global data declaration")
    require(lyrics_editor_h, "LyricsEditor(KarData& data)", "LyricsEditor injected data declaration")
    require(lyrics_editor_h, "KarData& data", "LyricsEditor injected data member")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#include <GridCtrl/GridCtrl.h>",
        '#include "TimeFormatter.h"',
        '#include "LyricsEditor.h"',
    ]:
        require(lyrics_editor_cpp, needle, "LyricsEditor direct dependency")
    for needle in [
        "lineEd.WhenEnter = Proxy(WhenEnter)",
        "lyricsList.AddColumn",
        "TimeFormatter::Format(tl.time)",
        "editor.CancelSelection()",
    ]:
        require(lyrics_editor_cpp, needle, "LyricsEditor behavior contract")

    timing_ctrl_cpp = (root / "TimingCtrl.cpp").read_text()
    reject(timing_ctrl_cpp, '#include "Croon.h"', "TimingCtrl app shell dependency")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        '#include "Constants.h"',
        '#include "KarData.h"',
        '#include "UiScaler.h"',
        '#include "VocalPart.h"',
        '#include "TimingLine.h"',
        '#include "TimingCtrl.h"',
    ]:
        require(timing_ctrl_cpp, needle, "TimingCtrl direct dependency")
    for needle in [
        "MaxLineLength",
        "TimingLine(\"(INTRO)\"",
        "WhenTiming(position)",
        "ScrollToLineAndCenter",
        "Vector<TimeLyrics> TimingCtrl::GetTimedLyrics",
        "Vector<Tuple<int, bool, bool, bool>> TimingCtrl::GetParts",
        "void TimingCtrl::SetParts",
        "void TimingCtrl::SetTimedLyrics",
        "void TimingCtrl::Adjust",
        "void TimingCtrl::PlayBackDone",
    ]:
        require(timing_ctrl_cpp, needle, "TimingCtrl timing behavior contract")

    main_cpp = (root / "main.cpp").read_text()
    reject(main_cpp, '#include "Croon.h"', "main app shell dependency")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "void RunCroon();",
        "GUI_APP_MAIN",
        "RunCroon();",
    ]:
        require(main_cpp, needle, "main entry-point contract")

    timing_line_cpp = (root / "TimingLine.cpp").read_text()
    reject(timing_line_cpp, '#include "Croon.h"', "TimingLine app shell dependency")
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
        '#include "TimeFormatter.h"',
        '#include "VocalPart.h"',
        '#include "TimingLine.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "TimingLineDlg.h"',
    ]:
        require(timing_line_cpp, needle, "TimingLine direct dependency")
    for needle in [
        "LyricsTransformer::SplitDecorations",
        "PromptYesNoCancel",
        "TimingLineDlg tlDlg",
        "TimeFormatter::Format(position)",
        "frame.SetColor",
        "WhenVocalPartChanged",
        "WhenTimeButtonsDisabled()",
    ]:
        require(timing_line_cpp, needle, "TimingLine behavior contract")


if __name__ == "__main__":
    main()
