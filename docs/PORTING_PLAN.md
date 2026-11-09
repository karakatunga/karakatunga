# EPUB engine porting plan

The standalone T5 reader is not safe to compile as a second Arduino application inside Meshtastic: its `main.cpp` owns setup/loop, display, touch, deep sleep and SD initialisation. The hybrid must instead reuse its parser/model selectively.

1. Isolate `EpubParser` and ZIP/XML dependencies from the standalone app.
2. Replace direct `SD.begin()`, `SPI.begin()` and SD operations with a storage adapter using Meshtastic's mounted `FSCom` and `concurrency::LockGuard(spiLock)`.
3. Make parsing incremental/stateful; do not monopolise the main loop during large archive reads.
4. Replace standalone display functions with InkHUD drawing calls.
5. Add library, opening, reading, table-of-contents and bookmark states.
6. Save progress/bookmarks in a namespaced directory such as `/books/.hybrid/`; do not reuse Meshtastic preference keys.
7. Test SD/LoRa bus contention and verify radio packets continue processing during navigation.

## Current prototype limitations

The applet only lists EPUB filenames in `/books`. Navigation selects a row; the primary button and header touch rescan the card. EPUB opening and page rendering are not implemented yet.
