/*
 * File  : RichTextBuilder.h
 * Author: Mark Documento
 */

#ifndef _Croon_RichTextBuilder_h_
#define _Croon_RichTextBuilder_h_

struct RTHelper {
    RTHelper& Clear() { vs.Clear(); return *this; }
    RTHelper& Fmt(String s) { vs.Add(Format("[%s ", s)); return *this; }
    RTHelper& EFmt() { vs.Add("]"); return *this; }
    RTHelper& EFmt(String s) { vs.Add(Format("][%s ", s)); return *this; }
    RTHelper& NL() { vs.AddPick("&"); return *this; }
    RTHelper& Text(String s) { vs.Add(DeQtf(s)); return *this; }
    String ToString() const { return Join(vs, ""); }
private:
    Vector<String> vs;
};

#endif
