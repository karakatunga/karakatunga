#pragma once
#ifdef MESHTASTIC_INCLUDE_INKHUD
#include "graphics/niche/InkHUD/Applet.h"
#include "epub.h"
#include <string>
#include <vector>

namespace NicheGraphics::InkHUD {
class HybridReaderApplet : public Applet {
  public:
    void onRender(bool full) override;
    void onActivate() override;
    void onButtonShortPress() override;
    void onButtonLongPress() override;
    void onNavUp() override;
    void onNavDown() override;
    void onNavLeft() override;
    void onNavRight() override;
    bool onTouchPoint(uint16_t x, uint16_t y, bool longPress) override;
  private:
    void scanLibrary();
    bool openSelectedBook();
    bool loadChapter(int chapter);
    void closeBook();
    void saveProgress();
    void turnPage(int delta);
    void requestRedraw();
    std::vector<std::string> books;
    std::vector<std::string> lines;
    EpubParser parser;
    std::string currentPath;
    std::string currentTitle;
    size_t selected = 0;
    int chapter = 0;
    int page = 0;
    int pages = 1;
    size_t linesPerPage = 15;
    bool scanned = false;
    bool reading = false;
    bool dirty = false;
};
}
#endif