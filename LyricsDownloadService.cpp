/*
 * File  : LyricsDownloadService.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>
#include <plugin/pcre/Pcre.h>

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

#include "ProgressDlg.h"
#include "DownloadDefaults.h"
#include "DownloadDlg.h"
#include "TextTools.h"
#include "LyricsDownloadService.h"

const char* LyricsDownloadService::ProviderName() {
    return "AZLyrics";
}

const char* LyricsDownloadService::AzLyricsUrlFormat() {
    return "https://www.azlyrics.com/lyrics/%s/%s.html";
}

const char* LyricsDownloadService::AzLyricsPattern() {
    return "<!-- Usage of azlyrics.com content by any third-party.+?-->(.+?)</div>";
}

String LyricsDownloadService::BuildAzLyricsUrl(String title, String artist) {
    String t = Join(Split(TextTools::CleanSpacing(ToLower(TextTools::StripNonAlnum(TrimBoth(title)))), ' '), "");
    String a = ToLower(TextTools::StripNonAlnum(TrimBoth(artist)));
    a.TrimStart("the ");
    a = Join(Split(TextTools::CleanSpacing(a), ' '), "");
    return Format(AzLyricsUrlFormat(), a, t);
}

bool LyricsDownloadService::ExtractAzLyrics(String content, String& lyrics) {
    static RegExp rx(AzLyricsPattern(), RegExp::DOTALL | RegExp::MULTILINE | RegExp::UTF8);
    if (rx.Study() && rx.Match(content)) {
        auto vl = Split(rx.GetString(0), "<br>");
        for (int i = 0; i < vl.GetCount(); i++) {
            vl[i] = TrimBoth(vl[i]);
        }
        lyrics = Join(vl, "\n");
        return true;
    }
    lyrics = "";
    return false;
}

bool LyricsDownloadService::Download(String title, String artist, String& lyrics) {
    DownloadDlg dlg;
    String* output = &lyrics;
    dlg.WhenDownloadSuccess << [output](String content) {
        LyricsDownloadService::ExtractAzLyrics(content, *output);
    };
    return dlg.Run(BuildAzLyricsUrl(title, artist), "Downloading lyrics") == IDOK;
}
