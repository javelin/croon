/*
 * File  : AppIdentity.h
 * Author: Mark Documento
 */

#ifndef _Croon_AppIdentity_h_
#define _Croon_AppIdentity_h_

struct AppIdentity {
    static String ProductName();
    static String Version();
    static String ProjectExtension();
    static String ProjectGlob();
    static String MetadataAttachmentName();
    static String TempPrefix();
    static String PosixDataDirectory();
    static String DataDirectory();
    static String VersionText();
    static String ProjectTypeName();
    static String ProjectMetadataApplication();
    static String ProjectAttachmentMetadata();
    static String ProjectTempFileName();
    static String TempFileName(String extension);
    static String TaggedTempFileName(String tag, String extension);
};

#endif
