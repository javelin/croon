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
	Check(FormatTime2(3661.9) == "01:01:01", "FormatTime2 handles hour rollover");
	Check(FormatTimeASS(65.12) == "0:01:05.12", "FormatTimeASS uses ASS timestamp precision");
	Check(FormatTimeASS(3661.5) == "1:01:01.50", "FormatTimeASS handles hour rollover");
	Check(StripNonAlnum("A! b-2") == "Ab2", "StripNonAlnum removes punctuation and spaces");

	String freqFilter = Visualization::Filter("@@freqs", "subtitles.ass", true);
	Check(freqFilter.Find("showfreqs") >= 0, "freq visualization uses showfreqs");
	Check(freqFilter.Find("subtitles=subtitles.ass") >= 0, "freq visualization includes subtitles filter");
	Check(Visualization::Filter("@@unknown", "subtitles.ass", true).IsVoid(), "unknown visualization returns void");

	KarData song;
	song.version = "9.9";
	song.title = "Long Song";
	song.artist = "The Singers";
	song.genre = "Pop";
	song.year = 2026;
	song.writer = "A Writer";
	song.owner = "A Publisher";
	song.origVideoFile = "background.mp4";
	song.duration = 3661.5;
	song.timed = 2;
	song.fontSize = 84;
	song.dehiss = true;
	song.timedLyrics.Add({song.duration, ""});
	song.timedLyrics.Add({1.25, "First line"});
	song.timedLyrics.Add({2.5, "Second line"});
	song.parts.Add(MakeTuple(1, true, false, true));

	KarData restored(song.ToJSONStr());
	Check(restored.version == "9.9", "KarData JSON preserves version");
	Check(restored.title == "Long Song", "KarData JSON preserves title");
	Check(restored.artist == "The Singers", "KarData JSON preserves artist");
	Check(restored.genre == "Pop", "KarData JSON preserves genre");
	Check(restored.year == 2026, "KarData JSON preserves year");
	Check(restored.writer == "A Writer", "KarData JSON preserves writer");
	Check(restored.owner == "A Publisher", "KarData JSON preserves owner");
	Check(restored.origVideoFile == "background.mp4", "KarData JSON preserves original video");
	Check(restored.duration == 3661.5, "KarData JSON preserves duration");
	Check(restored.timed == 2, "KarData JSON preserves timed count");
	Check(restored.fontSize == 84, "KarData JSON preserves font size");
	Check(restored.dehiss, "KarData JSON preserves dehiss");
	Check(restored.timedLyrics.GetCount() == 3, "KarData JSON restores sentinel lyric");
	Check(restored.timedLyrics[0].time == restored.duration, "KarData JSON sentinel uses duration");
	Check(restored.timedLyrics[1].lyrics == "First line", "KarData JSON preserves first timed lyric");
	Check(restored.timedLyrics[2].time == 2.5, "KarData JSON preserves second timed lyric time");
	Check(restored.parts.GetCount() == 1, "KarData JSON preserves part count");
	Check(restored.parts[0].a == 1 && restored.parts[0].b && !restored.parts[0].c && restored.parts[0].d,
		"KarData JSON preserves vocal part flags");

	String decorated = ">>{120}  Sing this line  ";
	String decor = SplitLyrics(decorated);
	Check(decor == ">>{120}", "SplitLyrics extracts blank and tempo decoration");
	Check(decorated == "Sing this line", "SplitLyrics trims lyric content");

	KarData lyricsData;
	lyricsData.duration = 42.0;
	lyricsData.rawLyrics = ">>{90} First line\n@Title\n-\nSecond line";
	Vector<TimeLyrics> untimed = RawToUntimedLyrics(lyricsData);
	Check(untimed.GetCount() == 5, "RawToUntimedLyrics keeps sentinel plus non-empty lines");
	Check(untimed[0].time == 42.0, "RawToUntimedLyrics sentinel uses duration");
	Check(untimed[1].lyrics == ">>{90}First line", "RawToUntimedLyrics preserves first-line decoration");
	Check(untimed[2].lyrics == "@Title", "RawToUntimedLyrics keeps metadata lines");
	Check(untimed[3].lyrics == "-", "RawToUntimedLyrics keeps dash placeholders");
	Check(TimedLyricsToRaw(untimed, true) == "First line\nSecond line",
		"TimedLyricsToRaw removes metadata, dash placeholders, and decorations");

	KarData longLineData;
	longLineData.duration = 10.0;
	longLineData.rawLyrics = "{60} " + String('a', MaxLineLength + 1);
	Vector<TimeLyrics> wrapped = RawToUntimedLyrics(longLineData);
	Check(wrapped.GetCount() == 2, "RawToUntimedLyrics does not emit decoration-only wrapped lines");
	Check(wrapped[1].lyrics == "{60}" + String('a', MaxLineLength + 1),
		"RawToUntimedLyrics keeps decoration with overlong first word");

	if(failures)
		SetExitCode(1);
}

