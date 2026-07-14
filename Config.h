/*
 * File  : Config.h
 * Author: Mark Documento
 */

#ifndef _Croon_Config_h_
#define _Croon_Config_h_

class Config {
public:
    ~Config();
    static String Get(const String& key, const String& defaultValue=String::GetVoid());
    static Config& Set(const String& key, String value);
    static int GetInt(const String& key, int defaultValue=0);
    static Config& Set(const String& key, int value);
    static Config& GetConfig();
    void Serialize();
    static int GetFontSize();

public:
    static const int DefaultFontSize;
    static const int MinFontSize;
    static const int MaxFontSize;

private:
    Config();
    
private:
    static Config config;
};

#define FFMPEG_LOCATION     "Ffmpeg_Bin"
#define LYRICS_PREFIX       "Lyrics_Prefix"
#define LYRICS_SUFFIX       "Lyrics_Suffix"
#define FONT_SIZE           "Font_Size"
#define MUSIC_DIR           "MusicDir"
#define PROJECT_DIR         "ProjectDir"
#define EXPORT_DIR          "ExportDir"
#define VIDEO_DIR           "VideoDir"
#define PROJECT_LIST        "ProjectList"
#define WIN_X               "WinX"
#define WIN_Y               "WinY"
#define WIN_W               "WinW"
#define WIN_H               "WinH"

#endif
