#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_header.h>

#include <Croon/Constants.h>
#include <Croon/KarData.h>
#include <Croon/Util.h>
#include <Croon/Config.h>

const int Config::DefaultFontSize = 72;
const int Config::MinFontSize = 70;
const int Config::MaxFontSize = 100;

String Config::Get(const String&, const String& defaultValue) {
	return defaultValue;
}

int Config::GetInt(const String&, int defaultValue) {
	return defaultValue;
}

