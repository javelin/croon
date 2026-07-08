/*
 * File  : VideoCatalog.cpp
 * Author: Mark Documento
 */

#include <Draw/Draw.h>

using namespace Upp;

#include "AppPaths.h"
#include "VideoCatalog.h"

Vector<String> VideoCatalog::FindVideoFiles(String videoDir) {
    return AppPaths::FindFiles(videoDir, "*.mp4");
}

String VideoCatalog::ThumbnailPath(String videoPath) {
    String tnPath = AppendFileName(AppPaths::DataDirectory(), GetFileName(videoPath));
    tnPath.Replace(".mp4", ".thumbnail.png");
    return tnPath;
}

Image VideoCatalog::LoadThumbnail(String videoPath) {
    String tnPath = ThumbnailPath(videoPath);
    if (!FileExists(tnPath)) return Image();
    return StreamRaster::LoadFileAny(tnPath);
}
