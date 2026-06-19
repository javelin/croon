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

    croon_h = (root / "Croon.h").read_text()
    if croon_h.find('#include "LyricsPartsCtrl.h"') > croon_h.find("#include <CtrlCore/lay.h>"):
        fail("Croon.h includes layouts before custom control declarations")


if __name__ == "__main__":
    main()
