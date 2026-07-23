#!/usr/bin/env python3
import re
import sys
from pathlib import Path


def fail(message: str) -> None:
    print(f"validate_simple_layouts: {message}", file=sys.stderr)
    raise SystemExit(1)


# Native U++ controls in a .lay ITEM must be written fully qualified (Upp::Button,
# Upp::WithDropChoice<Upp::EditString>, ...). TheIDE adds these prefixes on load,
# so writing them ourselves keeps the committed file stable. Project-defined
# controls (their own class in a repo header) stay unqualified. Any unqualified
# type that is not a project control is therefore a native control missing Upp::.
def check_lay_upp_prefix(root: Path, lay_files) -> None:
    project_types = set()
    for header in root.rglob("*.h"):
        if "build" in header.parts:
            continue
        project_types.update(re.findall(r"\b(?:class|struct)\s+(\w+)", header.read_text()))

    ident = re.compile(r"[A-Za-z_]\w*")
    for lay_path in lay_files:
        rel = lay_path.relative_to(root)
        text = lay_path.read_text()
        for m in re.finditer(r"ITEM\(([^,]+),", text):
            type_str = m.group(1)
            for token in ident.finditer(type_str):
                name = token.group(0)
                if name == "Upp":
                    continue
                qualified = type_str[max(0, token.start() - 2):token.start()] == "::"
                if qualified or name in project_types:
                    continue
                fail(f"{rel}: control type '{name}' must be Upp::-qualified "
                     "(native U++ controls need the prefix so TheIDE does not add it)")


def main() -> None:
    if len(sys.argv) != 2:
        fail("expected repository root argument")

    root = Path(sys.argv[1])

    # Every .lay file must end with exactly one trailing blank line. TheIDE adds
    # one automatically when it loads a layout, so keeping it in the committed
    # file avoids a spurious one-line diff every time the layout is opened. This
    # rule applies to all current and future .lay files.
    lay_files = sorted(p for p in root.rglob("*.lay") if "build" not in p.parts)
    if not lay_files:
        fail("no .lay files found to validate")
    for lay_path in lay_files:
        text = lay_path.read_text()
        rel = lay_path.relative_to(root)
        if not text.endswith("\n\n") or text.endswith("\n\n\n"):
            fail(f"{rel} must end with exactly one trailing blank line "
                 "(TheIDE adds one on load; commit it to avoid noisy diffs)")
        # U++ enum constants (ALIGN_*) must be Upp::-qualified so TheIDE does not
        # rewrite them on load.
        for m in re.finditer(r"(?<!Upp::)\bALIGN_\w+", text):
            fail(f"{rel}: enum '{m.group(0)}' must be Upp::-qualified")

    check_lay_upp_prefix(root, lay_files)

    lay = (root / "Croon.lay").read_text()
    for layout in [
        "LAYOUT(CroonSettingsLayout",
        "LAYOUT(CroonTimingLineLayout",
        "LAYOUT(CroonLyricsPartsLayout",
    ]:
        if layout not in lay:
            fail(f"missing {layout}")

    if 'SetLabel(t_("Export Video"))' not in lay:
        fail("export button is not labeled 'Export Video'")
    if 'SetLabel(t_("Export"))' in lay:
        fail("export button still uses the old 'Export' label")

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
        '#include "KarData.h"\n#include "Visualization.h"',
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
        "okBtn.Ok()",
        "cancelBtn.Cancel()",
        "Break(IDOK)",
        "Break(IDCANCEL)",
    ]:
        if needle not in timing_line_impl:
            fail(f"TimingLineDlg.cpp missing behavior {needle}")

    lyrics_parts_impl = (root / "LyricsPartsDlg.cpp").read_text()
    lyrics_parts_header = (root / "LyricsPartsDlg.h").read_text()
    if "dirty = false;\n        data = &karData;\n        Populate();\n        return TopWindow::Run();" in lyrics_parts_header:
        fail("LyricsPartsDlg.h still owns inline run setup")
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
        '#include "KarData.h"\n#include "Visualization.h"',
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
        "int LyricsPartsDlg::Run(KarData& karData)",
        "dirty = false",
        "data = &karData",
        "Populate()",
        "return TopWindow::Run()",
        "okBtn.Ok()",
        "cancelBtn.Cancel()",
        "PromptYesNo(\"Discard changes and cancel?\")",
        "lpCtrl.ClearParts()",
        "lpCtrl.WhenToggle",
        "data->timedLyrics",
        "lpCtrl.SetLyricsAndParts",
    ]:
        if needle not in lyrics_parts_impl:
            fail(f"LyricsPartsDlg.cpp missing behavior {needle}")

    settings_impl = (root / "SettingsDlg.cpp").read_text()
    if '#include "Croon.h"' in settings_impl:
        fail("SettingsDlg.cpp still depends on Croon.h")
    for needle in [
        "#include <CtrlLib/CtrlLib.h>",
        "#include <algorithm>",
        "#define IMAGECLASS CroonImg",
        "#define IMAGEFILE <Croon/Croon.iml>",
        "#include <Draw/iml_header.h>",
        '#include "Constants.h"',
        '#include "ConfigService.h"\n#include "Config.h"',
        '#include "UiScaler.h"',
        '#include "LyricsPartsCtrl.h"',
        '#include "ListCtrl.h"',
        '#include "AppIdentity.h"',
        '#include "KarData.h"\n#include "Visualization.h"',
        '#include "LyricsTransformer.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ProjectLoader.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "SettingsDlg.h"',
    ]:
        if needle not in settings_impl:
            fail(f"SettingsDlg.cpp missing direct dependency {needle}")
    for needle in [
        'CtrlLayout(*this, "Settings")',
        "Config::Get(FFMPEG_LOCATION)",
        "std::min(Config::MaxFontSize, std::max(Config::MinFontSize, fontSize))",
        "fs.ExecuteOpen(\"Find Ffmpeg Executable...\")",
        "Config::Set(LYRICS_PREFIX",
        "Config::Set(FONT_SIZE",
        "closeBtn.Ok()",
    ]:
        if needle not in settings_impl:
            fail(f"SettingsDlg.cpp missing behavior {needle}")

if __name__ == "__main__":
    main()
