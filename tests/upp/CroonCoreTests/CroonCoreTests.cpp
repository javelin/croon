#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_header.h>

#include <Croon/Constants.h>
#include <Croon/KarData.h>
#include <Croon/Util.h>
#include <Croon/Visualization.h>

typedef struct Visualization VIZ;

#include <Croon/Ffmpeg.h>

namespace {

int failures = 0;

void Check(bool condition, const String& message) {
	if(!condition) {
		Cerr() << "FAIL: " << message << "\n";
		++failures;
	}
}

void CheckEq(const Vector<String>& actual, std::initializer_list<const char*> expected, const String& message) {
	Vector<String> exp;
	for(const char *item : expected)
		exp.Add(item);

	Check(actual.GetCount() == exp.GetCount(), message + " count");
	if(actual.GetCount() != exp.GetCount())
		return;

	for(int i = 0; i < actual.GetCount(); ++i)
		Check(actual[i] == exp[i], Format("%s[%d]", message, i));
}

}

CONSOLE_APP_MAIN
{
	CheckEq(Ffmpeg::ConvertAudioToVorbis("song.mp3", "song.ogg"), {
		"-i", "song.mp3", "-vn", "-acodec", "libvorbis", "song.ogg"
	}, "ConvertAudioToVorbis");

	CheckEq(Ffmpeg::DehissAudio("song.ogg", "clean.ogg", 42), {
		"-i", "song.ogg", "-af", "afftdn=nr=42", "clean.ogg"
	}, "DehissAudio");

	Check(CountInDuration(120) == 0.5, "CountInDuration rounds beat duration");
	Check(FormatTime2(65.9) == "00:01:05", "FormatTime2 truncates seconds");
	Check(FormatTimeASS(65.12) == "0:01:05.12", "FormatTimeASS uses ASS timestamp precision");
	Check(StripNonAlnum("A! b-2") == "Ab2", "StripNonAlnum removes punctuation and spaces");

	String freqFilter = Visualization::Filter("@@freqs", "subtitles.ass", true);
	Check(freqFilter.Find("showfreqs") >= 0, "freq visualization uses showfreqs");
	Check(freqFilter.Find("subtitles=subtitles.ass") >= 0, "freq visualization includes subtitles filter");
	Check(Visualization::Filter("@@unknown", "subtitles.ass", true).IsVoid(), "unknown visualization returns void");

	if(failures)
		SetExitCode(1);
}

