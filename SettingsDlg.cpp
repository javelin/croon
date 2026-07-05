/*
 * File  : SettingsDlg.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

#include <algorithm>

using namespace Upp;

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_header.h>

#include "Constants.h"
#include "ConfigService.h"
#include "Config.h"
#include "UiScaler.h"
#include "LyricsPartsCtrl.h"
#include "ListCtrl.h"
#include "AppIdentity.h"
#include "KarData.h"
#include "Visualization.h"
#include "FfmpegCommandBuilder.h"
#include "LyricsTransformer.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ProjectLoader.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "SettingsDlg.h"

SettingsDlg::SettingsDlg() {
    CtrlLayout(*this, "Settings");
    NoSizeable().NoZoomable();
    CenterOwner();
    ffmpegLoc = Config::Get(FFMPEG_LOCATION);
    ffmpegEd.SetData(Config::Get(FFMPEG_LOCATION));
    prefixEd.SetData(Config::Get(LYRICS_PREFIX));
    suffixEd.SetData(Config::Get(LYRICS_SUFFIX));
    int fontSize = Config::GetInt(FONT_SIZE, Config::DefaultFontSize);
    fontSize = std::min(Config::MaxFontSize, std::max(Config::MinFontSize, fontSize));
    fontSizeEd.SetData(fontSize);
    ffmpegBtn << [=] {
        FileSel fs;
        fs <<= ffmpegLoc;
        if(!fs.ExecuteOpen("Find Ffmpeg Executable...")) return;
        ffmpegLoc = ~fs;
        ffmpegEd.SetData(ffmpegLoc);
        Config::Set(FFMPEG_LOCATION, ffmpegLoc);
    };
    
    closeBtn << [=] {
        Close();
    };
}

void SettingsDlg::Close() {
    Config::Set(LYRICS_PREFIX, (String)prefixEd.GetData());
    Config::Set(LYRICS_SUFFIX, (String)suffixEd.GetData());
    Config::Set(FONT_SIZE, (int)fontSizeEd.GetData());
    Break(IDOK);
    TopWindow::Close();
}
