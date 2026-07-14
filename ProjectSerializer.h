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
        InvalidMetadata,
    };

    static String FormatVersion();
    static String NormalizeReadVersion(const String& version);
    static bool SupportsVersion(const String& version);
    static String ReadVersion(const String& json);
    static MetadataCompatibility ReadCompatibility(const String& json);
    static String CompatibilityLabel(MetadataCompatibility compatibility);
    static bool SupportsJson(const String& json);
    static String ToJson(const KarData& data);
    static KarData FromJson(const String& json);
};

#endif
