/*
 * File  : VideoCatalog.cpp
 * Author: Mark Documento
 */

#include <Draw/Draw.h>

using namespace Upp;

#include "Constants.h"
#include "AppPaths.h"
#include "VideoCatalog.h"

Vector<String> VideoCatalog::FindVideoFiles(String videoDir) {
    return AppPaths::FindFiles(videoDir, "*.mp4");
}

Vector<VideoCatalogItem> VideoCatalog::FindCachedThumbnails(String videoDir) {
    Vector<VideoCatalogItem> items;
    Vector<String> paths = FindVideoFiles(videoDir);
    for (int i = 0; i < paths.GetCount(); ++i) {
        Image thumbnail = LoadThumbnail(paths[i]);
        if (thumbnail) {
            VideoCatalogItem& item = items.Add();
            item.videoPath = paths[i];
            item.thumbnailPath = ThumbnailPath(paths[i]);
            item.thumbnail = thumbnail;
        }
    }
    return items;
}

String VideoCatalog::ThumbnailPath(String videoPath) {
    String tnPath = AppendFileName(AppPaths::DataDirectory(), GetFileName(videoPath));
    tnPath.Replace(".mp4", ".thumbnail.png");
    return tnPath;
}

bool VideoCatalog::HasThumbnail(String videoPath) {
    return FileExists(ThumbnailPath(videoPath));
}

Image VideoCatalog::LoadThumbnail(String videoPath) {
    String tnPath = ThumbnailPath(videoPath);
    if (!HasThumbnail(videoPath)) return Image();
    return StreamRaster::LoadFileAny(tnPath);
}

bool VideoCatalog::DeleteThumbnail(String videoPath) {
    return FileDelete(ThumbnailPath(videoPath));
}

Vector<String> VideoCatalog::BuildThumbnailCommand(String videoPath) {
    return {
        "-i",
        videoPath,
        "-ss",
        "00:00:06",
        "-vframes",
        "1",
        "-vf",
        Format("crop='min(iw,ih)':'min(iw,ih)',scale=%d:%d", ThumbnailDim, ThumbnailDim),
        ThumbnailPath(videoPath)
    };
}
