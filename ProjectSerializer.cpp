/*
 * File  : ProjectSerializer.cpp
 * Author: Mark Documento
 */

#include <Draw/Draw.h>

using namespace Upp;

#include "AppIdentity.h"
#include "KarData.h"
#include "ProjectSerializer.h"

namespace {

bool ParseMetadataObject(const String& json, Value& js) {
    try {
        js = ParseJSON(json);
        return !js.IsError() && js.Is<ValueMap>();
    }
    catch (CParser::Error&) {
        return false;
    }
}

}

String ProjectSerializer::FormatVersion() {
    return AppIdentity::Version();
}

String ProjectSerializer::NormalizeReadVersion(const String& version) {
    if (version.IsEmpty() || version == "1.0") return FormatVersion();
    return version;
}

bool ProjectSerializer::SupportsVersion(const String& version) {
    return NormalizeReadVersion(version) == FormatVersion();
}

String ProjectSerializer::ReadVersion(const String& json) {
    Value js;
    if (!ParseMetadataObject(json, js)) return String::GetVoid();
    return NormalizeReadVersion(js.GetAdd("version"));
}

ProjectSerializer::MetadataCompatibility ProjectSerializer::ReadCompatibility(const String& json) {
    Value js;
    if (!ParseMetadataObject(json, js)) return InvalidMetadata;
    String version = js.GetAdd("version");
    if (version.IsEmpty()) return LegacyUnversionedMetadata;
    return SupportsVersion(version) ? CurrentMetadata : UnsupportedMetadata;
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
        ("subtitleLines", data.subtitleLines)
        ("dehiss", data.dehiss)
        ("timedLyrics", tlJsa)
        ("parts", partsJsa);
    return js.ToString();
}

KarData ProjectSerializer::FromJson(const String& json) {
    KarData data;
    Value js;
    if (!ParseMetadataObject(json, js)) {
        data.version = String::GetVoid();
        return pick(data);
    }
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
    data.SetSubtitleLines(js.GetAdd("subtitleLines"));
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
