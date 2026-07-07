/*
 * File  : ProjectSerializer.cpp
 * Author: Mark Documento
 */

#include <Draw/Draw.h>

using namespace Upp;

#include "AppIdentity.h"
#include "KarData.h"
#include "ProjectSerializer.h"

String ProjectSerializer::ReadVersion(const String& json) {
    auto js = ParseJSON(json);
    return NormalizeReadVersion(js.GetAdd("version"));
}

ProjectSerializer::MetadataCompatibility ProjectSerializer::ReadCompatibility(const String& json) {
    try {
        auto js = ParseJSON(json);
        if (js.IsError()) return InvalidMetadata;
        String version = js.GetAdd("version");
        if (version.IsEmpty()) return LegacyUnversionedMetadata;
        return SupportsVersion(version) ? CurrentMetadata : UnsupportedMetadata;
    }
    catch (CParser::Error&) {
        return InvalidMetadata;
    }
}

bool ProjectSerializer::SupportsJson(const String& json) {
    MetadataCompatibility compatibility = ReadCompatibility(json);
    return compatibility == CurrentMetadata || compatibility == LegacyUnversionedMetadata;
}

String ProjectSerializer::CompatibilityLabel(MetadataCompatibility compatibility) {
    switch (compatibility) {
    case CurrentMetadata:
        return "current";
    case LegacyUnversionedMetadata:
        return "legacy-unversioned";
    case UnsupportedMetadata:
        return "unsupported";
    case InvalidMetadata:
        return "invalid";
    }
    return "unknown";
}

String ProjectSerializer::ToJson(const KarData& data) {
    JsonArray tlJsa;
    for (const auto& tl : data.timedLyrics) {
        if (data.timedLyrics.GetIndex(tl) > 0) tlJsa << Json("time", tl.time)("lyrics", tl.lyrics);
    }
    JsonArray partsJsa;
    for (const auto& part : data.parts) {
        partsJsa << Json("index", part.a)("part1", part.b)("part2", part.c)("part3", part.d);
    }
    Json js;
    js("version", FormatVersion())
        ("title", data.title)
        ("artist", data.artist)
        ("genre", data.genre)
        ("year", data.year)
        ("writer", data.writer)
        ("owner", data.owner)
        ("origVideoFile", data.origVideoFile)
        ("duration", data.duration)
        ("timed", data.timed)
        ("fontSize", data.fontSize)
        ("dehiss", data.dehiss)
        ("timedLyrics", tlJsa)
        ("parts", partsJsa);
    return js.ToString();
}

KarData ProjectSerializer::FromJson(const String& json) {
    KarData data;
    auto js = ParseJSON(json);
    data.version = NormalizeReadVersion(js.GetAdd("version"));
    data.title = js.GetAdd("title");
    data.artist = js.GetAdd("artist");
    data.genre = js.GetAdd("genre");
    data.year = js.GetAdd("year");
    if (data.year < 0) data.year = 0;
    data.writer = js.GetAdd("writer");
    data.owner = js.GetAdd("owner");
    data.origVideoFile = js.GetAdd("origVideoFile");
    data.duration = js.GetAdd("duration");
    data.timed = js.GetAdd("timed");
    data.SetFontSize(js.GetAdd("fontSize"));
    data.dehiss = js.GetAdd("dehiss");
    data.timedLyrics.Add({data.duration, ""});
    for (auto tl : js.GetAdd("timedLyrics")) {
        data.timedLyrics.Add({tl.GetAdd("time"), tl.GetAdd("lyrics")});
    }
    for (auto part : js.GetAdd("parts")) {
        data.parts.Add({part.GetAdd("index"),
                        part.GetAdd("part1"),
                        part.GetAdd("part2"),
                        part.GetAdd("part3")});
    }
    return pick(data);
}
