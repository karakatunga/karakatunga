# T5 Hybrid — Meshtastic + EPUB Reader

Experimental integration for the LilyGO T5 E-Paper S3 Pro V2 (H752-01).

## Implemented in this branch

- Meshtastic remains the firmware and radio core.
- InkHUD applet browses EPUB files in `/books` on the microSD card.
- EPUB ZIP/OPF parsing and chapter text extraction are ported from the MIT-licensed T5 reader project.
- Basic word wrapping, page navigation and chapter transitions.
- Last opened book, chapter and page are saved in a separate Preferences namespace and restored on applet activation.
- SD file operations use Meshtastic's shared `spiLock`; EPUB inflate work happens after releasing the SPI lock.
- CI workflow overlays the applet into upstream Meshtastic and compiles `t5s3_epaper_inkhud`.

## Controls

- Library: short button press opens the highlighted book.
- Library: tap header to rescan; long-press a title to open it.
- Reader: short button press / right-side tap / down navigation advances a page.
- Reader: left-side tap / up navigation goes back a page.
- Reader: long press returns to the library.
- Reading position is saved automatically.

Copy EPUB files to `/books` on the microSD card.

## Build

GitHub Actions workflow: `.github/workflows/build.yml`.

The workflow checks out upstream Meshtastic `develop`, overlays the applet and EPUB parser, and runs:

```sh
pio run -e t5s3_epaper_inkhud
```

The firmware binary is uploaded as a short-lived workflow artifact after a successful compile.

## Important limitations before calling this final

This is a source integration milestone, not yet a hardware-validated release. The EPUB parser and initial chapter extraction are synchronous from the InkHUD input callback; SD bus ownership is protected, but radio servicing under worst-case large books still needs measurement on the device. Rich typography, inline images, table of contents, multiple bookmarks, settings, and EPUB edge-case validation are not yet implemented. Do not flash an artifact until the workflow succeeds and the image is validated on the exact T5 hardware revision.

## Hardware constraints

- Target: Meshtastic `t5s3_epaper_inkhud` (H752-01 V2)
- Display: ED047TC1, driven by the existing InkHUD display stack
- Touch: existing GT911 integration
- Storage: existing Meshtastic SD mount; no second `SD.begin()` or SPI bus
- Radio: SX1262 remains owned by Meshtastic

## Upstream projects

- Meshtastic firmware: https://github.com/meshtastic/firmware
- EPUB reader reference: https://github.com/yetisoldier/t5-ereader-firmware
- Ported parser license: [integration/epub/LICENSE](integration/epub/LICENSE)
