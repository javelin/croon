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
    main_lay = (root / "CroonMainWindow.lay").read_text()
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

    for layout in [
        "LAYOUT(CroonMainWindowLayout",
        "ITEM(ProjectList, projects",
        "ITEM(Project, project",
    ]:
        if layout not in main_lay:
            fail(f"missing {layout}")

    expected_bases = {
        "ProjectList.h": "WithCroonProjectListLayout<ParentCtrl>",
        "Project.h": "WithCroonProjectLayout<ParentCtrl>",
        "MainWindow.h": "WithCroonMainWindowLayout<TopWindow>",
    }
    for rel, base in expected_bases.items():
        if base not in (root / rel).read_text():
            fail(f"{rel} does not inherit {base}")

    constructors = {
        "ProjectList.cpp": "ProjectList::ProjectList()",
        "Project.cpp": "Project::Project()",
        "MainWindow.cpp": "MainWindow::MainWindow()",
    }
    for rel, marker in constructors.items():
        text = (root / rel).read_text()
        if "CtrlLayout(*this)" not in text:
            fail(f"{rel} does not call CtrlLayout")
        constructor_body = text.split(marker, 1)[-1].split("\n}\n", 1)[0]
        if "*this <<" in constructor_body:
            fail(f"{rel} still hardcodes top-level child placement")

    mainwindow_h = (root / "MainWindow.h").read_text()
    for runtime_member in ["StatusBar status;", "MenuBar menuBar;"]:
        if runtime_member not in mainwindow_h:
            fail(f"MainWindow.h missing runtime frame member {runtime_member}")
    for layout_member in ["Project project;", "ProjectList projects;"]:
        if layout_member in mainwindow_h:
            fail(f"MainWindow.h still declares layout member {layout_member}")

    project_h = (root / "Project.h").read_text()
    for runtime_member in ["DocEdit lyricsEd;", "RichTextCtrl previewRT;"]:
        if runtime_member not in project_h:
            fail(f"Project.h missing runtime tab member {runtime_member}")

    croon_h = (root / "Croon.h").read_text()
    if croon_h.find('#include "ProjectLoader.h"') > croon_h.find("#include <CtrlCore/lay.h>"):
        fail("Croon.h includes layouts before ProjectLoader declaration")
    if croon_h.find('#include "TimingCtrl.h"') > croon_h.find("<Croon/CroonTimingDlg.lay>"):
        fail("Croon.h includes TimingDlg layout before TimingCtrl declaration")

    timing_lay = (root / "CroonTimingDlg.lay").read_text()
    for layout in [
        "LAYOUT(CroonTimingDlgLayout",
        "ITEM(TimingCtrl, timingCtrl",
        "ITEM(SliderCtrl, sliderCtrl",
    ]:
        if layout not in timing_lay:
            fail(f"missing {layout}")

    timing_header = (root / "TimingDlg.h").read_text()
    if "WithCroonTimingDlgLayout<TopWindow>" not in timing_header:
        fail("TimingDlg does not inherit WithCroonTimingDlgLayout")

    timing_impl = (root / "TimingDlg.cpp").read_text()
    if "CtrlLayout(*this" not in timing_impl:
        fail("TimingDlg constructor does not call CtrlLayout")
    constructor_body = timing_impl.split("TimingDlg::TimingDlg()", 1)[-1].split("\n}\n", 1)[0]
    if "*this <<" in constructor_body:
        fail("TimingDlg still hardcodes top-level child placement")
    for layout_member in ["Button playBtn;", "TimingCtrl timingCtrl;", "SliderCtrl sliderCtrl;"]:
        if layout_member in timing_header:
            fail(f"TimingDlg.h still declares layout member {layout_member}")


if __name__ == "__main__":
    main()
