/*
 * File  : ConfigService.cpp
 * Author: Mark Documento
 */

#include "Croon.h"

#define CONFIG_FILE "config"

ConfigService ConfigService::service(AppendFileName(AppPaths::DataDirectory(), CONFIG_FILE));

const int ConfigService::DefaultFontSize=72;
const int ConfigService::MinFontSize=70;
const int ConfigService::MaxFontSize=100;

ConfigService::ConfigService(const char* fname) : fname(fname) {
    RegisterGlobalConfig(FFMPEG_LOCATION);
    RegisterGlobalConfig(LYRICS_PREFIX);
    RegisterGlobalConfig(LYRICS_SUFFIX);
    RegisterGlobalConfig(FONT_SIZE);
    RegisterGlobalConfig(MUSIC_DIR);
    RegisterGlobalConfig(PROJECT_DIR);
    RegisterGlobalConfig(EXPORT_DIR);
    RegisterGlobalConfig(VIDEO_DIR);
    RegisterGlobalConfig(PROJECT_LIST);
    RegisterGlobalConfig(WIN_X);
    RegisterGlobalConfig(WIN_Y);
    RegisterGlobalConfig(WIN_W);
    RegisterGlobalConfig(WIN_H);
    FileIn f(fname);
    SerializeGlobalConfigs(f);
}

ConfigService::~ConfigService() {
    Serialize();
}

String ConfigService::Get(const String& key, const String& defaultValue) {
    String value;
    return LoadFromGlobal(value, key) ? value : defaultValue;
}

ConfigService& ConfigService::Set(const String& key, String value) {
    StoreToGlobal(value, key);
    return service;
}

int ConfigService::GetInt(const String& key, int defaultValue) {
    int value;
    return LoadFromGlobal(value, key) ? value : defaultValue;
}

ConfigService& ConfigService::Set(const String& key, int value) {
    StoreToGlobal(value, key);
    return service;
}

void ConfigService::Serialize() {
    FileOut f(service.fname);
    SerializeGlobalConfigs(f);
}

int ConfigService::GetFontSize() {
    return std::max(MinFontSize, std::min(MaxFontSize, GetInt(FONT_SIZE, DefaultFontSize)));
}
