/*
 * File  : RecentProjectService.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

Vector<String> RecentProjectService::NormalizePaths(const Vector<String>& paths) {
    Vector<String> normalized;
    for (const auto& path : paths) {
        String trimmed = TrimBoth(path);
        if (trimmed.IsEmpty() || FindPathIndex(normalized, trimmed) >= 0) continue;
        normalized.Add(trimmed);
    }
    return normalized;
}

Vector<String> RecentProjectService::LoadPaths() {
    return NormalizePaths(Split(ConfigService::Get(PROJECT_LIST), "\n"));
}

void RecentProjectService::SavePaths(const Vector<String>& paths) {
    ConfigService::Set(PROJECT_LIST, Join(NormalizePaths(paths), "\n"));
}

int RecentProjectService::FindPathIndex(const Vector<String>& paths, const String& path) {
    for (int i = 0; i < paths.GetCount(); ++i) {
        if (paths[i] == path) return i;
    }
    return -1;
}
