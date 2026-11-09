#pragma once
#ifdef MESHTASTIC_INCLUDE_INKHUD
#include "graphics/niche/InkHUD/Applet.h"
#include <string>
#include <vector>

namespace NicheGraphics::InkHUD {
class HybridReaderApplet : public Applet {
  public:
    void onRender(bool full) override;
    void onActivate() override;
    void onButtonShortPress() override;
    void onNavUp() override;
    void onNavDown() override;
    bool onTouchPoint(uint16_t x, uint16_t y, bool longPress) override;
  private:
    void scanLibrary();
    void requestRedraw();
    std::vector<std::string> books;
    size_t selected = 0;
    bool scanned = false;
};
}
#endif
