/*
 * File  : KarData.h
 * Author: Mark Documento
 */

#ifndef _Croon_KarData_h_
#define _Croon_KarData_h_

struct TimeLyrics : Moveable<TimeLyrics>, public Pte<TimeLyrics> {
    TimeLyrics(double time, String lyrics, int partIndex=-1, bool isMeta=false) 
        : time(time), lyrics(lyrics), partIndex(partIndex), isMeta(isMeta) {}
    double time;
    String lyrics;
    int partIndex; // index into KarData::timedLyrics; -1 for inserted lines (blanks, count-ins)
    bool isMeta;   // true for @ metadata lines
};

struct KarData : Moveable<KarData> {
    KarData() { Reset(); }
    KarData(const String& JSONStr);
    void Dump() const;
    void DumpLyrics() const;
    void DumpTimedLyrics() const;
    void Reset();
    String ToJSONStr() const;
    
    bool loaded = false;
    String projectPath;
    String origAudioFilePath;
    String audioFilePath;
    double duration;    // seconds
    int tempo;          // bpm
    String title;
    String artist;
    String writer;
    String owner;
    String genre;
    int year;
    String rawLyrics;
    Vector<TimeLyrics> timedLyrics;
    Vector<Tuple<int, bool, bool, bool>> parts;
    String videoFilePath;
    String origVideoFile;
    String infoFilePath;
    String thumbnailFilePath;
    Image videoThumbnail;
    int timed;
    int fontSize;
    int subtitleLines;
    bool dehiss;
    String version;
    
    void SetFontSize(int size);
    void SetSubtitleLines(int lines);
};

#endif
