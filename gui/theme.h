// EnginotechC++ — GUI Theme System
// Provides pluggable color themes for the entire UI.

#pragma once
#include "renderer.h"
#include <string>
#include <memory>
#include <unordered_map>

namespace eng {
namespace gui {

// ── Font configuration (adjustable from script) ────────────────────
enum class FontFamily { Pixel, Mono, Sans };

struct FontConfig {
    FontFamily family = FontFamily::Pixel;
    int size       = 14;   // base pixel size per character (scales the embedded bitmap font)
    bool bold      = false; // renders a slightly thicker variant of the pixel font
    int lineHeight = 0;    // 0 = auto (size + 2)
    int letterSpacing = 0; // extra px between chars
};

struct ThemeColors {
    Color bg          = Color{18, 18, 24};    // window/background
    Color panelBg     = Color{30, 32, 40};    // widget base
    Color surface     = Color{38, 40, 50};    // card/surface
    Color fg          = Color{230, 230, 240}; // primary text
    Color fgMuted     = Color{130, 135, 150}; // secondary text
    Color accent      = Color{99, 130, 255};  // primary button
    Color accentHover = Color{120, 155, 255}; // hovered accent
    Color accentActive= Color{75, 105, 220};  // pressed accent
    Color border      = Color{55, 60, 75};    // borders
    Color success     = Color{50, 200, 100};  // positive
    Color error       = Color{240, 70, 70};   // negative
    Color warning     = Color{240, 180, 50};  // caution
    int  radius       = 8;                    // corner radius px
    FontConfig font;                        // default font for all widgets
};

class Theme {
public:
    std::string name;
    ThemeColors c;

    explicit Theme(std::string n, ThemeColors colors = {})
        : name(std::move(n)), c(std::move(colors)) {}

    static Theme& dark()   { static Theme t("Dark",   makeDark());   return t; }
    static Theme& light()  { static Theme t("Light",  makeLight());  return t; }
    static Theme& modern() { static Theme t("Modern", makeModern()); return t; }

    static void registerTheme(std::shared_ptr<Theme> t);
    static std::shared_ptr<Theme> get(const std::string& name);
    static std::shared_ptr<Theme> active();
    static void setActive(std::shared_ptr<Theme> t);

public:
    // Factory methods made public for external use
    static ThemeColors makeDark();
    static ThemeColors makeLight();
    static ThemeColors makeModern();

private:
    static std::unordered_map<std::string, std::shared_ptr<Theme>>& registry();
};

} // namespace gui
} // namespace eng
