/*
 * File  : ProjectSerializer.h
 * Author: Mark Documento
 */

#ifndef _Croon_ProjectSerializer_h_
#define _Croon_ProjectSerializer_h_

struct ProjectSerializer {
    static String FormatVersion() { return AppIdentity::Version(); }
    static bool SupportsVersion(const String& version) { return version == FormatVersion(); }
    static String ToJson(const KarData& data);
    static KarData FromJson(const String& json);
};

#endif
