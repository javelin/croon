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
    for layout in [
        "LAYOUT(CroonWizardPage1Layout",
        "LAYOUT(CroonWizardPage2Layout",
        "LAYOUT(CroonWizardLayout",
    ]:
        if layout not in lay:
            fail(f"missing {layout}")

    expected_bases = {
        "Page1.h": "WithCroonWizardPage1Layout<Page>",
        "Page2.h": "WithCroonWizardPage2Layout<Page>",
        "WizardDlg.h": "WithCroonWizardLayout<TopWindow>",
    }
    for rel, base in expected_bases.items():
        if base not in (root / rel).read_text():
            fail(f"{rel} does not inherit {base}")

    for rel in ["Page1.cpp", "Page2.cpp", "WizardDlg.cpp"]:
        text = (root / rel).read_text()
        if "CtrlLayout(*this" not in text:
            fail(f"{rel} does not call CtrlLayout")

    if "Page3::Page3(String gatherKey)" not in (root / "Page3.cpp").read_text():
        fail("Page3 dynamic constructor was unexpectedly removed")


if __name__ == "__main__":
    main()
