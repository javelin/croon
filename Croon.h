/*
 * File  : Croon.h
 * Author: Mark Documento
 */

#ifndef _Croon_Croon_h
#define _Croon_Croon_h

#include <algorithm>
#include <cmath>
#include <ctime>
#include <iostream>
#include <filesystem>
#include <fstream>

#include <CtrlLib/CtrlLib.h>
#include <GridCtrl/GridCtrl.h>
#include <plugin/pcre/Pcre.h>
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

#include "Visualization.h"
typedef struct Visualization VIZ;

#include "Constants.h"
#include "AppIdentity.h"
#include "KarData.h"
#include "ProjectSerializer.h"
#include "LyricsTransformer.h"
#include "SubtitleGenerator.h"
#include "Util.h"
#include "AudioPlayerBase.h"
#include "ConfigService.h"
#include "Config.h"
#include "FfmpegCommandBuilder.h"
#include "RecentProjectService.h"
#include "TimingLine.h"
#include "MediaProcessRunner.h"
#include "ListCtrl.h"
#include "LyricsEditor.h"
#include "LyricsPartsCtrl.h"
#include "Page.h"
#include "ProjectLoader.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "LyricsPartsDlg.h"
#include "TimingCtrl.h"

#define LAYOUTFILE <Croon/CroonTimingDlg.lay>
#include <CtrlCore/lay.h>

#include "MusicPlayer.h"
#include "ProgressDlg.h"
#include "ExportDlg.h"
#include "GatherDlg.h"
#include "DownloadDlg.h"
#include "OpenProjectDlg.h"
#include "SaveProjectDlg.h"
#include "SDLMixerAudioPlayer.h"
#include "SettingsDlg.h"
#include "TimingDlg.h"
#include "TimingLineDlg.h"
#include "VidThumbnail.h"
#include "Page1.h"
#include "Page2.h"
#include "Page3.h"

#define LAYOUTFILE <Croon/CroonVideoDlg.lay>
#include <CtrlCore/lay.h>

#include "ConvertDlg.h"
#include "VideoDlg.h"
#include "Project.h"
#include "ProjectList.h"

#define LAYOUTFILE <Croon/CroonMainWindow.lay>
#include <CtrlCore/lay.h>

#include "MainWindow.h"

#define LAYOUTFILE <Croon/CroonWizardShell.lay>
#include <CtrlCore/lay.h>

#include "WizardDlg.h"

GatherDlg& GetGatherDlg();
VideoDlg& GetVideoDlg();
WizardDlg& GetWizardDlg();
void RunCroon();

#endif
