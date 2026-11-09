#ifdef MESHTASTIC_INCLUDE_INKHUD
#include "HybridReaderApplet.h"
#include "FSCommon.h"
#include "SPILock.h"
#include <algorithm>
#include <cctype>

using namespace NicheGraphics;
namespace {
bool isEpub(const char *name) {
    if (!name) return false;
    std::string s(name);
    const auto dot = s.find_last_of('.');
    if (dot == std::string::npos) return false;
    s = s.substr(dot);
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s == ".epub";
}
}

void InkHUD::HybridReaderApplet::onActivate() {
    scanLibrary();
    requestRedraw();
}
void InkHUD::HybridReaderApplet::scanLibrary() {
    books.clear();
#ifdef HAS_SDCARD
    // SD and LoRa share the SPI bus on this board. Use Meshtastic's mount
    // and lock; never call SPI.begin() or SD.begin() here.
    {
        concurrency::LockGuard guard(spiLock);
        File dir = FSCom.open("/books", FILE_O_READ);
        if (dir && dir.isDirectory()) {
            File entry = dir.openNextFile();
            while (entry) {
                if (!entry.isDirectory() && isEpub(entry.name())) {
                    const char *path = entry.path();
                    books.emplace_back(path ? path : entry.name());
                }
                entry.close();
                entry = dir.openNextFile();
            }
            dir.close();
        }
    }
#endif
    std::sort(books.begin(), books.end());
    selected = books.empty() ? 0 : std::min(selected, books.size() - 1);
    scanned = true;
}
void InkHUD::HybridReaderApplet::requestRedraw() {
    requestUpdate(NicheGraphics::Drivers::EInk::UpdateTypes::UNSPECIFIED, true);
}
void InkHUD::HybridReaderApplet::onButtonShortPress() {
    // Opening an EPUB is enabled after the parser port is complete.
    scanLibrary();
    requestRedraw();
}
void InkHUD::HybridReaderApplet::onNavUp() {
    if (selected > 0) --selected;
    requestRedraw();
}
void InkHUD::HybridReaderApplet::onNavDown() {
    if (selected + 1 < books.size()) ++selected;
    requestRedraw();
}
bool InkHUD::HybridReaderApplet::onTouchPoint(uint16_t x, uint16_t y, bool longPress) {
    (void)x; (void)longPress;
    if (y < getHeaderHeight()) {
        scanLibrary();
        requestRedraw();
        return true;
    }
    const uint16_t firstRow = getHeaderHeight() + 18;
    if (y >= firstRow && !books.empty()) {
        const size_t row = (y - firstRow) / 42;
        if (row < books.size()) selected = row;
        requestRedraw();
        return true;
    }
    return false;
}
void InkHUD::HybridReaderApplet::onRender(bool full) {
    (void)full;
    if (!scanned) scanLibrary();
    drawHeader("Hybrid EPUB Library");
    setFont(fontMedium);
    if (books.empty()) {
        printAt(12, getHeaderHeight() + 24, "No EPUB files found");
        setFont(fontSmall);
        printAt(12, getHeaderHeight() + 62, "Place books in /books on SD");
        printAt(12, getHeaderHeight() + 96, "Tap header to rescan");
        return;
    }
    const uint16_t top = getHeaderHeight() + 18;
    constexpr size_t visibleRows = 9;
    const size_t first = selected >= visibleRows ? selected - visibleRows + 1 : 0;
    for (size_t i = first; i < books.size() && i < first + visibleRows; ++i) {
        const int y = top + static_cast<int>(i - first) * 42;
        std::string name = books[i];
        const auto slash = name.find_last_of('/');
        if (slash != std::string::npos) name = name.substr(slash + 1);
        if (name.size() > 42) name = name.substr(0, 39) + "...";
        if (i == selected) {
            printAt(8, y, "> " + name);
        } else {
            printAt(8, y, name);
        }
    }
    setFont(fontSmall);
    printAt(12, height() - 28, "Up/Down: select   Tap header: rescan");
}
#endif
