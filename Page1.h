/*
 * File  : Page1.h
 * Author: Mark Documento
 */

#ifndef _Croon_Page1_h_
#define _Croon_Page1_h_

class Page1 : public WithCroonWizardPage1Layout<Page> {
public:
    Page1();
    Event<> WhenAudioLoaded;
    Event<String> WhenAudioLoadError;
    Event<> WhenLyricsLoaded;
    void Reset() override;
    void Populate() override;
    void SaveData() override;
};

#endif
