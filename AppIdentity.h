/*
 * File  : AppIdentity.h
 * Author: Mark Documento
 */

#ifndef _Croon_AppIdentity_h_
#define _Croon_AppIdentity_h_

struct AppIdentity {
    static String ProductName() { return "Croon"; }
    static String Version() { return "1.0"; }
    static String ProjectExtension() { return ".croon"; }
    static String ProjectGlob() { return "*.croon"; }
    static String MetadataAttachmentName() { return "croon.info"; }
    static String TempPrefix() { return "Croon_"; }
    static String PosixDataDirectory() { return ".Croon"; }
    static String DataDirectory() { return "Croon"; }
    
    static String VersionText() {
        return Format("%s v%s", ProductName(), Version());
    }
    
    static String ProjectTypeName() {
        return Format("%s Projects (%s)", ProductName(), ProjectGlob());
    }
    
    static String ProjectMetadataApplication() {
        return Format("APPLICATION=%s v%s", ProductName(), Version());
    }
    
    static String ProjectAttachmentMetadata() {
        return Format("filename=%s", MetadataAttachmentName());
    }
    
    static String ProjectTempFileName() {
        return GetTempFileName(TempPrefix()) + ProjectExtension();
    }
    
    static String TempFileName(String extension) {
        return GetTempFileName(TempPrefix()) + extension;
    }
    
    static String TaggedTempFileName(String tag, String extension) {
        return GetTempFileName(TempPrefix() + tag + "_") + extension;
    }
};

#endif
