/*
 * File  : ListCtrl.h
 * Author: Mark Documento
 */

#ifndef _Croon_ListCtrl_h_
#define _Croon_ListCtrl_h_

class OverLayCtrl : public ParentCtrl {
public:
    virtual Image MouseEvent(int event, Point p, int zdelta, dword keyflags) override;
};

class ListChildCtrl : public ParentCtrl, public Pte<ListChildCtrl> {
public:
    ListChildCtrl();
    ListChildCtrl(ListChildCtrl&& ctrl);
    ListChildCtrl& SetCtrl(Ctrl& ctrl, bool consumeMouseEvents);
    ListChildCtrl& ConsumeMouseEvents(bool consume=true);
    ListChildCtrl& NoConsumeMouseEvents();
    const Ctrl* GetCtrl() const;
    Ctrl* GetCtrl();
    void Highlight(bool set=true);
    virtual void LeftDouble(Point p, dword keyflags) override;
    virtual void LeftUp(Point p, dword keyflags) override;
    virtual void RightUp(Point p, dword keyflags) override;
    
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
    virtual void SetMargin(int m);
    virtual bool Key(dword key, int) override;
    virtual void MouseWheel(Point, int zdelta, dword) override;
    virtual void Layout() override;
    virtual ListChildCtrl& AddChild(Ctrl& ctrl, bool consumeMouseEvents=true);
    ListCtrl& ConsumeMouseEvents(bool consume=true);
    ListCtrl& NoConsumeMouseEvents();
    virtual void ClearChildren();
    virtual void ClearHighlights();
    virtual void Scroll();
    virtual void LayCtrls();
    virtual void LayCtrlsGrid();
    virtual void Highlight(int item, bool set=true);
    ListChildCtrl* GetChildCtrl(int i);
    virtual int GetCount() const;
    
    virtual Orientation GetOrientation() const;
    virtual void SetOrientation(Orientation o, int hintW=DefaultDim, int hintH=DefaultDim);
    virtual int GetIndex(const ListChildCtrl& cc) const;
    virtual bool FocusItem(int item);
    
    Event<int, Ctrl*> WhenLeftDouble;
    Event<int, Ctrl*> WhenRightUp;
    
protected:
    virtual void ComputeTotal();
    virtual void ScrollToItemAndCenter(int item);
    bool IsVertical() const;
    bool IsGrid() const;
    
    int ItemWidth() const;
    int ItemHeight() const;
    int ItemsPerLine() const;
    int TotalLines() const;
    
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
