/*
 * File  : SubtitleWrapProbe.cpp
 * Author: Mark Documento
 */

#include <Draw/Draw.h>

using namespace Upp;

#include "KarData.h"
#include "SubtitleWrapProbe.h"
#include "TimeFormatter.h"
#include "VocalPart.h"

namespace {

String ProbeStyle(const KarData& data, int fontSize, bool bold) {
    return Format("Style: V1,Arial,%d,%s,%s,&H00000000,&H64000000,%d,0,0,0,100,100,0,0,1,2,1,2,10,10,120,1",
                  fontSize > 0 ? fontSize:data.fontSize,
                  VocalPartStyle::V1PrimaryAss(),
                  VocalPartStyle::V1SecondaryAss(),
                  bold ? -1:0);
}

String ProbeMoveTag(int resX, int resY) {
    int bottomPadding = max(30, resY / 12);
    return Format("{\\an2\\pos(%d,%d)}", resX / 2, max(1, resY - bottomPadding));
}

String ProbeText(String text) {
    text = TrimBoth(text);
    text.Replace("\r", "");
    text.Replace("\n", "\\N");
    return text;
}

SubtitleWrapProbeFrame AnalyzeOneFrame(const String& rgba,
                                       int width,
                                       int height,
                                       int frameOffset,
                                       int alphaThreshold) {
    SubtitleWrapProbeFrame frame;
    if (width <= 0 || height <= 0)
        return frame;

    auto isTextPixel = [&](int pixelOffset) {
        byte red = (byte)rgba[pixelOffset];
        byte green = (byte)rgba[pixelOffset + 1];
        byte blue = (byte)rgba[pixelOffset + 2];
        byte alpha = (byte)rgba[pixelOffset + 3];
        return alpha > alphaThreshold &&
               max((int)red, max((int)green, (int)blue)) > alphaThreshold;
    };

    Vector<bool> rowHasText;
    rowHasText.SetCount(height, false);
    for (int y = 0; y < height; y++) {
        int rowOffset = frameOffset + y * width * 4;
        for (int x = 0; x < width; x++) {
            if (isTextPixel(rowOffset + x * 4)) {
                rowHasText[y] = true;
                break;
            }
        }
    }

    bool inBand = false;
    int start = 0;
    for (int y = 0; y < height; y++) {
        if (rowHasText[y] && !inBand) {
            start = y;
            inBand = true;
        }
        else if (!rowHasText[y] && inBand) {
            SubtitleWrapProbeBand& band = frame.bands.Add();
            band.y0 = start;
            band.y1 = y;
            inBand = false;
        }
    }
    if (inBand) {
        SubtitleWrapProbeBand& band = frame.bands.Add();
        band.y0 = start;
        band.y1 = height;
    }

    for (auto& band : frame.bands) {
        int minX = width;
        int maxX = -1;
        for (int y = band.y0; y < band.y1; y++) {
            int rowOffset = frameOffset + y * width * 4;
            for (int x = 0; x < width; x++) {
                if (isTextPixel(rowOffset + x * 4)) {
                    minX = min(minX, x);
                    maxX = max(maxX, x);
                }
            }
        }
        band.width = maxX >= minX ? maxX - minX : 0;
    }

    return frame;
}

int CountTextLineGroups(const SubtitleWrapProbeFrame& frame, int fontSize) {
    if (frame.bands.IsEmpty())
        return 0;

    int maxFragmentGap = max(3, fontSize / 12);
    int groups = 1;
    int groupEnd = frame.bands[0].y1;
    for (int i = 1; i < frame.bands.GetCount(); i++) {
        const auto& band = frame.bands[i];
        if (band.y0 - groupEnd > maxFragmentGap)
            groups++;
        groupEnd = max(groupEnd, band.y1);
    }
    return groups;
}

int TextVerticalSpan(const SubtitleWrapProbeFrame& frame) {
    if (frame.bands.IsEmpty())
        return 0;

    int y0 = frame.bands[0].y0;
    int y1 = frame.bands[0].y1;
    for (const auto& band : frame.bands) {
        y0 = min(y0, band.y0);
        y1 = max(y1, band.y1);
    }
    return y1 - y0;
}

}

String SubtitleWrapProbe::BuildAss(const KarData& data,
                                   const Vector<String>& lyrics,
                                   int resX,
                                   int resY,
                                   int fontSize,
                                   bool bold) {
    if (lyrics.IsEmpty())
        return "";

    Vector<String> lines{
        "[Script Info]",
        Format("Title: %s wrap probe", data.title),
        "ScriptType: v4.00",
        Format("PlayResY: %d", resY),
        Format("PlayResX: %d", resX),
        "Timer: 100.0000",
        "[V4+ Styles]",
        "Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, "
           "Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, "
           "Alignment, MarginL, MarginR, MarginV, Encoding",
        ProbeStyle(data, fontSize, bold),
        "",
        "[Events]",
        "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text"
    };

    for (int i = 0; i < lyrics.GetCount(); i++) {
        lines.Add(Format("Dialogue: 0,%s,%s,V1,,0,0,0,,%s%s",
                         TimeFormatter::Ass((double)i),
                         TimeFormatter::Ass((double)i + 0.90),
                         ProbeMoveTag(resX, resY),
                         ProbeText(lyrics[i])));
    }
    return Join(lines, "\n");
}

Vector<SubtitleWrapProbeFrame> SubtitleWrapProbe::AnalyzeRgbaFrames(const String& rgba,
                                                                    int width,
                                                                    int height,
                                                                    int frameCount,
                                                                    int alphaThreshold) {
    Vector<SubtitleWrapProbeFrame> frames;
    if (width <= 0 || height <= 0 || frameCount <= 0)
        return frames;

    int frameBytes = width * height * 4;
    int availableFrames = rgba.GetCount() / frameBytes;
    int count = min(frameCount, availableFrames);
    for (int i = 0; i < count; i++)
        frames.Add(AnalyzeOneFrame(rgba, width, height, i * frameBytes, alphaThreshold));
    return frames;
}

bool SubtitleWrapProbe::IsWrappedFrame(const SubtitleWrapProbeFrame& frame, int fontSize) {
    return CountTextLineGroups(frame, fontSize) > 1 ||
           TextVerticalSpan(frame) > max(1, fontSize * 5 / 4);
}
