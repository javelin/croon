#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_header.h>

#include <Croon/Constants.h>
#include <Croon/AppIdentity.h>
#include <Croon/KarData.h>
#include <Croon/ProjectSerializer.h>
#include <Croon/Util.h>
#include <Croon/Config.h>
#include <Croon/Visualization.h>

typedef struct Visualization VIZ;

#include <Croon/FfmpegCommandBuilder.h>

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
	CheckEq(FfmpegCommandBuilder::ConvertAudioToVorbis("song.mp3", "song.ogg"), {
		"-i", "song.mp3", "-vn", "-acodec", "libvorbis", "song.ogg"
	}, "ConvertAudioToVorbis");

	CheckEq(FfmpegCommandBuilder::DehissAudio("song.ogg", "clean.ogg", 42), {
		"-i", "song.ogg", "-af", "afftdn=nr=42", "clean.ogg"
	}, "DehissAudio");

	CheckEq(FfmpegCommandBuilder::GenerateThumbnail("video.mp4", "thumb.png", 256, 256), {
		"-i", "video.mp4", "-ss", "00:00:06", "-vframes", "1",
		"-vf", "crop='min(iw,ih)':'min(iw,ih)',scale=256:256", "thumb.png"
	}, "GenerateThumbnail");

	CheckEq(FfmpegCommandBuilder::ProjectExtractAudioAndInfo("song.croon", "song.ogg", "song.json"), {
		"-dump_attachment:t:0", "song.json", "-i", "song.croon", "-map", "0:a:0",
		"-c:a", "copy", "song.ogg"
	}, "ProjectExtractAudioAndInfo");

	KarData commandData;
	commandData.version = "2.5";
	commandData.videoFilePath = "video.mp4";
	commandData.audioFilePath = "audio.ogg";
	commandData.infoFilePath = "croon.json";
	CheckEq(FfmpegCommandBuilder::ProjectSaveWithBackgroundVideo(commandData, "song.croon"), {
		"-i", "video.mp4", "-i", "audio.ogg", "-map", "0:v:0", "-map", "1:a:0",
		"-map_metadata:s", "-1", "-attach", "croon.json",
		"-metadata:s:2", "filename=croon.info", "-metadata:s:2", "mimetype=application/json",
		"-c", "copy", "-metadata", "APPLICATION=Croon v2.5", "-f", "matroska", "song.croon"
	}, "ProjectSaveWithBackgroundVideo");

	CheckEq(FfmpegCommandBuilder::ProjectSaveWithVisualization(commandData, "song.croon"), {
		"-i", "audio.ogg", "-map_metadata:s", "-1", "-attach", "croon.json",
		"-metadata:s:1", "filename=croon.info", "-metadata:s:1", "mimetype=application/json",
		"-c", "copy", "-metadata", "APPLICATION=Croon v2.5", "-f", "matroska", "song.croon"
	}, "ProjectSaveWithVisualization");

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

	Check(ProjectSerializer::FormatVersion() == AppIdentity::Version(), "ProjectSerializer format follows app identity version");
	Check(ProjectSerializer::SupportsVersion("1.0"), "ProjectSerializer supports current .croon format");
	Check(!ProjectSerializer::SupportsVersion("0.9"), "ProjectSerializer rejects unknown .croon format version");
	String serialized = ProjectSerializer::ToJson(song);
	const char *projectKeys[] = {
		"\"version\"", "\"title\"", "\"artist\"", "\"genre\"", "\"year\"", "\"writer\"",
		"\"owner\"", "\"origVideoFile\"", "\"duration\"", "\"timed\"", "\"fontSize\"",
		"\"dehiss\"", "\"timedLyrics\"", "\"parts\""
	};
	for(const char *key : projectKeys) {
		Check(serialized.Find(key) >= 0, String("ProjectSerializer writes ") + key);
	}
	Check(serialized.Find("\"rawLyrics\"") < 0, "ProjectSerializer keeps raw lyrics out of project metadata");

	KarData restored = ProjectSerializer::FromJson(serialized);
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
	Check(song.ToJSONStr() == serialized, "KarData ToJSONStr delegates to ProjectSerializer");
	KarData restoredViaKarData(serialized);
	Check(restoredViaKarData.title == restored.title && restoredViaKarData.timedLyrics.GetCount() == restored.timedLyrics.GetCount(),
		"KarData JSON constructor delegates to ProjectSerializer");

	KarData normalized = ProjectSerializer::FromJson("{\"version\":\"1.0\",\"year\":-7,\"fontSize\":999,\"timedLyrics\":[],\"parts\":[]}");
	Check(normalized.year == 0, "ProjectSerializer normalizes negative years");
	Check(normalized.fontSize == Config::DefaultFontSize, "ProjectSerializer keeps font-size clamping behavior");
	Check(normalized.timedLyrics.GetCount() == 1, "ProjectSerializer restores sentinel for empty timed lyrics");

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
