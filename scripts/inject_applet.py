#!/usr/bin/env python3
"""Overlay the HybridReader InkHUD applet into a Meshtastic source checkout."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
firmware = Path("firmware")
variant = firmware / "variants/esp32s3/t5s3_epaper/nicheGraphics.h"
if not variant.exists():
    raise SystemExit("Meshtastic checkout not found at ./firmware")

target = firmware / "src/graphics/niche/InkHUD/Applets/User/HybridReader"
target.mkdir(parents=True, exist_ok=True)
for name in ("HybridReaderApplet.h", "HybridReaderApplet.cpp"):
    (target / name).write_bytes((root / "integration" / name).read_bytes())

# EPUB parser and miniz are compiled as part of the Meshtastic source tree.
for name in ("epub.h", "epub.cpp", "miniz.h", "miniz.c"):
    (target / name).write_bytes((root / "integration" / "epub" / name).read_bytes())

text = variant.read_text()
include = '#include "graphics/niche/InkHUD/Applets/User/HybridReader/HybridReaderApplet.h"'
registration = '    inkhud->addApplet("EPUB Reader", new InkHUD::HybridReaderApplet, true, false);'
if include not in text:
    anchor = '#include "graphics/niche/InkHUD/InkHUD.h"'
    if anchor not in text:
        raise SystemExit("Could not find InkHUD include anchor; upstream changed")
    text = text.replace(anchor, anchor + "\n" + include, 1)
if registration not in text:
    anchor = '    // Enable reusable InkHUD touch status indicator'
    if anchor not in text:
        raise SystemExit("Could not find applet registration anchor; upstream changed")
    text = text.replace(anchor, registration + "\n\n" + anchor, 1)
variant.write_text(text)
print("HybridReader applet injected into T5 S3 Pro InkHUD variant")
