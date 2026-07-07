/*
 * File  : Page2.h
 * Author: Mark Documento
 */

#ifndef _Croon_Page2_h_
#define _Croon_Page2_h_

#include "Page.h"

struct KarData;

class Page2 : public WithCroonWizardPage2Layout<Page> {
public:
    Page2();
    Page2(KarData& data);
    void Populate() override;
    void SaveData() override;
    void Reset() override;
    
public:
    bool willDownloadLyrics;

private:
    KarData& data;
};

#endif
