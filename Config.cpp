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

Config::~Config() {
}
