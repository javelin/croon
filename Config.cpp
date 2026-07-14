/*
 * File  : Config.cpp
 * Author: Mark Documento
 */

#include <Core/Core.h>

using namespace Upp;

#include "ConfigService.h"
#include "Config.h"

Config Config::config;

const int Config::DefaultFontSize = ConfigService::DefaultFontSize;
const int Config::MinFontSize = ConfigService::MinFontSize;
const int Config::MaxFontSize = ConfigService::MaxFontSize;

Config::Config() {
}

Config::~Config() {
}

String Config::Get(const String& key, const String& defaultValue) {
    return ConfigService::Get(key, defaultValue);
}

Config& Config::Set(const String& key, String value) {
    ConfigService::Set(key, value);
    return config;
}

int Config::GetInt(const String& key, int defaultValue) {
    return ConfigService::GetInt(key, defaultValue);
}

Config& Config::Set(const String& key, int value) {
    ConfigService::Set(key, value);
    return config;
}

Config& Config::GetConfig() {
    return config;
}

void Config::Serialize() {
    ConfigService::Serialize();
}

int Config::GetFontSize() {
    return ConfigService::GetFontSize();
}
