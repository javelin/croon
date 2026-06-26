/*
 * File  : KarData.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

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
    JsonArray tlJsa;
    for (const auto& tl : timedLyrics) {
        if (timedLyrics.GetIndex(tl) > 0) tlJsa << Json("time", tl.time)("lyrics", tl.lyrics);
    }
    JsonArray partsJsa;
    for (const auto& part : parts) {
        partsJsa << Json("index", part.a)("part1", part.b)("part2", part.c)("part3", part.d);
    }
    Json js;
    js("version", version)
        ("title", title)
        ("artist", artist)
        ("genre", genre)
        ("year", year)
        ("writer", writer)
        ("owner", owner)
        ("origVideoFile", origVideoFile)
        ("duration", duration)
        ("timed", timed)
        ("fontSize", fontSize)
        ("dehiss", dehiss)
        ("timedLyrics", tlJsa)
        ("parts", partsJsa);
    return js.ToString();
}

KarData::KarData(const String& JSONStr) {
    Reset();
    auto js = ParseJSON(JSONStr);
    version = js.GetAdd("version");
    title = js.GetAdd("title");
    artist = js.GetAdd("artist");
    genre = js.GetAdd("genre");
    year = js.GetAdd("year");
    if (year < 0) year = 0;
    writer = js.GetAdd("writer");
    owner = js.GetAdd("owner");
    origVideoFile = js.GetAdd("origVideoFile");
    duration = js.GetAdd("duration");
    timed = js.GetAdd("timed");
    SetFontSize(js.GetAdd("fontSize"));
    dehiss = js.GetAdd("dehiss");
    timedLyrics.Add({duration, ""});
    for (auto tl : js.GetAdd("timedLyrics")) {
        timedLyrics.Add({tl.GetAdd("time"), tl.GetAdd("lyrics")});
    }
    for (auto part : js.GetAdd("parts")) {
        parts.Add({part.GetAdd("index"),
                    part.GetAdd("part1"),
                    part.GetAdd("part2"),
                    part.GetAdd("part3")});
    }
}

void KarData::SetFontSize(int size) {
    int globalSize = std::min(Config::MaxFontSize, std::max(Config::MinFontSize,
        Config::GetInt(FONT_SIZE, Config::DefaultFontSize)));
    if (Config::MinFontSize > size || Config::MaxFontSize < size) fontSize = globalSize;
    else fontSize = size;
}
