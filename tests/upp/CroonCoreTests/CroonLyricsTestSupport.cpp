#include <CtrlLib/CtrlLib.h>
#include <plugin/pcre/Pcre.h>

using namespace Upp;

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_header.h>

#include <Croon/Constants.h>
#include <Croon/KarData.h>
#include <Croon/Util.h>

String SplitLyrics(String& line) {
	String decor;
	while(line.StartsWith(">")) {
		decor += '>';
		line.Remove(0, 1);
	}
	RegExp rxp("{\\s*(\\d*)\\s*}(.*)");
	if(rxp.Study() && rxp.Match(line)) {
		decor += "{" + rxp.GetString(0) + "}";
		line = rxp.GetString(1);
	}
	line = TrimBoth(line);
	return decor;
}

Vector<TimeLyrics> RawToUntimedLyrics(const KarData& data) {
	Vector<TimeLyrics> lyrics{TimeLyrics(data.duration, "")};
	for(String line : Split(data.rawLyrics, '\n')) {
		line = TrimBoth(line);
		auto decor = SplitLyrics(line);
		if(line.IsEmpty())
			continue;

		String current;
		for(String word : Split(line, ' ')) {
			if(current.GetLength() + word.GetLength() + 1 > MaxLineLength) {
				lyrics.Add(TimeLyrics(0.0f, decor + TrimBoth(current)));
				decor.Clear();
				current = word;
			}
			else {
				current += " ";
				current += word;
			}
		}
		if(!current.IsEmpty())
			lyrics.Add(TimeLyrics(0.0f, decor + TrimBoth(current)));
	}
	return lyrics;
}

String TimedLyricsToRaw(const Vector<TimeLyrics>& vtl, bool removeMetadata) {
	Vector<String> lines;
	for(int i = 1; i < vtl.GetCount(); ++i) {
		auto lyrics = vtl[i].lyrics;
		if(removeMetadata) {
			if(lyrics == "-")
				lyrics.Clear();
			else
				(void)SplitLyrics(lyrics);
			if(!lyrics.IsEmpty() && !lyrics.StartsWith("@"))
				lines.Add(lyrics);
		}
		else {
			lines.Add(lyrics);
		}
	}
	return Join(lines, "\n");
}

