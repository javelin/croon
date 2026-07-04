/*
 * File  : VidThumbnail.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#include "Constants.h"
#include "VidThumbnail.h"

const int VidThumbnail::Width = ThumbnailDim;
const int VidThumbnail::Height = ThumbnailDim;

VidThumbnail::VidThumbnail() {
    Initialize();
}

VidThumbnail::VidThumbnail(String path, Image image) : path(path), image(image) {
    Initialize();
    imgCtrl.SetImage(image);
}

VidThumbnail::VidThumbnail(const VidThumbnail& vt) : VidThumbnail(vt.path, vt.image) {
    SetSelected(vt.selected.Get() == 1);
}

void VidThumbnail::SetSelected(bool set) {
    selected.Set(set ? 1:0);
}

void VidThumbnail::Layout() {
    auto sz = imgCtrl.GetSize();
    if (sz.cx < 1 || sz.cy < 1 || sz.cx > Width || sz.cy > Height) return;
    imgCtrl.SetImage(Rescale(image, sz));
}

void VidThumbnail::Initialize() {
    *this << imgCtrl.HSizePosZ().VSizePosZ()
            << selected.LeftPosZ(10).TopPosZ(10);
    SetSelected(false);
    selected.WantFocus(false);
    selected << [=] { WhenSelected(path); SetSelected(); };
    imgCtrl.WhenPushed << [=] { WhenSelected(path); SetSelected(); };
}
