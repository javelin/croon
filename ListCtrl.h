/*
 * File  : ListCtrl.h
 * Author: Mark Documento
 */

#ifndef _Croon_ListCtrl_h_
#define _Croon_ListCtrl_h_

class OverLayCtrl : public ParentCtrl {
public:
    virtual Image MouseEvent(int event, Point p, int zdelta, dword keyflags) override {
        Ctrl* parent = GetParent();
        if (parent) return parent->MouseEvent(event, p, zdelta, keyflags);
        return ParentCtrl::MouseEvent(event, p, zdelta, keyflags);
    }
};

class ListChildCtrl : public ParentCtrl, public Pte<ListChildCtrl> {
public:
    ListChildCtrl();
    ListChildCtrl(ListChildCtrl&& ctrl);
    ListChildCtrl& SetCtrl(Ctrl& ctrl, bool consumeMouseEvents);
    ListChildCtrl& ConsumeMouseEvents(bool consume=true);
    ListChildCtrl& NoConsumeMouseEvents() { return ConsumeMouseEvents(false); }
    const Ctrl* GetCtrl() const { return child; }
    Ctrl* GetCtrl() { return child; }
    void Highlight(bool set=true) { focusFrame.SetColor(set ? LtRed():normal); }
    virtual void LeftDouble(Point p, dword keyflags) override { WhenLeftDouble(); }
    virtual void LeftUp(Point p, dword keyflags) override { WhenLeftUp(); }
    virtual void RightUp(Point p, dword keyflags) override { WhenRightUp(); }
    
    Event<> WhenLeftDouble;
    Event<> WhenLeftUp;
    Event<> WhenRightUp;
    
private:
    Ctrl* child;
    MarginFrame focusFrame;
    Color normal;
    bool consumeMouseEvents{true};
    OverLayCtrl overlay;
};

class ListCtrl : public ParentCtrl {
public:
    enum Orientation {
        Vertical,
        VerticalGrid,
        Horizontal,
        HorizontalGrid
    };
    
    static const int DefaultDim = 50;
    static const int DefaultMargin = 5;
    
public:
    ListCtrl();
    virtual void SetSizeHint(int w, int h);
    virtual void SetMargin(int m) { margin = m; Refresh(); RefreshLayout(); }
    virtual bool Key(dword key, int) override;
    virtual void MouseWheel(Point, int zdelta, dword) override {
        scrollBar.Wheel(zdelta); }
    virtual void Layout() override;
    virtual ListChildCtrl& AddChild(Ctrl& ctrl, bool consumeMouseEvents=true);
    ListCtrl& ConsumeMouseEvents(bool consume=true) {
        for (auto& child : mychildren) child.ConsumeMouseEvents(consume); return *this; }
    ListCtrl& NoConsumeMouseEvents() { return ConsumeMouseEvents(false); }
    virtual void ClearChildren();
    virtual void ClearHighlights();
    virtual void Scroll() {
        if (IsGrid()) {
            LayCtrlsGrid();
        }
        else {
            LayCtrls();
        }
    }
    virtual void LayCtrls();
    virtual void LayCtrlsGrid();
    virtual void Highlight(int item, bool set=true);
    ListChildCtrl* GetChildCtrl(int i) {
        if (i < 0 || i >= mychildren.GetCount()) return nullptr;
        return &mychildren[i];
    }
    virtual int GetCount() const { return mychildren.GetCount(); }
    
    virtual Orientation GetOrientation() const { return orientation; }
    virtual void SetOrientation(Orientation o, int hintW=DefaultDim, int hintH=DefaultDim);
    virtual int GetIndex(const ListChildCtrl& cc) const { return mychildren.GetIndex(cc); }
    virtual bool FocusItem(int item) {
        if (!GetChildCtrl(item)) return false;
        ScrollToItemAndCenter(item);
        return true;
    }
    
    Event<int, Ctrl*> WhenLeftDouble;
    Event<int, Ctrl*> WhenRightUp;
    
protected:
    virtual void ComputeTotal();
    virtual void ScrollToItemAndCenter(int item);
    bool IsVertical() const { return orientation < Horizontal; }
    bool IsGrid() const { return orientation == VerticalGrid || orientation == HorizontalGrid; }
    
    int ItemWidth() const { return UiScaler::X(sizeHint.cx == 0 ? UiScaler::X(DefaultDim):sizeHint.cx) + margin*2; }
    int ItemHeight() const { return UiScaler::Y(sizeHint.cy == 0 ? UiScaler::Y(DefaultDim):sizeHint.cy) + margin*2; }
    int ItemsPerLine() const {
        return IsGrid() ? max(IsVertical() ? GetSize().cx/ItemWidth():GetSize().cy/ItemHeight(), 1):1; }
    int TotalLines() const {
        return IsGrid() ? ceil((double)mychildren.GetCount()/ItemsPerLine()):mychildren.GetCount(); }
    
protected:
    static const int timerId{1};
    Orientation orientation;
    Size sizeHint;
    int margin;
    int focusItem;
    ScrollBar scrollBar;
    Array<ListChildCtrl> mychildren;
};

#endif
