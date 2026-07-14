/*
 * File  : AppIdentity.cpp
 * Author: Mark Documento
 */

#include <Core/Core.h>

using namespace Upp;

#include "AppIdentity.h"

String AppIdentity::ProductName() {
    return "Croon";
}

String AppIdentity::Version() {
    return "1.0";
}

String AppIdentity::ProjectExtension() {
    return ".croon";
}

String AppIdentity::ProjectGlob() {
    return "*.croon";
}

String AppIdentity::MetadataAttachmentName() {
    return "croon.info";
}

String AppIdentity::TempPrefix() {
    return "Croon_";
}

String AppIdentity::PosixDataDirectory() {
    return ".Croon";
}

String AppIdentity::DataDirectory() {
    return "Croon";
}

String AppIdentity::VersionText() {
    return Format("%s v%s", ProductName(), Version());
}

String AppIdentity::ProjectTypeName() {
    return Format("%s Projects (%s)", ProductName(), ProjectGlob());
}

String AppIdentity::ProjectMetadataApplication() {
    return Format("APPLICATION=%s v%s", ProductName(), Version());
}

String AppIdentity::ProjectAttachmentMetadata() {
    return Format("filename=%s", MetadataAttachmentName());
}

String AppIdentity::ProjectTempFileName() {
    return GetTempFileName(TempPrefix()) + ProjectExtension();
}

String AppIdentity::TempFileName(String extension) {
    return GetTempFileName(TempPrefix()) + extension;
}

String AppIdentity::TaggedTempFileName(String tag, String extension) {
    return GetTempFileName(TempPrefix() + tag + "_") + extension;
}
