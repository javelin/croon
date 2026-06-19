/*
 * File  : Page2.h
 * Author: Mark Documento
 */

#ifndef _Croon_Page2_h_
#define _Croon_Page2_h_

#include "Page.h"

class Page2 : public WithCroonWizardPage2Layout<Page> {
public:
    Page2();
    void Populate() override;
    void SaveData() override;
    void Reset() override;
    
public:
    bool willDownloadLyrics;
};

#endif
