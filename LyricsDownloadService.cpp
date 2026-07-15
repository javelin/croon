/*
 * File  : LyricsDownloadService.cpp
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
#include "LyricsTransformer.h"
#include "MediaProcessRunner.h"
#include "RecentProjectService.h"
#include "ProjectLoader.h"

#define LAYOUTFILE <Croon/Croon.lay>
#include <CtrlCore/lay.h>

#include "ProgressDlg.h"
#include "DownloadDefaults.h"
#include "DownloadDlg.h"
#include "AzLyricsProvider.h"
#include "GeniusLyricsProvider.h"
#include "SongLyricsProvider.h"
#include "LyricsDownloadService.h"

namespace {
struct LyricsProvider {
    const char* name;
    String (*buildUrl)(String title, String artist);
    bool (*extractLyrics)(String content, String& lyrics);
};

Vector<LyricsProvider> GetProviders() {
    Vector<LyricsProvider> providers;
    providers.Add({ AzLyricsProvider::Name(), AzLyricsProvider::BuildUrl, AzLyricsProvider::ExtractLyrics });
    providers.Add({ GeniusLyricsProvider::Name(), GeniusLyricsProvider::BuildUrl, GeniusLyricsProvider::ExtractLyrics });
    providers.Add({ SongLyricsProvider::Name(), SongLyricsProvider::BuildUrl, SongLyricsProvider::ExtractLyrics });
    return providers;
}
}

const char* LyricsDownloadService::ProviderName() {
    return GetProviders()[0].name;
}

const char* LyricsDownloadService::DownloadStatusLabel(DownloadStatus status) {
    switch (status) {
    case DownloadOk:
        return "ok";
    case DownloadCancelled:
        return "cancelled";
    case ExtractionFailed:
        return "extraction-failed";
    }
    return "unknown";
}

String LyricsDownloadService::BuildProviderUrl(String title, String artist) {
    return GetProviders()[0].buildUrl(title, artist);
}

bool LyricsDownloadService::ExtractProviderLyrics(String content, String& lyrics) {
    return GetProviders()[0].extractLyrics(content, lyrics);
}

LyricsDownloadService::DownloadStatus LyricsDownloadService::DownloadWithStatus(String title, String artist, String& lyrics) {
    for (const auto& provider : GetProviders()) {
        DownloadDlg dlg;
        String* output = &lyrics;
        bool extracted = false;
        auto extractLyrics = provider.extractLyrics;
        dlg.WhenDownloadSuccess << [output, &extracted, extractLyrics](String content) {
            extracted = extractLyrics(content, *output);
        };

        int result = dlg.Run(provider.buildUrl(title, artist), "Downloading lyrics");
        if (result == IDABORT)
            return DownloadCancelled;
        if (result == IDOK && extracted)
            return DownloadOk;
    }

    lyrics = "";
    return ExtractionFailed;
}

bool LyricsDownloadService::Download(String title, String artist, String& lyrics) {
    return DownloadWithStatus(title, artist, lyrics) == DownloadOk;
}
