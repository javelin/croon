/*
 * File  : RecentProjectService.h
 * Author: Mark Documento
 */

#ifndef _Croon_RecentProjectService_h_
#define _Croon_RecentProjectService_h_

struct RecentProjectService {
    static Vector<String> NormalizePaths(const Vector<String>& paths);
    static Vector<String> LoadPaths();
    static void SavePaths(const Vector<String>& paths);
    static int FindPathIndex(const Vector<String>& paths, const String& path);
};

#endif
