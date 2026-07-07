/*
 * File  : ProjectSerializer.h
 * Author: Mark Documento
 */

#ifndef _Croon_ProjectSerializer_h_
#define _Croon_ProjectSerializer_h_

struct ProjectSerializer {
    enum MetadataCompatibility {
        CurrentMetadata,
        LegacyUnversionedMetadata,
        UnsupportedMetadata,
    };

    static String FormatVersion() { return AppIdentity::Version(); }
    static String NormalizeReadVersion(const String& version) { return version.IsEmpty() ? FormatVersion() : version; }
    static bool SupportsVersion(const String& version) { return NormalizeReadVersion(version) == FormatVersion(); }
    static String ReadVersion(const String& json);
    static MetadataCompatibility ReadCompatibility(const String& json);
    static bool SupportsJson(const String& json) { return ReadCompatibility(json) != UnsupportedMetadata; }
    static String ToJson(const KarData& data);
    static KarData FromJson(const String& json);
};

#endif
