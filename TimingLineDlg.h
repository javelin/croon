/*
 * File  : TimingLineDlg.h
 * Author: Mark Documento
 */

#ifndef _Croon_TimingLineDlg_h_
#define _Croon_TimingLineDlg_h_

class TimingLineDlg : public WithCroonTimingLineLayout<TopWindow> {
public:
    TimingLineDlg(String decor, String lyrics);
    String GetDecor() const { return decorEd.GetData(); }
    String GetLyrics() const { return lyricsEd.GetData(); }
    
};

#endif
