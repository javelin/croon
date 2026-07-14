/*
 * File  : Page.cpp
 * Author: Mark Documento
 */

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#include "Page.h"

Page::Page() : Page("") {
}

Page::Page(String name) : pageName(name), enableNext(false), enablePrev(true) {
    prevBtn.SetLabel("< Back").Disable();
    prevBtn.Hide();
    prevBtn << [=] { WhenPreviousPage(); };
    nextBtn.SetLabel("Next >").Disable();
    nextBtn.Hide();
    nextBtn << [=] { WhenNextPage(); };
    WantFocus(true);
}

Button& Page::GetPrevButton() {
    return prevBtn;
}

Button& Page::GetNextButton() {
    return nextBtn;
}

const String& Page::GetPageName() const {
    return pageName;
}

void Page::HidePage() {
    Disable();
    Hide();
    PageHidden();
}

void Page::ShowPage() {
    Show();
    Enable();
    PageShown();
}

void Page::PageShown() {
    Populate();
    ShowButtons();
    WhenShown();
}

void Page::PageHidden() {
    SaveData();
    HideButtons();
    WhenHidden();
}

void Page::Reset() {
}

void Page::Populate() {
}

void Page::SaveData() {
}

void Page::HideButtons() {
    prevBtn.Disable();
    prevBtn.Hide();
    nextBtn.Disable();
    nextBtn.Hide();
}

void Page::ShowButtons() {
    prevBtn.Enable(enablePrev);
    prevBtn.Show();
    nextBtn.Enable(enableNext);
    nextBtn.Show();
}
