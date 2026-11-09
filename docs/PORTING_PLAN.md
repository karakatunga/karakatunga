# EPUB engine integration and validation

## Implemented
- The standalone reader's EPUB ZIP/OPF parser and miniz inflate implementation are included under `integration/epub` with the upstream MIT license.
- The InkHUD applet lists EPUB files in `/books`, opens a book, extracts chapter text, wraps it to the InkHUD font width, and paginates it.
- Page turns cross chapter boundaries.
- The current book, chapter and page are persisted in a dedicated Preferences namespace.
- ZIP file reads are protected by Meshtastic's shared `spiLock`; compressed data is read while holding the lock and inflated after releasing it.

## Still required before a final release
1. Obtain a successful PlatformIO build from GitHub Actions and resolve all compiler/linker errors.
2. Validate on the exact H752-01 V2 hardware, including display orientation, GT911 touch and physical button semantics.
3. Measure LoRa packet handling while opening large EPUBs. The current parser/chapter load is synchronous at the UI callback level, so the radio must be stress-tested with large books; if latency is unacceptable, move parsing to a bounded background worker with short SPI critical sections.
4. Add TOC navigation, persistent multiple bookmarks, font/spacing settings and robust handling for malformed/large EPUBs.
5. Test SD removal, missing `/books`, non-ASCII filenames, stored/deflated ZIP entries, and power interruption during progress save.
6. Pin Meshtastic upstream to the exact commit used for the validated build, publish the firmware binary with checksums and release notes.

## Hardware constraints
- Do not add a second `setup()` or `loop()`.
- Do not call `SD.begin()` or `SPI.begin()` from the applet.
- The T5 variant shares SPI between SD and LoRa. All SD access must use the existing mount and `spiLock`.
- Keep display and touch ownership in Meshtastic's existing InkHUD stack.
