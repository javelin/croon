/*
 * File  : Croon.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

#include <atomic>
#include <ctime>
#include <filesystem>
#ifdef PLATFORM_POSIX
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#else
#include <SDL.h>
#include <SDL_mixer.h>
#endif

using namespace Upp;

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_header.h>

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_source.h>

#include "Constants.h"
#include "AppIdentity.h"
#include "AppPaths.h"
#include "ConfigService.h"
#include "Config.h"
#include "KarData.h"
#include "LyricsTransformer.h"
#include "SubtitleGenerator.h"
#include "TimeFormatter.h"
#include "UiScaler.h"
#include "GenreCatalog.h"
#include "RichTextBuilder.h"
#include "TextTools.h"
#include "Visualization.h"
#include "FfmpegProgressParser.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ListCtrl.h"
#include "LyricsPartsCtrl.h"
#include "ProjectLoader.h"
#include "Page.h"
#include "LyricsDownloadService.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "ProgressDlg.h"
#include "ConvertDlg.h"
#include "OpenProjectDlg.h"
#include "SaveProjectDlg.h"
#include "ExportDlg.h"
#include "LyricsPartsDlg.h"
#include "TimingLine.h"
#include "TimingCtrl.h"

#define LAYOUTFILE <Croon/CroonTimingDlg.lay>
#include <CtrlCore/lay.h>

#include "AudioPlayerBase.h"
#include "AudioPlayer.h"
#include "SDLMixerAudioPlayer.h"
#include "AppAudioPlayer.h"
#include "TimingDlg.h"
#include "GatherDlg.h"
#include "VidThumbnail.h"
#include "Page1.h"
#include "Page2.h"
#include "Page3.h"

#define LAYOUTFILE <Croon/CroonWizardShell.lay>
#include <CtrlCore/lay.h>

#define LAYOUTFILE <Croon/CroonVideoDlg.lay>
#include <CtrlCore/lay.h>

#include "VideoDlg.h"
#include "WizardDlg.h"
#include "Project.h"
#include "ProjectList.h"

#define LAYOUTFILE <Croon/CroonMainWindow.lay>
#include <CtrlCore/lay.h>

#include "MainWindow.h"

void RunCroon() {
    auto ffmpegLoc = Config::Get(FFMPEG_LOCATION);
    if (ffmpegLoc.IsVoid() || ffmpegLoc.IsEmpty()) ffmpegLoc = "ffmpeg";
    MediaProcessRunner proc;
    if (!proc.Start(ffmpegLoc)) {
        if (PromptYesNoCancel("Unable to validate ffmpeg executable. Manually look for ffmpeg?") != 1) {
            Exclamation("Croon needs ffmpeg to operate. Exiting application.");
            return;
        }
        FileSel fsel;
        fsel <<= ffmpegLoc;
        if (!fsel.ExecuteOpen("Find Ffmpeg Executable...")) {
            Exclamation("Croon needs ffmpeg to operate. Exiting application.");
            return;
        }
        ffmpegLoc = ~fsel;
        Config::Set(FFMPEG_LOCATION, ffmpegLoc);
    }
    
    AppAudioPlayer::InitPlayer();
    
    KarData data;
    MainWindow(data).Run();
    
    AppAudioPlayer::DeInitPlayer();
}
