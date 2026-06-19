#!/usr/bin/env python3
import sys
from pathlib import Path


def fail(message: str) -> None:
    print(f"validate_editor_layouts: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if len(sys.argv) != 2:
        fail("expected repository root argument")

    root = Path(sys.argv[1])
    lay = (root / "Croon.lay").read_text()
    for layout in [
        "LAYOUT(CroonProjectListLayout",
        "LAYOUT(CroonProjectLayout",
        "ITEM(ListCtrl, projectLst",
        "ITEM(ProjectLoader, loader",
        "ITEM(TabCtrl, tab",
        "ITEM(ImageCtrl, videoImg",
    ]:
        if layout not in lay:
            fail(f"missing {layout}")

    expected_bases = {
        "ProjectList.h": "WithCroonProjectListLayout<ParentCtrl>",
        "Project.h": "WithCroonProjectLayout<ParentCtrl>",
    }
    for rel, base in expected_bases.items():
        if base not in (root / rel).read_text():
            fail(f"{rel} does not inherit {base}")

    constructors = {
        "ProjectList.cpp": "ProjectList::ProjectList()",
        "Project.cpp": "Project::Project()",
    }
    for rel, marker in constructors.items():
        text = (root / rel).read_text()
        if "CtrlLayout(*this)" not in text:
            fail(f"{rel} does not call CtrlLayout")
        constructor_body = text.split(marker, 1)[-1].split("\n}\n", 1)[0]
        if "*this <<" in constructor_body:
            fail(f"{rel} still hardcodes top-level child placement")

    project_h = (root / "Project.h").read_text()
    for runtime_member in ["DocEdit lyricsEd;", "RichTextCtrl previewRT;"]:
        if runtime_member not in project_h:
            fail(f"Project.h missing runtime tab member {runtime_member}")

    croon_h = (root / "Croon.h").read_text()
    if croon_h.find('#include "ProjectLoader.h"') > croon_h.find("#include <CtrlCore/lay.h>"):
        fail("Croon.h includes layouts before ProjectLoader declaration")


if __name__ == "__main__":
    main()
