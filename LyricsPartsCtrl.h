#ifndef _Croon_LyricsPartsCtrl_h_
#define _Croon_LyricsPartsCtrl_h_

class LyricsPartsCtrl : public Ctrl {
public:
	// row, col[0..2], newState
    Event<int, int, bool> WhenToggle;

    LyricsPartsCtrl();
    void SetLyricsAndParts(Vector<String>&& lyrics,
                               const Vector<Tuple<int, bool, bool, bool>>& parts);
    
    void ClearParts();

    int GetCount() const;

    bool GetBox(int row, int col) const;

    void SetBox(int row, int col, bool on);

    Vector<Tuple<int, bool, bool, bool>> GetParts() const;

    void SetFont(Font f);

    void SetVerticalPadding(int px);

private:
    static constexpr int BOX_COUNT = 2;

    Vector<String> lines;  // pickable / movable in U++
    Vector<dword>  state;  // bit0=box1, bit1=box2, bit2=box3
    Font           font;
    Font           boldFont;

    int            vpad;   // extra vertical spacing per row

    ScrollBar      sb;

    bool dragging;
    int  last_row, last_col;

private:
    bool HasLyrics(int row) const;
    int RowHeight() const;
    
    int HeaderHeight() const;
    int BoxSize() const;
    int BoxesWidth() const;
    void SyncScroll();
    void RefreshRow(int row);
    Rect BoxRect(int row, int col) const;
    bool HitTestBox(Point p, int& out_row, int& out_col) const;
    void ToggleAt(Point p);

public:
    virtual void Layout() override;

    virtual void Paint(Draw& w) override;

    virtual void MouseWheel(Point, int zdelta, dword) override;

    virtual void LeftDown(Point p, dword) override;

    virtual void MouseMove(Point p, dword) override;

    virtual void LeftUp(Point, dword) override;

    virtual void LostCapture();
};


#endif
