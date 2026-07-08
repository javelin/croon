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
#include "AzLyricsProvider.h"
#include "LyricsDownloadService.h"

const char* LyricsDownloadService::ProviderName() {
    return AzLyricsProvider::Name();
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
    return AzLyricsProvider::BuildUrl(title, artist);
}

bool LyricsDownloadService::ExtractProviderLyrics(String content, String& lyrics) {
    return AzLyricsProvider::ExtractLyrics(content, lyrics);
}

LyricsDownloadService::DownloadStatus LyricsDownloadService::DownloadWithStatus(String title, String artist, String& lyrics) {
    DownloadDlg dlg;
    String* output = &lyrics;
    bool extracted = false;
    dlg.WhenDownloadSuccess << [output, &extracted](String content) {
        extracted = LyricsDownloadService::ExtractProviderLyrics(content, *output);
    };
    if (dlg.Run(BuildProviderUrl(title, artist), "Downloading lyrics") != IDOK)
        return DownloadCancelled;
    return extracted ? DownloadOk : ExtractionFailed;
}

bool LyricsDownloadService::Download(String title, String artist, String& lyrics) {
    return DownloadWithStatus(title, artist, lyrics) == DownloadOk;
}
