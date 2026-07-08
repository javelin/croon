/*
 * File  : VideoCatalog.h
 * Author: Mark Documento
 */

#ifndef _Croon_VideoCatalog_h_
#define _Croon_VideoCatalog_h_

struct VideoCatalog {
    static Vector<String> FindVideoFiles(String videoDir);
    static String ThumbnailPath(String videoPath);
    static Image LoadThumbnail(String videoPath);
};

#endif
