/*
 * File  : Page2.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

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
        if (LyricsDownloadService::Download(karData.title, karData.artist, lyrics)) lyrics = TrimBoth(lyrics);
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
