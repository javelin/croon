/*
 * File  : AppPaths.cpp
 * Author: Mark Documento
 */

#include <Core/Core.h>

using namespace Upp;

#include "AppIdentity.h"
#include "AppPaths.h"

String AppPaths::DataDirectory() {
    static String dataDir = AppendFileName(GetAppDataFolder(),
#ifdef PLATFORM_POSIX
                                           AppIdentity::PosixDataDirectory()
#else
                                           AppIdentity::DataDirectory()
#endif
                                           );
    if (!DirectoryExists(dataDir)) DirectoryCreate(dataDir);
    return dataDir;
}

Vector<String> AppPaths::FindFiles(String dir, String pattern) {
    Vector<String> paths;
    for (FindFile ff(dir + "/*.*"); ff; ff++) {
        String p = ff.GetPath();
        if(PatternMatchMulti(pattern, ff.GetName()) && ff.IsFile()) {
            paths.Add(ff.GetPath());
        }
    }
    return paths;
}
