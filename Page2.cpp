/*
 * File  : Page2.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

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
#include "LyricsDownloadService.h"
#include "TextTools.h"
#include "Page.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "Page2.h"

Page2::Page2() : willDownloadLyrics(true) {
    pageName = "Lyrics";
    CtrlLayout(*this);
    lyricsEd.WhenAction = [=] {
        enableNext = !TrimBoth(~lyricsEd).IsEmpty();
        nextBtn.Enable(enableNext);
    };
}

void Page2::Populate() {
    auto& karData = KarData::GetGlobal();
    willDownloadLyrics = willDownloadLyrics && karData.timedLyrics.IsEmpty();
    if (willDownloadLyrics) {
        String lyrics{""};
        auto downloadStatus = LyricsDownloadService::DownloadWithStatus(karData.title, karData.artist, lyrics);
        if (downloadStatus == LyricsDownloadService::DownloadOk)
            lyrics = TrimBoth(lyrics);
        else if (downloadStatus == LyricsDownloadService::ExtractionFailed)
            Exclamation("Downloaded lyrics could not be read. You can enter lyrics manually.");
        if (lyrics.IsEmpty()) lyrics = "\n";
        lyricsEd.SetData(
                    TrimBoth(Format("%s\n%s\n%s",
                            TextTools::CleanSpacing(TrimBoth(Config::Get(LYRICS_PREFIX))),
                            lyrics,
                            TextTools::CleanSpacing(TrimBoth(Config::Get(LYRICS_SUFFIX))))));
        willDownloadLyrics = false;
    }
    else {
        lyricsEd.SetData(TrimBoth(karData.rawLyrics));
    }
    lyricsEd.SetFocus();
    enableNext = !((String)~lyricsEd).IsEmpty();
}

void Page2::Reset() {
    lyricsEd.Clear();
    enableNext = false;
    willDownloadLyrics = true;
}

void Page2::SaveData() {
    auto raw = TrimBoth(lyricsEd.GetData());
    auto& karData = KarData::GetGlobal();
    karData.rawLyrics = raw;
}
