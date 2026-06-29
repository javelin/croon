#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_header.h>

#include <Croon/Constants.h>
#include <Croon/KarData.h>
#include <Croon/ConfigService.h>
#include <Croon/Config.h>

const int ConfigService::DefaultFontSize = 72;
const int ConfigService::MinFontSize = 70;
const int ConfigService::MaxFontSize = 100;

const int Config::DefaultFontSize = 72;
const int Config::MinFontSize = 70;
const int Config::MaxFontSize = 100;

String ConfigService::Get(const String&, const String& defaultValue) {
	return defaultValue;
}

ConfigService& ConfigService::Set(const String&, String) {
	static char serviceStorage[sizeof(ConfigService)];
	return *reinterpret_cast<ConfigService *>(serviceStorage);
}

int ConfigService::GetInt(const String&, int defaultValue) {
	return defaultValue;
}

int ConfigService::GetFontSize() {
	return DefaultFontSize;
}
