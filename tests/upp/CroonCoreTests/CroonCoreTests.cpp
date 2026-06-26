#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_header.h>

#include <Croon/Constants.h>
#include <Croon/AppIdentity.h>
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

	KarData metaData;
	metaData.title = "Song (Title)";
	metaData.artist = "Artist";
	String titleLine = "@Title";
	ReplaceMetadata(titleLine, metaData);
	Check(titleLine == "Song \\(Title\\)", "ReplaceMetadata escapes parentheses in title");
	String dashLine = "-";
	ReplaceMetadata(dashLine, metaData);
	Check(dashLine == "\u00A0", "ReplaceMetadata converts dash placeholder to nbsp");

	Vector<Tuple<int, bool, bool, bool>> vocalParts;
	vocalParts.Add(MakeTuple(1, true, false, true));
	vocalParts.Add(MakeTuple(2, false, true, true));
	vocalParts.Add(MakeTuple(3, true, true, true));
	Check(ResolveVocalPart(-1, vocalParts) == VP_V1, "ResolveVocalPart defaults missing index to V1");
	Check(ResolveVocalPart(1, vocalParts) == VP_V1, "ResolveVocalPart maps V1-only part");
	Check(ResolveVocalPart(2, vocalParts) == VP_V2, "ResolveVocalPart maps V2-only part");
	Check(ResolveVocalPart(3, vocalParts) == VP_B, "ResolveVocalPart maps dual-voice part");

	Check(ResolveCountInStyle(VP_V2, "Sing now") == "CountInV2", "ResolveCountInStyle uses vocal part color");
	Check(ResolveCountInStyle(VP_V1, "~echo") == "BUC", "ResolveCountInStyle honors backup override");

	String backupLine = "~echo";
	Check(ResolveStyle(VP_V1, backupLine, false) == "BUC", "ResolveStyle maps backup vocals to BUC");
	Check(backupLine.Find("{\\i1}") >= 0, "ResolveStyle wraps backup vocals in italic");

	String miscLine = "(note)";
	Check(ResolveStyle(VP_V1, miscLine, false) == "MC", "ResolveStyle maps parenthetical lines to MC");

	String metaLine = "@Title";
	Check(ResolveStyle(VP_V1, metaLine, false, "", true) == "MC", "ResolveStyle maps metadata to MC");
	Check(!metaLine.StartsWith("@"), "ResolveStyle strips metadata prefix");

	KarData processed;
	processed.duration = 20.0;
	processed.title = "My Title";
	processed.timedLyrics.Add({processed.duration, ""});
	processed.timedLyrics.Add({10.0, ">>>{60} First line"});
	processed.timedLyrics.Add({15.0, "@Title"});
	Vector<TimeLyrics> processedLines;
	ProcessMetadata(processed, processedLines, 4);
	Check(processedLines.GetCount() >= 4, "ProcessMetadata inserts blanks and count-in lines");
	bool foundCountIn = false;
	bool foundMeta = false;
	for(const TimeLyrics& line : processedLines) {
		if(line.lyrics.Find("@CountIn") >= 0)
			foundCountIn = true;
		if(line.isMeta && line.lyrics == processed.title)
			foundMeta = true;
	}
	Check(foundCountIn, "ProcessMetadata emits count-in marker");
	Check(foundMeta, "ProcessMetadata resolves @Title metadata");

	KarData emptyAss;
	Check(TimedToASS(emptyAss).IsEmpty(), "TimedToASS returns empty output without timed lyrics");

	KarData exportData;
	exportData.duration = 10.0;
	exportData.title = "Export Song";
	exportData.artist = "Export Artist";
	exportData.fontSize = 72;
	exportData.timedLyrics.Add({exportData.duration, ""});
	exportData.timedLyrics.Add({1.0, "Sing along"});
	exportData.parts.Add(MakeTuple(1, true, false, true));
	String ass = TimedToASS(exportData, 2);
	Check(ass.Find("[Script Info]") >= 0, "TimedToASS emits script info section");
	Check(ass.Find("Export Song by Export Artist") >= 0, "TimedToASS includes project title");
	Check(ass.Find("Style: V1,") >= 0, "TimedToASS defines V1 style");
	Check(ass.Find("Dialogue: 0,0:00:01.00") >= 0, "TimedToASS emits timed dialogue");
	Check(ass.Find(",V1,,0,0,0,,Sing along") >= 0, "TimedToASS uses V1 style for default vocal part");

	KarData v2Data;
	v2Data.duration = 10.0;
	v2Data.title = "Export Song";
	v2Data.artist = "Export Artist";
	v2Data.fontSize = 72;
	v2Data.timedLyrics.Add({v2Data.duration, ""});
	v2Data.timedLyrics.Add({1.0, "Sing along"});
	v2Data.parts.Add(MakeTuple(1, false, true, true));
	Check(TimedToASS(v2Data, 2).Find(",V2,") >= 0, "TimedToASS uses V2 style for second vocal part");

	KarData bucData;
	bucData.duration = 10.0;
	bucData.title = "Export Song";
	bucData.artist = "Export Artist";
	bucData.fontSize = 72;
	bucData.timedLyrics.Add({bucData.duration, ""});
	bucData.timedLyrics.Add({1.0, "~echo"});
	Check(TimedToASS(bucData, 2).Find(",BUC,") >= 0, "TimedToASS uses BUC style for backup vocals");

	if(failures)
		SetExitCode(1);
}
