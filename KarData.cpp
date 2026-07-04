/*
 * File  : KarData.cpp
 * Author: Mark Documento
 */

#include <Draw/Draw.h>

#include <algorithm>
#include <iostream>

using namespace Upp;

#include "AppIdentity.h"
#include "ConfigService.h"
#include "Config.h"
#include "KarData.h"
#include "ProjectSerializer.h"

KarData _karData;
KarData& KarData::GetGlobal() { return _karData; }
void KarData::Reset() {
    loaded = false;
    projectPath.Clear();
    origAudioFilePath.Clear();
    audioFilePath.Clear();
    duration = 0.0f;
    tempo = 0;
    title.Clear();
    artist.Clear();
    writer.Clear();
    owner.Clear();
    genre.Clear();
    year = 0;
    rawLyrics.Clear();
    timedLyrics.Clear();
    parts.Clear();
    videoFilePath.Clear();
    origVideoFile.Clear();
    infoFilePath.Clear();
    thumbnailFilePath.Clear();
    videoThumbnail.Clear();
    timed = 0;
    fontSize = Config::GetInt(FONT_SIZE, Config::DefaultFontSize);
    fontSize = std::min(Config::MaxFontSize, std::max(Config::MinFontSize, fontSize));
    dehiss = false;
    version = AppIdentity::Version();
}

void KarData::Dump() const {
    std::cout << "Is Loaded   : " << (loaded ? "yes":"no")<< std::endl
                << "Version       : " << version.ToStd() << std::endl
                << "Project Path  : " << projectPath.ToStd() << std::endl
                << "OrigAudio Path: " << origAudioFilePath.ToStd() << std::endl
                << "Audio Path    : " << audioFilePath.ToStd() << std::endl
                << "Video Path    : " << videoFilePath.ToStd() << std::endl
                << "Thumbnail Path: " << thumbnailFilePath.ToStd() << std::endl
                << "Info Path     : " << infoFilePath.ToStd() << std::endl
                << "Duration      : " << duration << std::endl
                << "Tempo         : " << tempo << std::endl
                << "Timed         : " << timed << std::endl
                << "Thumbnail     : " << videoThumbnail.ToString().ToStd() << std::endl
                << "Title         : " << title.ToStd() << std::endl
                << "Artist        : " << artist.ToStd() << std::endl
                << "Year          : " << year << std::endl
                << "Genre         : " << genre.ToStd() << std::endl
                << "Writer        : " << writer.ToStd() << std::endl
                << "Owner         : " << owner.ToStd() << std::endl
                << "Original Video: " << origVideoFile.ToStd() << std::endl;
}

void KarData::DumpLyrics() const {
    std::cout << rawLyrics.ToStd() << std::endl;
}

void KarData::DumpTimedLyrics() const {
    for (const auto& tl : timedLyrics) {
        std::cout << tl.time << ": " << tl.lyrics.ToStd() << std::endl;
    }
}

String KarData::ToJSONStr() const {
    return ProjectSerializer::ToJson(*this);
}

KarData::KarData(const String& JSONStr) {
    KarData data = ProjectSerializer::FromJson(JSONStr);
    *this = pick(data);
}

void KarData::SetFontSize(int size) {
    int globalSize = std::min(Config::MaxFontSize, std::max(Config::MinFontSize,
        Config::GetInt(FONT_SIZE, Config::DefaultFontSize)));
    if (Config::MinFontSize > size || Config::MaxFontSize < size) fontSize = globalSize;
    else fontSize = size;
}
