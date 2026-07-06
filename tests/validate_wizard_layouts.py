#!/usr/bin/env python3
import sys
from pathlib import Path


def fail(message: str) -> None:
    print(f"validate_wizard_layouts: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if len(sys.argv) != 2:
        fail("expected repository root argument")

    root = Path(sys.argv[1])
    lay = (root / "Croon.lay").read_text()
    shell_lay = (root / "CroonWizardShell.lay").read_text()
    for layout in [
        "LAYOUT(CroonWizardPage1Layout",
        "LAYOUT(CroonWizardPage2Layout",
        "LAYOUT(CroonWizardPage3Layout",
        "ITEM(TabCtrl, tab",
    ]:
        if layout not in lay:
            fail(f"missing {layout}")

    for layout in [
        "LAYOUT(CroonWizardLayout",
        "ITEM(Page1, page1",
        "ITEM(Page2, page2",
        "ITEM(Page3, page3",
        "ITEM(Label, pageName",
        "ITEM(Button, cancelBtn",
    ]:
        if layout not in shell_lay:
            fail(f"missing {layout}")

    if "LAYOUT(CroonWizardLayout" in lay:
        fail("Croon.lay still defines CroonWizardLayout")

    expected_bases = {
        "Page1.h": "WithCroonWizardPage1Layout<Page>",
        "Page2.h": "WithCroonWizardPage2Layout<Page>",
        "Page3.h": "WithCroonWizardPage3Layout<Page>",
        "WizardDlg.h": "WithCroonWizardLayout<TopWindow>",
    }
    for rel, base in expected_bases.items():
        if base not in (root / rel).read_text():
            fail(f"{rel} does not inherit {base}")

    for rel in ["Page1.cpp", "Page2.cpp", "Page3.cpp", "WizardDlg.cpp"]:
        text = (root / rel).read_text()
        if "CtrlLayout(*this" not in text:
            fail(f"{rel} does not call CtrlLayout")

    page1_impl = (root / "Page1.cpp").read_text()
    if '#include "Croon.h"' in page1_impl:
        fail("Page1.cpp still depends on Croon.h")
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
        '#include "GenreCatalog.h"',
        '#include "Page.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "Page1.h"',
    ]:
        if needle not in page1_impl:
            fail(f"Page1.cpp missing direct dependency {needle}")
    for needle in [
        "GenreCatalog::List()",
        "titleEd.WhenAction",
        "artistEd.WhenAction",
        "KarData::GetGlobal()",
        "loadedAudioLbl.SetLabel(data.origAudioFilePath)",
        "data.title = titleEd.TrimBoth().GetData()",
        "data.year = year.IsNull() ? 0:(int)year",
    ]:
        if needle not in page1_impl:
            fail(f"Page1.cpp missing behavior {needle}")

    page2_impl = (root / "Page2.cpp").read_text()
    if '#include "Croon.h"' in page2_impl:
        fail("Page2.cpp still depends on Croon.h")
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
        '#include "LyricsDownloadService.h"',
        '#include "TextTools.h"',
        '#include "Page.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "Page2.h"',
    ]:
        if needle not in page2_impl:
            fail(f"Page2.cpp missing direct dependency {needle}")
    for needle in [
        "lyricsEd.WhenAction",
        "LyricsDownloadService::Download",
        "TextTools::CleanSpacing",
        "Config::Get(LYRICS_PREFIX)",
        "Config::Get(LYRICS_SUFFIX)",
        "karData.rawLyrics = raw",
    ]:
        if needle not in page2_impl:
            fail(f"Page2.cpp missing behavior {needle}")

    page3_impl = (root / "Page3.cpp").read_text()
    if '#include "Croon.h"' in page3_impl:
        fail("Page3.cpp still depends on Croon.h")
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
        '#include "AppPaths.h"',
        '#include "KarData.h"\n#include "Visualization.h"\n#include "FfmpegCommandBuilder.h"',
        '#include "LyricsTransformer.h"',
        '#include "MediaProcessRunner.h"',
        '#include "RecentProjectService.h"',
        '#include "ProjectLoader.h"',
        '#include "Page.h"',
        "#define LAYOUTFILE <Croon/Croon.lay>",
        "#include <CtrlCore/lay.h>",
        '#include "ProgressDlg.h"',
        '#include "GatherDlg.h"',
        '#include "SaveProjectDlg.h"',
        '#include "VidThumbnail.h"',
        '#include "Page3.h"',
        "GatherDlg& GetGatherDlg();",
    ]:
        if needle not in page3_impl:
            fail(f"Page3.cpp missing direct dependency {needle}")
    if "Page3::Page3(String gatherKey)" not in page3_impl:
        fail("Page3 dynamic constructor was unexpectedly removed")
    constructor_body = page3_impl.split("Page3::Page3(String gatherKey)", 1)[-1].split("\n}\n", 1)[0]
    if "*this <<" in constructor_body:
        fail("Page3 still hardcodes top-level child placement")

    page3_header = (root / "Page3.h").read_text()
    if "TabCtrl tab;" in page3_header:
        fail("Page3.h still declares layout member TabCtrl tab")
    for needle in [
        "AppPaths::FindFiles(videoDir, \"*.mp4\")",
        "AppPaths::DataDirectory()",
        "Visualization::Thumbnail(\"@@freqs\")",
        "GetGatherDlg().WhenVideoAdded",
        "LyricsTransformer::RawToUntimed(data)",
        "AppIdentity::TempFileName(\".json\")",
        "SaveProjectDlg saveDlg",
        "Config::Set(PROJECT_DIR",
        "new VidThumbnail(path, img)",
        "WhenSelected(path, tnPath, img)",
    ]:
        if needle not in page3_impl:
            fail(f"Page3.cpp missing video/save workflow {needle}")

    wizard_header = (root / "WizardDlg.h").read_text()
    for layout_member in ["Page1 page1;", "Page2 page2;", "Page3 page3;"]:
        if layout_member in wizard_header:
            fail(f"WizardDlg.h still declares layout member {layout_member}")

    wizard_impl = (root / "WizardDlg.cpp").read_text()
    constructor_body = wizard_impl.split("WizardDlg::WizardDlg()", 1)[-1].split("\n}\n", 1)[0]
    for line in constructor_body.splitlines():
        if "*this <<" in line and "GetPrevButton" not in line and "GetNextButton" not in line and "GatherButton" not in line:
            fail("WizardDlg still hardcodes non-navigation child placement")

    croon_h = (root / "Croon.h").read_text()
    if croon_h.find('#include "Page3.h"') > croon_h.find("<Croon/CroonWizardShell.lay>"):
        fail("Croon.h includes wizard shell layout before Page3 declaration")


if __name__ == "__main__":
    main()
