#!/usr/bin/env python3
import sys
from pathlib import Path


def fail(message: str) -> None:
    print(f"validate_simple_layouts: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if len(sys.argv) != 2:
        fail("expected repository root argument")

    root = Path(sys.argv[1])
    lay = (root / "Croon.lay").read_text()
    for layout in [
        "LAYOUT(CroonSettingsLayout",
        "LAYOUT(CroonTimingLineLayout",
        "LAYOUT(CroonLyricsPartsLayout",
    ]:
        if layout not in lay:
            fail(f"missing {layout}")

    checks = {
        "SettingsDlg": "WithCroonSettingsLayout<TopWindow>",
        "TimingLineDlg": "WithCroonTimingLineLayout<TopWindow>",
        "LyricsPartsDlg": "WithCroonLyricsPartsLayout<TopWindow>",
    }
    for stem, base in checks.items():
        header = (root / f"{stem}.h").read_text()
        impl = (root / f"{stem}.cpp").read_text()
        if base not in header:
            fail(f"{stem} does not inherit {base}")
        if "CtrlLayout(*this" not in impl:
            fail(f"{stem} constructor does not call CtrlLayout")
        constructor_body = impl.split(f"{stem}::{stem}", 1)[-1]
        if "*this <<" in constructor_body:
            fail(f"{stem} still hardcodes child placement")

    timing_line_impl = (root / "TimingLineDlg.cpp").read_text()
    if '#include "Croon.h"' in timing_line_impl:
        fail("TimingLineDlg.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#define IMAGECLASS CroonImg",
        "#define IMAGEFILE <Croon/Croon.iml>",
        "#include <Draw/iml_header.h>",
        '#include "Constants.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "UiScaler.h"',
        '#include "LyricsPartsCtrl.h"',
        '#include "ListCtrl.h"',
        '#include "AppIdentity.h"',
        '#include "KarData.h"\n#include "Visualization.h"\n#include "FfmpegCommandBuilder.h"',
        '#include "LyricsTransformer.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ProjectLoader.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "TimingLineDlg.h"',
    ]:
        if needle not in timing_line_impl:
            fail(f"TimingLineDlg.cpp missing direct dependency {needle}")
    for needle in [
        'CtrlLayout(*this, "Edit Lyrics")',
        "decorEd.SetData(decor)",
        "lyricsEd.SetData(lyrics)",
        "Break(IDOK)",
        "Break(IDCANCEL)",
    ]:
        if needle not in timing_line_impl:
            fail(f"TimingLineDlg.cpp missing behavior {needle}")

    lyrics_parts_impl = (root / "LyricsPartsDlg.cpp").read_text()
    if '#include "Croon.h"' in lyrics_parts_impl:
        fail("LyricsPartsDlg.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#define IMAGECLASS CroonImg",
        "#define IMAGEFILE <Croon/Croon.iml>",
        "#include <Draw/iml_header.h>",
        '#include "Constants.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "UiScaler.h"',
        '#include "LyricsPartsCtrl.h"',
        '#include "ListCtrl.h"',
        '#include "AppIdentity.h"',
        '#include "KarData.h"\n#include "Visualization.h"\n#include "FfmpegCommandBuilder.h"',
        '#include "LyricsTransformer.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ProjectLoader.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "LyricsPartsDlg.h"',
    ]:
        if needle not in lyrics_parts_impl:
            fail(f"LyricsPartsDlg.cpp missing direct dependency {needle}")
    for needle in [
        "CtrlLayout(*this)",
        "PromptYesNo(\"Discard changes and cancel?\")",
        "lpCtrl.ClearParts()",
        "lpCtrl.WhenToggle",
        "data->timedLyrics",
        "lpCtrl.SetLyricsAndParts",
    ]:
        if needle not in lyrics_parts_impl:
            fail(f"LyricsPartsDlg.cpp missing behavior {needle}")

    croon_h = (root / "Croon.h").read_text()
    if croon_h.find('#include "LyricsPartsCtrl.h"') > croon_h.find("#include <CtrlCore/lay.h>"):
        fail("Croon.h includes layouts before custom control declarations")


if __name__ == "__main__":
    main()
