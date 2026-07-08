/*
 * File  : VideoCatalog.cpp
 * Author: Mark Documento
 */

#include <Core/Core.h>

using namespace Upp;

#include "AppPaths.h"
#include "VideoCatalog.h"

Vector<String> VideoCatalog::FindVideoFiles(String videoDir) {
    return AppPaths::FindFiles(videoDir, "*.mp4");
}
