#ifdef MESHTASTIC_INCLUDE_INKHUD
#include "HybridReaderApplet.h"
#include "SPILock.h"
#include <SD.h>
#include <Preferences.h>
#include <algorithm>
#include <cctype>
#include <sstream>

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
std::string basenameOf(const std::string &path) {
    const auto slash = path.find_last_of('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
}
}

void InkHUD::HybridReaderApplet::onActivate() {
    scanLibrary();
    Preferences prefs;
    if (prefs.begin("hybreader", true)) {
        String last = prefs.getString("path", "");
        const int savedChapter = prefs.getInt("chapter", 0);
        const int savedPage = prefs.getInt("page", 0);
        prefs.end();
        if (last.length()) {
            auto it = std::find(books.begin(), books.end(), std::string(last.c_str()));
            if (it != books.end()) {
                selected = std::distance(books.begin(), it);
                if (openSelectedBook()) {
                    loadChapter(savedChapter);
                    page = std::max(0, std::min(savedPage, pages - 1));
                }
            }
        }
    }
    requestRedraw();
}
void InkHUD::HybridReaderApplet::scanLibrary() {
    books.clear();
#ifdef HAS_SDCARD
    {
        concurrency::LockGuard guard(spiLock);
        File dir = SD.open("/books", FILE_READ);
        if (dir && dir.isDirectory()) {
            File entry = dir.openNextFile();
            while (entry) {
                if (!entry.isDirectory() && isEpub(entry.name())) {
                    std::string path = entry.name();
                    if (path.empty() || path[0] != '/') path = "/books/" + path;
                    books.emplace_back(path);
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
bool InkHUD::HybridReaderApplet::openSelectedBook() {
    if (selected >= books.size()) return false;
    closeBook();
    currentPath = books[selected];
    if (!parser.open(currentPath.c_str())) {
        currentTitle = "Unable to open EPUB";
        reading = false;
        return false;
    }
    currentTitle = parser.getTitle().length() ? parser.getTitle().c_str() : basenameOf(currentPath);
    reading = true;
    chapter = 0;
    page = 0;
    if (!loadChapter(0)) {
        closeBook();
        return false;
    }
    saveProgress();
    return true;
}
bool InkHUD::HybridReaderApplet::loadChapter(int index) {
    if (!reading || index < 0 || index >= parser.getChapterCount()) return false;
    String content = parser.getChapterText(index);
    if (!content.length()) return false;
    lines.clear();
    std::string word, line;
    const int maxWidth = width() - 36;
    auto pushWord = [&]() {
        if (word.empty()) return;
        std::string candidate = line.empty() ? word : line + " " + word;
        if (!line.empty() && getTextWidth(candidate) > maxWidth) {
            lines.push_back(line);
            line = word;
        } else line = candidate;
        word.clear();
    };
    for (size_t i = 0; i < content.length(); ++i) {
        const char ch = content[i];
        if (ch == '\n' || ch == '\r') {
            pushWord();
            if (!line.empty()) { lines.push_back(line); line.clear(); }
            if (ch == '\n' && i + 1 < content.length() && content[i + 1] == '\n')
                lines.emplace_back("");
        } else if (ch == ' ' || ch == '\t') pushWord();
        else word.push_back(ch);
        if ((i & 1023U) == 0) yield();
    }
    pushWord();
    if (!line.empty()) lines.push_back(line);
    if (lines.empty()) lines.emplace_back("(No readable text in this chapter)");
    const int bodyHeight = height() - getHeaderHeight() - 54;
    const int fontHeight = 25;
    linesPerPage = std::max(1, static_cast<size_t>(bodyHeight / fontHeight));
    pages = std::max(1, static_cast<int>((lines.size() + linesPerPage - 1) / linesPerPage));
    chapter = index;
    page = 0;
    return true;
}
void InkHUD::HybridReaderApplet::closeBook() {
    if (reading) saveProgress();
    parser.close();
    lines.clear();
    currentPath.clear();
    currentTitle.clear();
    reading = false;
    chapter = page = 0;
    pages = 1;
}
void InkHUD::HybridReaderApplet::saveProgress() {
    if (currentPath.empty()) return;
    Preferences prefs;
    if (prefs.begin("hybreader", false)) {
        prefs.putString("path", currentPath.c_str());
        prefs.putInt("chapter", chapter);
        prefs.putInt("page", page);
        prefs.end();
    }
}
void InkHUD::HybridReaderApplet::turnPage(int delta) {
    if (!reading) return;
    const int next = page + delta;
    if (next >= 0 && next < pages) page = next;
    else if (delta > 0 && chapter + 1 < parser.getChapterCount()) loadChapter(chapter + 1);
    else if (delta < 0 && chapter > 0) {
        if (loadChapter(chapter - 1)) page = pages - 1;
    }
    saveProgress();
    requestRedraw();
}
void InkHUD::HybridReaderApplet::requestRedraw() {
    requestUpdate(Drivers::EInk::UpdateTypes::UNSPECIFIED, true);
}
void InkHUD::HybridReaderApplet::onButtonShortPress() {
    if (reading) turnPage(1);
    else if (openSelectedBook()) requestRedraw();
    else requestRedraw();
}
void InkHUD::HybridReaderApplet::onButtonLongPress() {
    if (reading) { closeBook(); requestRedraw(); }
}
void InkHUD::HybridReaderApplet::onNavUp() {
    if (reading) turnPage(-1);
    else if (selected > 0) { --selected; requestRedraw(); }
}
void InkHUD::HybridReaderApplet::onNavDown() {
    if (reading) turnPage(1);
    else if (selected + 1 < books.size()) { ++selected; requestRedraw(); }
}
void InkHUD::HybridReaderApplet::onNavLeft() { if (reading) turnPage(-1); }
void InkHUD::HybridReaderApplet::onNavRight() { if (reading) turnPage(1); }
bool InkHUD::HybridReaderApplet::onTouchPoint(uint16_t x, uint16_t y, bool longPress) {
    if (reading) {
        if (longPress) { closeBook(); requestRedraw(); return true; }
        if (x < width() / 3) turnPage(-1); else turnPage(1);
        return true;
    }
    if (y < getHeaderHeight()) { scanLibrary(); requestRedraw(); return true; }
    const uint16_t top = getHeaderHeight() + 18;
    if (y >= top) {
        const size_t row = (y - top) / 42;
        if (row < books.size()) {
            selected = row;
            if (longPress) openSelectedBook();
            requestRedraw();
        }
        return true;
    }
    return false;
}
void InkHUD::HybridReaderApplet::onRender(bool full) {
    (void)full;
    if (!scanned) scanLibrary();
    if (!reading) {
        drawHeader("EPUB Library");
        setFont(fontMedium);
        if (books.empty()) {
            printAt(14, getHeaderHeight() + 28, "No EPUB files found");
            setFont(fontSmall);
            printAt(14, getHeaderHeight() + 66, "Create /books on the SD card");
            printAt(14, getHeaderHeight() + 96, "Copy EPUB files there and tap the header");
            return;
        }
        constexpr size_t visibleRows = 9;
        const size_t first = selected >= visibleRows ? selected - visibleRows + 1 : 0;
        const uint16_t top = getHeaderHeight() + 18;
        for (size_t i = first; i < books.size() && i < first + visibleRows; ++i) {
            std::string label = basenameOf(books[i]);
            if (label.size() > 54) label = label.substr(0, 51) + "...";
            printAt(12, top + static_cast<int>(i - first) * 42,
                    (i == selected ? "> " : "  ") + label);
        }
        setFont(fontSmall);
        printAt(12, height() - 26, "Select: short press   Open: long press on title");
        return;
    }
    std::string header = currentTitle;
    if (header.size() > 44) header = header.substr(0, 41) + "...";
    drawHeader(header);
    setFont(fontMedium);
    const size_t first = static_cast<size_t>(page) * linesPerPage;
    int y = getHeaderHeight() + 12;
    for (size_t i = first; i < lines.size() && i < first + linesPerPage; ++i) {
        printAt(16, y, lines[i]);
        y += 25;
    }
    setFont(fontSmall);
    printAt(14, height() - 27, "Ch. " + std::to_string(chapter + 1) + "/" +
            std::to_string(parser.getChapterCount()) + "   Page " +
            std::to_string(page + 1) + "/" + std::to_string(pages) +
            "   Tap sides: turn   Hold: library");
}
}
#endif