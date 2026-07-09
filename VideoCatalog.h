/*
 * File  : VideoCatalog.h
 * Author: Mark Documento
 */

#ifndef _Croon_VideoCatalog_h_
#define _Croon_VideoCatalog_h_

struct VideoCatalogItem : Moveable<VideoCatalogItem> {
    String videoPath;
    String thumbnailPath;
    Image thumbnail;
};

struct VideoCatalog {
    static Vector<String> FindVideoFiles(String videoDir);
    static Vector<VideoCatalogItem> FindCachedThumbnails(String videoDir);
    static String ThumbnailPath(String videoPath);
    static bool HasThumbnail(String videoPath);
    static Image LoadThumbnail(String videoPath);
    static bool DeleteThumbnail(String videoPath);
    static Vector<String> BuildThumbnailCommand(String videoPath);
};

#endif
