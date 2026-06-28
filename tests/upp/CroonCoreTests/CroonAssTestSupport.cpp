#include <CtrlLib/CtrlLib.h>
#include <plugin/pcre/Pcre.h>

using namespace Upp;

#define IMAGECLASS CroonImg
#define IMAGEFILE <Croon/Croon.iml>
#include <Draw/iml_header.h>

#include <Croon/Constants.h>
#include <Croon/KarData.h>
#include <Croon/SubtitleGenerator.h>
#include <Croon/TimeFormatter.h>
#include <Croon/Util.h>

#define RED_H               "&H000000FF"
#define RED_D               "&H00000080"
#define STRAWBERRY_RED_H    "&H005350FA"
#define STRAWBERRY_RED_D    "&H004644D9"
#define MALACHITE_H         "&H0051DA0B"
#define MALACHITE_D         "&H0040AA0A"
#define BLUE_H              "&H00FF0000"
#define LIGHT_BLUE_H        "&H00FFD590"
#define LIGHT_BLUE_D        "&H00FFB957"
#define PURPLE_H            "&H00FF00FF"
#define ORCHID_H            "&H00E980ED"
#define ORCHID_D            "&H00C66DC9"
#define MAIZE_H             "&H005DECFB"
#define MAIZE_D             "&H0053CFDC"
#define GRAY                "&H00808080"

String& ReplaceMetadata(String& line, const KarData& data, bool replaceDash) {
	auto copyright{data.owner.IsEmpty() ? String(""):
	                                    Format("Copyright \u00A9 %s%s",
	                                            data.year > 0 ? IntStr(data.year) + " ":"",
	                                            data.owner)};
	if(replaceDash && line == "-")
		line = "\u00A0";
	else {
		static auto escape = [](const auto &text) {
			auto escaped{text};
			escaped.Replace("(", "\\(");
			escaped.Replace(")", "\\)");
			return escaped;
		};
		line.Replace("@Title", escape(data.title));
		line.Replace("@Artist", escape(data.artist));
		line.Replace("@Genre", escape(data.genre));
		line.Replace("@Writer", escape(data.writer));
		line.Replace("@Copyright", escape(copyright));
		line.Replace("@Owner", escape(data.owner));
	}
	return line;
}

void ProcessMetadata(const KarData& data, Vector<TimeLyrics>& vtl, int linesToDisplay) {
	vtl.Clear();
	vtl.Add({data.duration, "", -1});
	static const auto blankLine = "\u00A0";
	static const auto defaultTempo = 60;
	double lastTs = 0.0f;
	int maxBlanks = linesToDisplay - 1;
	for(int idx = 1; idx < data.timedLyrics.GetCount(); ++idx) {
		const auto& vt = data.timedLyrics[idx];
		auto lyrics = vt.lyrics;
		auto ts = vt.time;
		auto blanks = 0;
		auto bpm = 0;
		RegExp rxBlank{"^(>+)(.+)"};
		if(rxBlank.Study() && rxBlank.Match(lyrics)) {
			blanks = min(rxBlank.GetString(0).GetLength(), maxBlanks);
			lyrics = TrimBoth(rxBlank.GetString(1));
		}
		RegExp rxTempo{"^{(.*)}(.+)"};
		if(rxTempo.Study() && rxTempo.Match(lyrics)) {
			lyrics = TrimBoth(rxTempo.GetString(1));
			auto tempo = rxTempo.GetString(0);
			bpm = StrInt(tempo.IsEmpty() ? "0":tempo);
			if(bpm == 0)
				bpm = defaultTempo;
			else if(bpm < 0 || bpm > 300)
				bpm = 0;
		}
		if(lyrics.IsEmpty())
			continue;
		if(bpm > 0) {
			auto countIn = TimeFormatter::CountInDuration(bpm);
			auto totalDec = countIn*3 + CountInDelay;
			if(ts - totalDec > lastTs) {
				for(int i = 0; i < blanks; ++i)
					vtl.Add({ts - totalDec, blankLine, -1});
				int csec = countIn*100;
				vtl.Add({ts - totalDec, Format("@CountIn{\\k%d}\u00A0{\\k%d}3... {\\k%d}2... {\\k%d}1...",
				                                (int)CountInDelay*100, csec, csec, csec), -1});
			}
			else {
				for(int i = 0; i < blanks; ++i)
					vtl.Add({ts, blankLine, -1});
			}
		}
		else {
			for(int i = 0; i < blanks; ++i)
				vtl.Add({ts, blankLine, -1});
		}
		bool isMeta = false;
		if(lyrics.StartsWith("\\@")) {
			lyrics.Remove(0, 1);
		}
		else if(lyrics.StartsWith("@")) {
			isMeta = true;
		}
		vtl.Add({ts, ReplaceMetadata(lyrics, data), idx, isMeta});
		lastTs = ts;
	}
}

VocalPart ResolveVocalPart(int partIndex,
                                   const Vector<Tuple<int,bool,bool,bool>>& parts) {
	if(partIndex < 0)
		return VP_V1;
	for(const auto& p : parts) {
		if(p.a == partIndex) {
			bool v1 = p.b;
			bool v2 = p.c;
			if(v1 && v2)
				return VP_B;
			if(v2)
				return VP_V2;
			return VP_V1;
		}
	}
	return VP_V1;
}

String ResolveCountInStyle(VocalPart part, const String& incomingLyrics) {
	if(incomingLyrics.StartsWith("~"))
		return "BUC";
	if(incomingLyrics.StartsWith("("))
		return "MC";
	switch(part) {
	case VP_V2: return "CountInV2";
	case VP_B:  return "CountInB";
	default:    return "CountInV1";
	}
}

String ResolveStyle(VocalPart part, String& line, bool isCountIn,
                            const String& incomingLyrics, bool isMeta) {
	if(isCountIn)
		return ResolveCountInStyle(part, incomingLyrics);
	if(isMeta) {
		if(line.StartsWith("@"))
			line.Remove(0, 1);
		return "MC";
	}
	if(line.StartsWith("~")) {
		line.Remove(0);
		line = Format("{\\i1}%s{\\i0}", line);
		return "BUC";
	}
	if(line.StartsWith("(")) {
		line = Format("{\\i1}%s{\\i0}", line);
		return "MC";
	}
	switch(part) {
	case VP_V2: return "V2";
	case VP_B:  return "B";
	default:    return "V1";
	}
}

String ResolveDimStyle(VocalPart part, String& line, bool isMeta) {
	if(isMeta) {
		if(line.StartsWith("@"))
			line.Remove(0, 1);
		return "MCNormal";
	}
	if(line.StartsWith("~")) {
		line.Remove(0);
		line = Format("{\\i1}%s{\\i0}", line);
		return "BUCNormal";
	}
	if(line.StartsWith("(")) {
		line = Format("{\\i1}%s{\\i0}", line);
		return "MCNormal";
	}
	switch(part) {
	case VP_V2: return "V2Normal";
	case VP_B:  return "BNormal";
	default:    return "V1Normal";
	}
}

VocalPart LookaheadVocalPart(const Vector<TimeLyrics>& vtl, int startIdx,
                                     const Vector<Tuple<int,bool,bool,bool>>& parts,
                                     String& outLyrics) {
	for(int k = startIdx; k < vtl.GetCount(); ++k) {
		if(vtl[k].partIndex >= 0) {
			outLyrics = TrimBoth(vtl[k].lyrics);
			return ResolveVocalPart(vtl[k].partIndex, parts);
		}
	}
	outLyrics = "";
	return VP_V1;
}

String SubtitleGenerator::ToAss(const KarData& data, int linesToDisplay, int resX, int resY) {
	if(data.timedLyrics.IsEmpty())
		return "";
	Vector<TimeLyrics> vtl;
	ProcessMetadata(data, vtl, linesToDisplay);

	auto S = [&](const char* name, const char* primary, const char* secondary, bool bold=true) {
		auto size = bold ? data.fontSize:(int)(data.fontSize*0.7);
		return Format("Style: %s,Arial,%d,%s,%s,&H00000000,&H64000000,%d,0,0,0,100,100,0,0,1,2,1,2,10,10,120,1",
		                name, size, primary, secondary, bold ? -1:0);
	};

	Vector<String> vs{
		"[Script Info]",
		Format("Title: %s by %s", data.title, data.artist),
		"ScriptType: v4.00",
		Format("PlayResY: %d", resY),
		Format("PlayResX: %d", resX),
		"Timer: 100.0000",
		"[V4+ Styles]",
		"Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, "
		   "Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, "
		   "Alignment, MarginL, MarginR, MarginV, Encoding",
		S("V1",       STRAWBERRY_RED_H, STRAWBERRY_RED_D),
		S("V1Normal", STRAWBERRY_RED_D, STRAWBERRY_RED_H, false),
		S("V2",       LIGHT_BLUE_H, LIGHT_BLUE_D),
		S("V2Normal", LIGHT_BLUE_D, LIGHT_BLUE_H, false),
		S("B",        ORCHID_H, ORCHID_D),
		S("BNormal",  ORCHID_D, ORCHID_H, false),
		S("BUC",       MALACHITE_H, MALACHITE_D),
		S("BUCNormal", MALACHITE_D, MALACHITE_H, false),
		S("MC",       MAIZE_H, MAIZE_D),
		S("MCNormal", MAIZE_D, MAIZE_H, false),
		S("CountInV1", RED_H, STRAWBERRY_RED_D),
		S("CountInV2", BLUE_H, LIGHT_BLUE_D),
		S("CountInB",  PURPLE_H, ORCHID_D),
		S("Grayed",   GRAY, GRAY),
		"",
		"[Events]",
		"Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text"
	};

	String lastLine{""};
	for(int i = 1; i < vtl.GetCount(); ++i) {
		const auto& tl = vtl[i];

		auto startTS = tl.time;
		auto endTS = startTS + 5.0f;
		String line = TrimBoth(tl.lyrics);
		if(line == "\u00A0" && lastLine == "\u00A0")
			continue;
		bool hasCountIn = line.StartsWith("@CountIn");
		line.Replace("@CountIn", "");
		if(hasCountIn)
			lastLine.Clear();
		if(i + 1 < vtl.GetCount())
			endTS = vtl[i + 1].time;

		String incomingLyrics;
		VocalPart part = hasCountIn
			? LookaheadVocalPart(vtl, i + 1, data.parts, incomingLyrics)
			: ResolveVocalPart(tl.partIndex, data.parts);

		for(int j = max(1, linesToDisplay - (lastLine.IsEmpty() ? 1:2)); j > 0; --j) {
			if(i + j < vtl.GetCount()) {
				const auto& ntl = vtl[i + j];
				String nextLine = TrimBoth(ntl.lyrics);
				if(nextLine.StartsWith("@CountIn"))
					nextLine = "\u00A0";
				nextLine.Replace("\\(", "(");
				nextLine.Replace("\\)", ")");
				VocalPart nextPart = ResolveVocalPart(ntl.partIndex, data.parts);
				String dimStyle = ResolveDimStyle(nextPart, nextLine, ntl.isMeta);
				vs.AddPick(Format("Dialogue: 0,%s,%s,%s,,0,0,0,,%s%s",
				                    TimeFormatter::Ass(startTS),
				                    TimeFormatter::Ass(endTS),
				                    dimStyle,
				                    hasCountIn ? "":"{\\fad(150,100)}",
				                    nextLine));
			}
		}

		String singLine = line;
		singLine.Replace("\\(", "(");
		singLine.Replace("\\)", ")");
		String hilite = ResolveStyle(part, singLine, hasCountIn, incomingLyrics, tl.isMeta);
		vs.AddPick(Format("Dialogue: 0,%s,%s,%s,,0,0,0,,%s",
		                    TimeFormatter::Ass(startTS),
		                    TimeFormatter::Ass(endTS),
		                    hilite,
		                    singLine));

		if(!lastLine.IsEmpty()) {
			String grayLine = lastLine;
			grayLine.Replace("\\(", "(");
			grayLine.Replace("\\)", ")");
			if(grayLine.StartsWith("~")) {
				grayLine.Remove(0);
				grayLine = Format("{\\i1}%s{\\i0}", grayLine);
			}
			else if(grayLine.StartsWith("(")) {
				grayLine = Format("{\\i1}%s{\\i0}", grayLine);
			}
			else if(grayLine.StartsWith("@")) {
				grayLine = grayLine.Mid(1);
			}
			vs.AddPick(Format("Dialogue: 0,%s,%s,Grayed,,0,0,0,,%s",
			                    TimeFormatter::Ass(startTS),
			                    TimeFormatter::Ass(endTS),
			                    grayLine));
		}
		lastLine = hasCountIn ? "\u00A03... 2... 1...":line;
	}
	return Join(vs, "\n");
}

String TimedToASS(const KarData& data, int linesToDisplay, int resX, int resY) {
	return SubtitleGenerator::ToAss(data, linesToDisplay, resX, resY);
}

String SubtitleGenerator::ToRichAss(const KarData& data, int linesToDisplay, int resX, int resY) {
	return SubtitleGenerator::ToAss(data, linesToDisplay, resX, resY);
}

String TimedToRichASS(const KarData& data, int linesToDisplay, int resX, int resY) {
	return SubtitleGenerator::ToRichAss(data, linesToDisplay, resX, resY);
}
