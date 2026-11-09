# T5 Hybrid — Meshtastic + EPUB Reader

Experimental integration workspace for the LilyGO T5 E-Paper S3 Pro V2 (H752-01).

**Goal:** keep Meshtastic as the firmware and radio core, and add an EPUB reader as an InkHUD applet. LoRa, Bluetooth, GPS, routing and message handling must remain active while reading.

## Current implementation status

This repository contains an InkHUD library applet prototype and an automated firmware build workflow. The applet scans the Meshtastic-mounted SD card for EPUB files and presents a selectable library. It is **not yet a complete EPUB reader**: EPUB parsing, pagination, bookmarks and reading-position persistence remain to be ported.

No prebuilt firmware is published. Do not flash an artifact until a release is explicitly marked hardware-validated.

## Build

GitHub Actions checks out upstream Meshtastic `develop`, overlays the applet into the T5 S3 Pro InkHUD variant and runs:

```sh
pio run -e t5s3_epaper_inkhud
```

For a reproducible release, upstream must be pinned to an exact commit.

## Hardware / upstream

- Board target: Meshtastic `t5s3_epaper_inkhud` (H752-01 V2)
- Display/UI: ED047TC1 + InkHUD
- Touch: existing T5 touch integration
- Storage: Meshtastic `FSCom` / `SPILock`; no second `SD.begin()`
- EPUB engine reference: https://github.com/yetisoldier/t5-ereader-firmware

## Roadmap

- [x] Create integration repository and branch
- [x] Add InkHUD library applet prototype
- [x] Add automated compile workflow
- [ ] Compile against current upstream
- [ ] Port EPUB parser without taking over Meshtastic setup/loop
- [ ] Add chapter rendering, page turns, TOC and bookmarks
- [ ] Persist reading progress and settings
- [ ] Validate concurrent LoRa traffic during SD reads/page turns on hardware
- [ ] Publish firmware only after hardware validation

## Critical constraints

The T5 variant explicitly states that SD and LoRa share pins and must use the default SPI object. Reader code accesses the already-mounted `FSCom` filesystem under `spiLock`; it must never initialise another SPI bus or call `SD.begin()`. Long EPUB operations must be incremental so Meshtastic continues servicing the radio.
