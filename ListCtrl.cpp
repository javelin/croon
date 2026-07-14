/*
 * File  : ListCtrl.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

#include <cmath>

using namespace Upp;

#include "UiScaler.h"
#include "ListCtrl.h"

Image OverLayCtrl::MouseEvent(int event, Point p, int zdelta, dword keyflags) {
    Ctrl* parent = GetParent();
    if (parent) return parent->MouseEvent(event, p, zdelta, keyflags);
    return ParentCtrl::MouseEvent(event, p, zdelta, keyflags);
}

ListChildCtrl::ListChildCtrl() {
    child = nullptr;
    focusFrame.SetMargins(Rect(4, 4, 4, 4));
    normal = Color(0xEE, 0xEE, 0xEE);
    focusFrame.SetColor(normal);
    SetFrame(focusFrame);
    WantFocus();
}

ListChildCtrl::ListChildCtrl(ListChildCtrl&& ctrl) {
    child = ctrl.child;
    ctrl.child = nullptr;
    consumeMouseEvents = ctrl.consumeMouseEvents;
    if (child) {
        child->Remove();
        *this << child->HSizePos().VSizePos();
        if (consumeMouseEvents) *this << overlay.HSizePos().VSizePos();
    }
    focusFrame.SetMargins(Rect(4, 4, 4, 4));
    normal = Color(0xEE, 0xEE, 0xEE);
    focusFrame.SetColor(normal);
    SetFrame(focusFrame);
}

ListChildCtrl& ListChildCtrl::SetCtrl(Ctrl& ctrl, bool consumeMouseEvents) {
    *this << ctrl.HSizePos().VSizePos();
    child = GetLastChild();
    this->consumeMouseEvents = consumeMouseEvents;
    if (consumeMouseEvents) *this << overlay.HSizePos().VSizePos();
    return *this;
}

ListChildCtrl& ListChildCtrl::ConsumeMouseEvents(bool consume) {
    if (consumeMouseEvents && !consume) overlay.Remove();
    else if (!consumeMouseEvents && consume) *this << overlay.HSizePos().VSizePos();
    consumeMouseEvents = consume;
    return *this;
}

ListChildCtrl& ListChildCtrl::NoConsumeMouseEvents() {
    return ConsumeMouseEvents(false);
}

const Ctrl* ListChildCtrl::GetCtrl() const {
    return child;
}

Ctrl* ListChildCtrl::GetCtrl() {
    return child;
}

void ListChildCtrl::Highlight(bool set) {
    focusFrame.SetColor(set ? LtRed():normal);
}

void ListChildCtrl::LeftDouble(Point p, dword keyflags) {
    WhenLeftDouble();
}

void ListChildCtrl::LeftUp(Point p, dword keyflags) {
    WhenLeftUp();
}

void ListChildCtrl::RightUp(Point p, dword keyflags) {
    WhenRightUp();
}

ListCtrl::ListCtrl() : margin(DefaultMargin) {
    scrollBar.Vert().AutoHide();
    AddFrame(scrollBar);
    scrollBar.WhenScroll = [=] { Scroll(); };
    SetOrientation(Vertical, 0, UiScaler::InverseY(DefaultDim));
    WantFocus().SetFocus();
    focusItem = 0;
    Highlight(focusItem, true);
    ScrollToItemAndCenter(focusItem);
}

void ListCtrl::Layout() {
    ParentCtrl::Layout();
    scrollBar.SetPage(IsVertical() ? GetSize().cy : GetSize().cx);
    PostCallback([=] {
        ComputeTotal();
        Scroll();
    });
}

bool ListCtrl::Key(dword key, int count) {
    switch (key) {
    case K_UP:
    case K_LEFT:
        if (focusItem > 0) {
            Highlight(focusItem, false);
            int ipl = ItemsPerLine();
            if (IsGrid()) {
                int row = focusItem/ipl, col = focusItem%ipl;
                if ((IsVertical() && key == K_UP) ||
                    (!IsVertical() && key == K_LEFT)) {
                    if (row > 0) --row;
                }
                else {
                    if (col > 0) --col;
                    else if (row > 0) {
                        col = ipl - 1;
                        --row;
                    }
                }
                focusItem = row*ipl + col;
                if (focusItem < 0) focusItem = 0;
            }
            else --focusItem;
            Highlight(focusItem, true);
        }
        ScrollToItemAndCenter(focusItem);
        return true;
    case K_DOWN:
    case K_RIGHT:
        if (focusItem < mychildren.GetCount() - 1) {
            Highlight(focusItem, false);
            int ipl = ItemsPerLine(), tl = TotalLines();
            if (IsGrid()) {
                int row = focusItem/ipl, col = focusItem%ipl;
                if ((IsVertical() && key == K_DOWN) ||
                    (!IsVertical() && key == K_RIGHT)) {
                    if (row < tl - 1) ++row;
                }
                else {
                    if (col < ipl - 1) ++col;
                    else if (row < tl - 1) {
                        col = 0;
                        ++row;
                    }
                }
                focusItem = row*ipl + col;
                if (focusItem > mychildren.GetCount() - 1) focusItem = mychildren.GetCount() - 1;
            }
            else ++focusItem;
            Highlight(focusItem, true);
        }
        ScrollToItemAndCenter(focusItem);
        return true;
    default:
        return GetParent()->Key(key, count);
    }
    
    return false;
}

void ListCtrl::SetSizeHint(int hintW, int hintH) {
    if (IsGrid()) {
        hintW = hintW == 0 ? UiScaler::InverseX(DefaultDim):hintW;
        hintH = hintH == 0 ? UiScaler::InverseY(DefaultDim):hintH;
    }
    else {
        hintW = max(hintW, 0);
        hintH = max(hintH, 0);
        if (IsVertical() && hintH < 1) {
            hintH = UiScaler::InverseY(DefaultDim);
        }
        else if (!IsVertical() && hintW < 1) {
            hintW = UiScaler::InverseX(DefaultDim);
        }
    }
    sizeHint.cx = hintW;
    sizeHint.cy = hintH;
    scrollBar.SetLine(IsVertical() ? UiScaler::Y(sizeHint.cy):UiScaler::X(sizeHint.cx));
    if (IsVertical())
    ComputeTotal();
    Refresh();
    RefreshLayout();
}

void ListCtrl::SetMargin(int m) {
    margin = m;
    Refresh();
    RefreshLayout();
}

ListChildCtrl& ListCtrl::AddChild(Ctrl& ctrl, bool consumeMouseEvents) {
    ListChildCtrl child;
    mychildren.Add(pick(child.SetCtrl(ctrl, consumeMouseEvents)));
    Add(mychildren.back().LeftPos(0, 0).TopPos(0, 0));
    PostCallback([=] {
        ComputeTotal();
        Refresh();
    });
    int index = mychildren.GetCount() - 1;
    mychildren.back().WhenLeftDouble << [=] {
        WhenLeftDouble(index, mychildren[index].GetCtrl());
    };
    mychildren.back().WhenLeftUp << [=] {
        focusItem = index;
        KillSetTimeCallback(500, [=] { ScrollToItemAndCenter(focusItem); }, timerId);
    };
    mychildren.back().WhenRightUp << [=] {
        focusItem = index;
        for (auto& child : mychildren) child.Highlight(false);
        Highlight(index);
        Refresh();
        WhenRightUp(index, mychildren[index].GetCtrl());
    };
    return mychildren.back();
}

void ListCtrl::MouseWheel(Point, int zdelta, dword) {
    scrollBar.Wheel(zdelta);
}

ListCtrl& ListCtrl::ConsumeMouseEvents(bool consume) {
    for (auto& child : mychildren) child.ConsumeMouseEvents(consume);
    return *this;
}

ListCtrl& ListCtrl::NoConsumeMouseEvents() {
    return ConsumeMouseEvents(false);
}

void ListCtrl::ClearChildren() {
    for (auto& child : mychildren) child.Remove();
    mychildren.Clear();
}

void ListCtrl::ClearHighlights() {
    for (auto& child : mychildren) child.Highlight(false);
}

void ListCtrl::Scroll() {
    if (IsGrid()) {
        LayCtrlsGrid();
    }
    else {
        LayCtrls();
    }
}

void ListCtrl::LayCtrls() {
    auto pos{scrollBar.Get()};
    auto ih{ItemHeight()}, iw{ItemWidth()};
    for(int i = 0; ; ++i){
        auto* child{GetChildCtrl(i)};
        if (!child) return;
        if (orientation == Vertical) {
            child->TopPos(ih*i - pos, ih);
            if (sizeHint.cx == 0) {
                child->HSizePos();
            }
            else {
                child->LeftPos(0, iw);
            }
        }
        else if (orientation == Horizontal) {
            child->TopPos(iw*i - pos, iw);
            if (sizeHint.cy == 0) {
                child->VSizePos();
            }
            else {
                child->TopPos(0, iw);
            }
        }
    }
}

void ListCtrl::LayCtrlsGrid() {
    auto pos{scrollBar.Get()};
    auto iw{ItemWidth()}, ih{ItemHeight()};
    for (int i = 0, ci = 0; ; ++i) {
        for (int j = 0; j < max(ItemsPerLine(), 1); ++j, ++ci) {
            auto* child{GetChildCtrl(ci)};
            if (!child) return;
            if (IsVertical()) {
                child->TopPos(ih*i - pos, ih);
                child->LeftPos(iw*j, iw);
            }
            else {
                child->LeftPos(iw*i - pos, iw);
                child->TopPos(ih*j, ih);
            }
        }
    }
}

void ListCtrl::ScrollToItemAndCenter(int item) {
    ListChildCtrl* ch = GetChildCtrl(item);
    if (!ch) return;
    auto itemSize{IsVertical() ? ItemHeight():ItemWidth()};
    auto span = IsVertical() ? GetSize().cy:GetSize().cx;
    auto row{item/ItemsPerLine()};
    auto pos{itemSize*row - (span/2 - itemSize/2)};
    scrollBar.Set(pos);
    for (auto& child : mychildren) child.Highlight(false);
    Highlight(item);
    Refresh();
    RefreshLayout();
}

void ListCtrl::Highlight(int item, bool set) {
    ListChildCtrl* ch = GetChildCtrl(item);
    if (ch) ch->Highlight(set);
}

ListChildCtrl* ListCtrl::GetChildCtrl(int i) {
    if (i < 0 || i >= mychildren.GetCount()) return nullptr;
    return &mychildren[i];
}

int ListCtrl::GetCount() const {
    return mychildren.GetCount();
}

ListCtrl::Orientation ListCtrl::GetOrientation() const {
    return orientation;
}

void ListCtrl::SetOrientation(Orientation o, int hintW, int hintH) {
    orientation = o;
    SetSizeHint(hintW, hintH);
}

int ListCtrl::GetIndex(const ListChildCtrl& cc) const {
    return mychildren.GetIndex(cc);
}

bool ListCtrl::FocusItem(int item) {
    if (!GetChildCtrl(item)) return false;
    ScrollToItemAndCenter(item);
    return true;
}

void ListCtrl::ComputeTotal() {
    scrollBar.SetTotal((IsVertical() ? ItemHeight():ItemWidth())*TotalLines());
}

bool ListCtrl::IsVertical() const {
    return orientation < Horizontal;
}

bool ListCtrl::IsGrid() const {
    return orientation == VerticalGrid || orientation == HorizontalGrid;
}

int ListCtrl::ItemWidth() const {
    return UiScaler::X(sizeHint.cx == 0 ? UiScaler::InverseX(DefaultDim):sizeHint.cx) + margin*2;
}

int ListCtrl::ItemHeight() const {
    return UiScaler::Y(sizeHint.cy == 0 ? UiScaler::InverseY(DefaultDim):sizeHint.cy) + margin*2;
}

int ListCtrl::ItemsPerLine() const {
    return IsGrid() ? max(IsVertical() ? GetSize().cx/ItemWidth():GetSize().cy/ItemHeight(), 1):1;
}

int ListCtrl::TotalLines() const {
    return IsGrid() ? ceil((double)mychildren.GetCount()/ItemsPerLine()):mychildren.GetCount();
}
