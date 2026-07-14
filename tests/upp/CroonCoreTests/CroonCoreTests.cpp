#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_header.h>

#include <Croon/Constants.h>
#include <Croon/AppIdentity.h>
#include <Croon/AzLyricsProvider.h>
#include <Croon/KarData.h>
#include <Croon/LyricsTransformer.h>
#include <Croon/ProjectSerializer.h>
#include <Croon/SubtitleGenerator.h>
#include <Croon/SubtitleLineProcessor.h>
#include <Croon/VocalPart.h>
#include <Croon/TextTools.h>
#include <Croon/TimeFormatter.h>
#include <Croon/ConfigService.h>
#include <Croon/Config.h>
#include <Croon/RecentProjectService.h>
#include <Croon/Visualization.h>

#include <Croon/FfmpegAudioCommandBuilder.h>
#include <Croon/FfmpegExportCommandBuilder.h>
#include <Croon/FfmpegProjectCommandBuilder.h>
#include <Croon/FfmpegThumbnailCommandBuilder.h>

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

String LoadFixture(const char *name) {
	String path = AppendFileName("tests/upp/CroonCoreTests/fixtures", name);
	String content = LoadFile(path);
	Check(!content.IsVoid(), String("loads fixture ") + name);
	return content;
}

}

CONSOLE_APP_MAIN
{
	CheckEq(FfmpegAudioCommandBuilder::ConvertToVorbis("song.mp3", "song.ogg"), {
		"-i", "song.mp3", "-vn", "-acodec", "libvorbis", "song.ogg"
	}, "ConvertToVorbis");

	CheckEq(FfmpegAudioCommandBuilder::Dehiss("song.ogg", "clean.ogg", 42), {
		"-i", "song.ogg", "-af", "afftdn=nr=42", "clean.ogg"
	}, "Dehiss");

	CheckEq(FfmpegThumbnailCommandBuilder::Generate("video.mp4", "thumb.png", 256, 256), {
		"-i", "video.mp4", "-ss", "00:00:06", "-vframes", "1",
		"-vf", "crop='min(iw,ih)':'min(iw,ih)',scale=256:256", "thumb.png"
	}, "GenerateThumbnail");

	CheckEq(FfmpegProjectCommandBuilder::ExtractAudioAndInfo("song.croon", "song.ogg", "song.json"), {
		"-dump_attachment:t:0", "song.json", "-i", "song.croon", "-map", "0:a:0",
		"-c:a", "copy", "song.ogg"
	}, "ExtractAudioAndInfo");

	KarData commandData;
	commandData.version = "2.5";
	commandData.videoFilePath = "video.mp4";
	commandData.audioFilePath = "audio.ogg";
	commandData.infoFilePath = "croon.json";
	CheckEq(FfmpegExportCommandBuilder::WithBackgroundVideo(commandData, "song.ass", "song.mp4", "clean.ogg", false), {
		"-stream_loop", "-1", "-i", "video.mp4", "-i", "clean.ogg", "-map_metadata:s", "-1",
		"-filter_complex", "[0:v]subtitles=song.ass[v]", "-map", "[v]", "-map", "1:a",
		"-shortest", "-c:v", "libx264", "-c:a", "copy",
		"-metadata", "title=", "-metadata", "artist=", "-metadata", "composer=",
		"-metadata", "copyright=", "-metadata", "genre=", "-metadata", "year=0",
		"-metadata", "comment=Year: 0\nOriginal Video: ", "-metadata", "lyrics=", "song.mp4"
	}, "WithBackgroundVideo");

	CheckEq(FfmpegProjectCommandBuilder::SaveWithBackgroundVideo(commandData, "song.croon"), {
		"-i", "video.mp4", "-i", "audio.ogg", "-map", "0:v:0", "-map", "1:a:0",
		"-map_metadata:s", "-1", "-attach", "croon.json",
		"-metadata:s:2", "filename=croon.info", "-metadata:s:2", "mimetype=application/json",
		"-c", "copy", "-metadata", "APPLICATION=Croon v2.5", "-f", "matroska", "song.croon"
	}, "SaveWithBackgroundVideo");

	CheckEq(FfmpegProjectCommandBuilder::SaveWithVisualization(commandData, "song.croon"), {
		"-i", "audio.ogg", "-map_metadata:s", "-1", "-attach", "croon.json",
		"-metadata:s:1", "filename=croon.info", "-metadata:s:1", "mimetype=application/json",
		"-c", "copy", "-metadata", "APPLICATION=Croon v2.5", "-f", "matroska", "song.croon"
	}, "SaveWithVisualization");

	Check(TimeFormatter::CountInDuration(120) == 0.5, "TimeFormatter rounds beat duration");
	Check(TimeFormatter::Clock(65.9) == "00:01:05", "TimeFormatter truncates clock seconds");
	Check(TimeFormatter::Clock(3661.9) == "01:01:01", "TimeFormatter handles hour rollover");
	Check(TimeFormatter::Ass(65.12) == "0:01:05.12", "TimeFormatter uses ASS timestamp precision");
	Check(TimeFormatter::Ass(3661.5) == "1:01:01.50", "TimeFormatter handles ASS hour rollover");
	Check(TextTools::StripNonAlnum("A! b-2") == "Ab2", "TextTools removes punctuation and spaces");
	Check(String(AzLyricsProvider::Name()) == "AZLyrics", "AzLyricsProvider exposes provider name");
	Check(AzLyricsProvider::BuildUrl("The Song!", "Artist") == "https://www.azlyrics.com/lyrics/artist/thesong.html",
		"AzLyricsProvider builds sanitized URL");
	Check(AzLyricsProvider::BuildUrl("The Song!", "The Artist") == "https://www.azlyrics.com/lyrics/artist/thesong.html",
		"AzLyricsProvider strips leading artist prefix");
	String extractedLyrics;
	Check(AzLyricsProvider::ExtractLyrics("before<!-- Usage of azlyrics.com content by any third-party is prohibited. -->First<br>Second</div>after", extractedLyrics),
		"AzLyricsProvider extracts provider lyric block");
	Check(extractedLyrics == "First\nSecond", "AzLyricsProvider trims extracted lyric lines");
	Check(!AzLyricsProvider::ExtractLyrics("<html>No lyrics here</html>", extractedLyrics),
		"AzLyricsProvider rejects pages without a lyric block");
	Check(extractedLyrics.IsEmpty(), "AzLyricsProvider clears output on extraction failure");

	String freqFilter = Visualization::Filter("@@freqs", "subtitles.ass", true);
	Check(freqFilter.Find("showfreqs") >= 0, "freq visualization uses showfreqs");
	Check(freqFilter.Find("subtitles=subtitles.ass") >= 0, "freq visualization includes subtitles filter");
	Check(Visualization::Filter("@@unknown", "subtitles.ass", true).IsVoid(), "unknown visualization returns void");
	Check(Config::GetFontSize() == ConfigService::DefaultFontSize, "Config facade preserves default font size");
	Vector<String> recentPaths;
	recentPaths.Add(" /tmp/song-a.croon ");
	recentPaths.Add("");
	recentPaths.Add("/tmp/song-b.croon");
	recentPaths.Add("/tmp/song-a.croon");
	Vector<String> normalizedPaths = RecentProjectService::NormalizePaths(recentPaths);
	Check(normalizedPaths.GetCount() == 2, "RecentProjectService removes empty and duplicate paths");
	Check(normalizedPaths[0] == "/tmp/song-a.croon", "RecentProjectService trims paths");
	Check(RecentProjectService::FindPathIndex(normalizedPaths, "/tmp/song-b.croon") == 1,
		"RecentProjectService finds normalized path index");
	Check(RecentProjectService::FindPathIndex(normalizedPaths, "/tmp/missing.croon") < 0,
		"RecentProjectService returns negative index for missing paths");

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
	Check(ProjectSerializer::SupportsVersion(""), "ProjectSerializer supports legacy unversioned .croon metadata");
	Check(!ProjectSerializer::SupportsVersion("0.9"), "ProjectSerializer rejects unknown .croon format version");
	Check(ProjectSerializer::ReadVersion("{\"version\":\"1.0\",\"timedLyrics\":[],\"parts\":[]}") == ProjectSerializer::FormatVersion(),
		"ProjectSerializer reads current metadata version directly");
	Check(ProjectSerializer::ReadVersion("{\"timedLyrics\":[],\"parts\":[]}") == ProjectSerializer::FormatVersion(),
		"ProjectSerializer reads legacy unversioned metadata as current");
	Check(ProjectSerializer::ReadVersion("{\"version\":\"9.9\",\"timedLyrics\":[],\"parts\":[]}") == "9.9",
		"ProjectSerializer preserves unsupported explicit metadata version on direct read");
	Check(ProjectSerializer::ReadVersion("{\"version\":\"1.0\",").IsVoid(),
		"ProjectSerializer returns void version for invalid metadata");
	Check(ProjectSerializer::ReadVersion("[{\"version\":\"1.0\"}]").IsVoid(),
		"ProjectSerializer returns void version for non-object metadata");
	Check(!ProjectSerializer::SupportsVersion(ProjectSerializer::ReadVersion("{\"version\":\"9.9\",\"timedLyrics\":[],\"parts\":[]}")),
		"ProjectSerializer reports unsupported direct-read metadata versions");
	Check(ProjectSerializer::SupportsJson("{\"version\":\"1.0\",\"timedLyrics\":[],\"parts\":[]}"),
		"ProjectSerializer supports current project metadata JSON");
	Check(ProjectSerializer::SupportsJson("{\"timedLyrics\":[],\"parts\":[]}"),
		"ProjectSerializer supports legacy unversioned project metadata JSON");
	Check(!ProjectSerializer::SupportsJson("{\"version\":\"9.9\",\"timedLyrics\":[],\"parts\":[]}"),
		"ProjectSerializer rejects unsupported explicit project metadata JSON");
	Check(ProjectSerializer::ReadCompatibility("{\"version\":\"1.0\",\"timedLyrics\":[],\"parts\":[]}") == ProjectSerializer::CurrentMetadata,
		"ProjectSerializer classifies current project metadata");
	Check(ProjectSerializer::ReadCompatibility("{\"timedLyrics\":[],\"parts\":[]}") == ProjectSerializer::LegacyUnversionedMetadata,
		"ProjectSerializer classifies legacy unversioned project metadata");
	Check(ProjectSerializer::ReadCompatibility("{\"version\":\"9.9\",\"timedLyrics\":[],\"parts\":[]}") == ProjectSerializer::UnsupportedMetadata,
		"ProjectSerializer classifies unsupported explicit project metadata");
	Check(ProjectSerializer::ReadCompatibility("{\"version\":\"1.0\",") == ProjectSerializer::InvalidMetadata,
		"ProjectSerializer classifies invalid project metadata");
	Check(ProjectSerializer::ReadCompatibility("[{\"version\":\"1.0\"}]") == ProjectSerializer::InvalidMetadata,
		"ProjectSerializer rejects array-shaped project metadata");
	Check(ProjectSerializer::ReadCompatibility("\"not a project\"") == ProjectSerializer::InvalidMetadata,
		"ProjectSerializer rejects string-shaped project metadata");
	Check(ProjectSerializer::CompatibilityLabel(ProjectSerializer::CurrentMetadata) == "current",
		"ProjectSerializer labels current project metadata");
	Check(ProjectSerializer::CompatibilityLabel(ProjectSerializer::LegacyUnversionedMetadata) == "legacy-unversioned",
		"ProjectSerializer labels legacy unversioned project metadata");
	Check(ProjectSerializer::CompatibilityLabel(ProjectSerializer::UnsupportedMetadata) == "unsupported",
		"ProjectSerializer labels unsupported project metadata");
	Check(ProjectSerializer::CompatibilityLabel(ProjectSerializer::InvalidMetadata) == "invalid",
		"ProjectSerializer labels invalid project metadata");
	String currentMetadataFixture = LoadFixture("current-project-metadata.json");
	String legacyMetadataFixture = LoadFixture("legacy-unversioned-project-metadata.json");
	String unsupportedMetadataFixture = LoadFixture("unsupported-project-metadata.json");
	String invalidMetadataFixture = LoadFixture("invalid-project-metadata.json");
	Check(ProjectSerializer::ReadCompatibility(currentMetadataFixture) == ProjectSerializer::CurrentMetadata,
		"ProjectSerializer classifies current metadata fixture");
	Check(ProjectSerializer::ReadCompatibility(legacyMetadataFixture) == ProjectSerializer::LegacyUnversionedMetadata,
		"ProjectSerializer classifies legacy unversioned metadata fixture");
	Check(ProjectSerializer::ReadCompatibility(unsupportedMetadataFixture) == ProjectSerializer::UnsupportedMetadata,
		"ProjectSerializer classifies unsupported metadata fixture");
	Check(ProjectSerializer::ReadCompatibility(invalidMetadataFixture) == ProjectSerializer::InvalidMetadata,
		"ProjectSerializer classifies invalid metadata fixture");
	Check(ProjectSerializer::SupportsJson(currentMetadataFixture), "ProjectSerializer supports current metadata fixture");
	Check(ProjectSerializer::SupportsJson(legacyMetadataFixture), "ProjectSerializer supports legacy metadata fixture");
	Check(!ProjectSerializer::SupportsJson(unsupportedMetadataFixture), "ProjectSerializer rejects unsupported metadata fixture");
	Check(!ProjectSerializer::SupportsJson(invalidMetadataFixture), "ProjectSerializer rejects invalid metadata fixture");
	Check(!ProjectSerializer::SupportsJson("[{\"version\":\"1.0\"}]"), "ProjectSerializer rejects array-shaped metadata JSON");
	KarData fixtureCurrent = ProjectSerializer::FromJson(currentMetadataFixture);
	Check(fixtureCurrent.version == ProjectSerializer::FormatVersion(), "ProjectSerializer loads current metadata fixture version");
	Check(fixtureCurrent.title == "Fixture Current Song", "ProjectSerializer loads current metadata fixture title");
	KarData fixtureLegacy = ProjectSerializer::FromJson(legacyMetadataFixture);
	Check(fixtureLegacy.version == ProjectSerializer::FormatVersion(), "ProjectSerializer loads legacy metadata fixture as current version");
	String serialized = ProjectSerializer::ToJson(song);
	const char *projectKeys[] = {
		"\"version\"", "\"title\"", "\"artist\"", "\"genre\"", "\"year\"", "\"writer\"",
		"\"owner\"", "\"origVideoFile\"", "\"duration\"", "\"timed\"", "\"fontSize\"",
		"\"dehiss\"", "\"timedLyrics\"", "\"parts\""
	};
	for(const char *key : projectKeys) {
		Check(serialized.Find(key) >= 0, String("ProjectSerializer writes ") + key);
	}
	Check(serialized.Find("\"version\":\"" + ProjectSerializer::FormatVersion() + "\"") >= 0,
		"ProjectSerializer stamps current format version on save");
	Check(serialized.Find("\"rawLyrics\"") < 0, "ProjectSerializer keeps raw lyrics out of project metadata");

	KarData restored = ProjectSerializer::FromJson(serialized);
	Check(restored.version == ProjectSerializer::FormatVersion(), "KarData JSON restores saved format version");
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

	KarData loadedLegacy = ProjectSerializer::FromJson("{\"timedLyrics\":[],\"parts\":[]}");
	Check(loadedLegacy.version == ProjectSerializer::FormatVersion(), "ProjectSerializer normalizes unversioned metadata to current format");
	KarData loadedUnsupported = ProjectSerializer::FromJson("{\"version\":\"9.9\",\"timedLyrics\":[],\"parts\":[]}");
	Check(loadedUnsupported.version == "9.9", "ProjectSerializer preserves unsupported source version on read");
	KarData invalidMetadata = ProjectSerializer::FromJson(invalidMetadataFixture);
	Check(invalidMetadata.version.IsVoid(), "ProjectSerializer marks invalid metadata hydration with void version");
	Check(invalidMetadata.timedLyrics.IsEmpty(), "ProjectSerializer does not hydrate invalid metadata lyrics");
	KarData nonObjectMetadata = ProjectSerializer::FromJson("[{\"version\":\"1.0\"}]");
	Check(nonObjectMetadata.version.IsVoid(), "ProjectSerializer marks non-object metadata hydration with void version");
	KarData normalized = ProjectSerializer::FromJson("{\"version\":\"1.0\",\"year\":-7,\"fontSize\":999,\"timedLyrics\":[],\"parts\":[]}");
	Check(normalized.year == 0, "ProjectSerializer normalizes negative years");
	Check(normalized.fontSize == Config::DefaultFontSize, "ProjectSerializer keeps font-size clamping behavior");
	Check(normalized.timedLyrics.GetCount() == 1, "ProjectSerializer restores sentinel for empty timed lyrics");

	String decorated = ">>{120}  Sing this line  ";
	String decor = LyricsTransformer::SplitDecorations(decorated);
	Check(decor == ">>{120}", "LyricsTransformer extracts blank and tempo decoration");
	Check(decorated == "Sing this line", "LyricsTransformer trims lyric content");

	KarData lyricsData;
	lyricsData.duration = 42.0;
	lyricsData.rawLyrics = ">>{90} First line\n@Title\n-\nSecond line";
	Vector<TimeLyrics> untimed = LyricsTransformer::RawToUntimed(lyricsData);
	Check(untimed.GetCount() == 5, "LyricsTransformer keeps sentinel plus non-empty lines");
	Check(untimed[0].time == 42.0, "LyricsTransformer sentinel uses duration");
	Check(untimed[1].lyrics == ">>{90}First line", "LyricsTransformer preserves first-line decoration");
	Check(untimed[2].lyrics == "@Title", "LyricsTransformer keeps metadata lines");
	Check(untimed[3].lyrics == "-", "LyricsTransformer keeps dash placeholders");
	Check(LyricsTransformer::TimedToRaw(untimed, true) == "First line\nSecond line",
		"LyricsTransformer removes metadata, dash placeholders, and decorations");

	KarData longLineData;
	longLineData.duration = 10.0;
	longLineData.rawLyrics = "{60} " + String('a', MaxLineLength + 1);
	Vector<TimeLyrics> wrapped = LyricsTransformer::RawToUntimed(longLineData);
	Check(wrapped.GetCount() == 2, "LyricsTransformer does not emit decoration-only wrapped lines");
	Check(wrapped[1].lyrics == "{60}" + String('a', MaxLineLength + 1),
		"LyricsTransformer keeps decoration with overlong first word");

	KarData metaData;
	metaData.title = "Song (Title)";
	metaData.artist = "Artist";
	String titleLine = "@Title";
	SubtitleLineProcessor::ReplaceMetadata(titleLine, metaData);
	Check(titleLine == "Song \\(Title\\)", "SubtitleLineProcessor escapes parentheses in title");
	String dashLine = "-";
	SubtitleLineProcessor::ReplaceMetadata(dashLine, metaData);
	Check(dashLine == "\u00A0", "SubtitleLineProcessor converts dash placeholder to nbsp");

	Vector<Tuple<int, bool, bool, bool>> vocalParts;
	vocalParts.Add(MakeTuple(1, true, false, true));
	vocalParts.Add(MakeTuple(2, false, true, true));
	vocalParts.Add(MakeTuple(3, true, true, true));
	Check(SubtitleLineProcessor::ResolveVocalPart(-1, vocalParts) == VP_V1, "SubtitleLineProcessor defaults missing index to V1");
	Check(SubtitleLineProcessor::ResolveVocalPart(1, vocalParts) == VP_V1, "SubtitleLineProcessor maps V1-only part");
	Check(SubtitleLineProcessor::ResolveVocalPart(2, vocalParts) == VP_V2, "SubtitleLineProcessor maps V2-only part");
	Check(SubtitleLineProcessor::ResolveVocalPart(3, vocalParts) == VP_B, "SubtitleLineProcessor maps dual-voice part");
	Check(VocalPartStyle::FromParts(true, false) == VP_V1, "VocalPartStyle maps lyrics dialog V1 assignments without part3");
	Check(VocalPartStyle::FromParts(false, true) == VP_V2, "VocalPartStyle maps lyrics dialog V2 assignments without part3");
	Check(VocalPartStyle::Next(VP_NONE) == VP_V1, "VocalPartStyle cycles unassigned to V1");
	Check(VocalPartStyle::Next(VP_B) == VP_NONE, "VocalPartStyle cycles both singers to unassigned");
	Check(VocalPartStyle::ToParts(4, VP_B) == MakeTuple(4, true, true, true), "VocalPartStyle preserves tuple storage contract");
	Check(VocalPartStyle::FromParts(false, false) == VP_NONE, "VocalPartStyle exposes unassigned editor state");

	Check(SubtitleLineProcessor::ResolveCountInStyle(VP_V2, "Sing now") == "CountInV2", "SubtitleLineProcessor uses vocal part color");
	Check(SubtitleLineProcessor::ResolveCountInStyle(VP_V1, "~echo") == "BUC", "SubtitleLineProcessor honors backup override");

	String backupLine = "~echo";
	Check(SubtitleLineProcessor::ResolveStyle(VP_V1, backupLine, false) == "BUC", "SubtitleLineProcessor maps backup vocals to BUC");
	Check(backupLine.Find("{\\i1}") >= 0, "SubtitleLineProcessor wraps backup vocals in italic");

	String miscLine = "(note)";
	Check(SubtitleLineProcessor::ResolveStyle(VP_V1, miscLine, false) == "MC", "SubtitleLineProcessor maps parenthetical lines to MC");

	String metaLine = "@Title";
	Check(SubtitleLineProcessor::ResolveStyle(VP_V1, metaLine, false, "", true) == "MC", "SubtitleLineProcessor maps metadata to MC");
	Check(!metaLine.StartsWith("@"), "SubtitleLineProcessor strips metadata prefix");

	KarData processed;
	processed.duration = 20.0;
	processed.title = "My Title";
	processed.timedLyrics.Add({processed.duration, ""});
	processed.timedLyrics.Add({10.0, ">>>{60} First line"});
	processed.timedLyrics.Add({15.0, "@Title"});
	Vector<TimeLyrics> processedLines;
	SubtitleLineProcessor::ProcessMetadata(processed, processedLines, 4);
	Check(processedLines.GetCount() >= 4, "SubtitleLineProcessor inserts blanks and count-in lines");
	bool foundCountIn = false;
	bool foundMeta = false;
	for(const TimeLyrics& line : processedLines) {
		if(line.lyrics.Find("@CountIn") >= 0)
			foundCountIn = true;
		if(line.isMeta && line.lyrics == processed.title)
			foundMeta = true;
	}
	Check(foundCountIn, "SubtitleLineProcessor emits count-in marker");
	Check(foundMeta, "SubtitleLineProcessor resolves @Title metadata");

	KarData emptyAss;
	Check(SubtitleGenerator::ToAss(emptyAss).IsEmpty(), "SubtitleGenerator returns empty output without timed lyrics");

	KarData exportData;
	exportData.duration = 10.0;
	exportData.title = "Export Song";
	exportData.artist = "Export Artist";
	exportData.fontSize = 72;
	exportData.timedLyrics.Add({exportData.duration, ""});
	exportData.timedLyrics.Add({1.0, "Sing along"});
	exportData.timedLyrics.Add({3.0, "Next line"});
	exportData.timedLyrics.Add({5.0, "Second next"});
	exportData.parts.Add(MakeTuple(1, true, false, true));
	String ass = SubtitleGenerator::ToAss(exportData, 4);
	Check(ass.Find("[Script Info]") >= 0, "SubtitleGenerator emits script info section");
	Check(ass.Find("Export Song by Export Artist") >= 0, "SubtitleGenerator includes project title");
	Check(ass.Find("Style: V1,") >= 0, "SubtitleGenerator defines V1 style");
	Check(ass.Find("Dialogue: 0,0:00:01.00") >= 0, "SubtitleGenerator emits timed dialogue");
	Check(ass.Find(",V1,,0,0,0,,{\\an2\\move(") >= 0, "SubtitleGenerator positions highlighted lines with motion");
	Check(ass.Find("Sing along") >= 0, "SubtitleGenerator preserves highlighted lyric text");
	Check(ass.Find("\\move(") >= 0, "SubtitleGenerator emits scrolling ASS movement tags");
	String richAss = SubtitleGenerator::ToRichAss(exportData, 4);
	Check(richAss.Find("@4") >= 0, "SubtitleGenerator emits rich ASS formatting");
	Check(richAss.Find("Script Info") >= 0, "SubtitleGenerator emits rich ASS script info");

	KarData v2Data;
	v2Data.duration = 10.0;
	v2Data.title = "Export Song";
	v2Data.artist = "Export Artist";
	v2Data.fontSize = 72;
	v2Data.timedLyrics.Add({v2Data.duration, ""});
	v2Data.timedLyrics.Add({1.0, "Sing along"});
	v2Data.parts.Add(MakeTuple(1, false, true, true));
	Check(SubtitleGenerator::ToAss(v2Data, 2).Find(",V2,") >= 0,
		"SubtitleGenerator uses V2 style for second vocal part");

	KarData bucData;
	bucData.duration = 10.0;
	bucData.title = "Export Song";
	bucData.artist = "Export Artist";
	bucData.fontSize = 72;
	bucData.timedLyrics.Add({bucData.duration, ""});
	bucData.timedLyrics.Add({1.0, "~echo"});
	Check(SubtitleGenerator::ToAss(bucData, 2).Find(",BUC,") >= 0,
		"SubtitleGenerator uses BUC style for backup vocals");

	if(failures)
		SetExitCode(1);
}
