/*
 * File  : Page.h
 * Author: Mark Documento
 */

#ifndef _Croon_Page_h_
#define _Croon_Page_h_

class Page : public Ctrl {
public:
    Page();
    Page(String name);
    Button& GetPrevButton();
    Button& GetNextButton();
    virtual const String& GetPageName() const;
    virtual void HidePage();
    virtual void ShowPage();
    virtual void PageShown();
    virtual void PageHidden();
    virtual void Reset();
    virtual void Populate();
    virtual void SaveData();
    virtual void HideButtons();
    virtual void ShowButtons();
    
    Event<> WhenPreviousPage;
    Event<> WhenNextPage;
    Event<> WhenShown;
    Event<> WhenHidden;
    
protected:
    String pageName;
    Button prevBtn;
    Button nextBtn;
    bool enableNext;
    bool enablePrev;
};

#endif
