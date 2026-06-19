#!/usr/bin/env python3
import sys
from pathlib import Path


def fail(message: str) -> None:
    print(f"validate_process_contracts: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if len(sys.argv) != 2:
        fail("expected repository root argument")

    root = Path(sys.argv[1])
    lay = (root / "Croon.lay").read_text()
    for needle in [
        "LAYOUT(CroonProgressLayout",
        "ITEM(Label, monitor",
        "ITEM(ProgressIndicator, progress",
        "ITEM(Button, cancelBtn",
    ]:
        if needle not in lay:
            fail(f"Croon.lay missing {needle}")

    header = (root / "ProgressDlg.h").read_text()
    if "WithCroonProgressLayout<TopWindow>" not in header:
        fail("ProgressDlg is not backed by CroonProgressLayout")
    for old_member in ["Label monitor;", "ProgressIndicator progress;", "Button cancelBtn;"]:
        if old_member in header:
            fail(f"ProgressDlg still declares layout-owned member {old_member}")

    impl = (root / "ProgressDlg.cpp").read_text()
    if "CtrlLayout(*this, \"Progress\")" not in impl:
        fail("ProgressDlg constructor does not apply Designer layout")
    if "process.Start" not in "".join((root / rel).read_text() for rel in [
        "ConvertDlg.cpp",
        "ExportDlg.cpp",
        "GatherDlg.cpp",
        "OpenProjectDlg.cpp",
        "SaveProjectDlg.cpp",
    ]):
        fail("process dialogs no longer start external processes")


if __name__ == "__main__":
    main()
