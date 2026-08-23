// EnginotechC++ — GUI Theme Implementation

#include "theme.h"
#include <algorithm>

namespace eng {
namespace gui {

// ── Built-in theme palettes ─────────────────────────────────────────

ThemeColors Theme::makeDark() {
    return {
        .bg       = Color{18, 18, 24},
        .panelBg  = Color{30, 32, 40},
        .surface  = Color{38, 40, 50},
        .fg       = Color{230, 230, 240},
        .fgMuted  = Color{130, 135, 150},
        .accent   = Color{99, 130, 255},
        .accentHover = Color{120, 155, 255},
        .accentActive = Color{75, 105, 220},
        .border   = Color{55, 60, 75},
        .success  = Color{50, 200, 100},
        .error    = Color{240, 70, 70},
        .warning  = Color{240, 180, 50},
        .radius   = 8,
        .font     = { FontFamily::Pixel, 14, false, 0, 0 },
    };
}

ThemeColors Theme::makeLight() {
    return {
        .bg       = Color{240, 241, 244},
        .panelBg  = Color{255, 255, 255},
        .surface  = Color{248, 249, 252},
        .fg       = Color{20, 22, 30},
        .fgMuted  = Color{100, 105, 120},
        .accent   = Color{59, 100, 240},
        .accentHover = Color{45, 80, 210},
        .accentActive = Color{40, 70, 180},
        .border   = Color{210, 212, 220},
        .success  = Color{34, 160, 70},
        .error    = Color{220, 50, 50},
        .warning  = Color{210, 140, 20},
        .radius   = 6,
        .font     = { FontFamily::Pixel, 14, false, 0, 0 },
    };
}

ThemeColors Theme::makeModern() {
    return {
        .bg       = Color{10, 12, 20},
        .panelBg  = Color{22, 25, 38},
        .surface  = Color{30, 34, 52},
        .fg       = Color{240, 242, 255},
        .fgMuted  = Color{140, 148, 175},
        .accent   = Color{0, 210, 220},     // cyan accent
        .accentHover = Color{80, 230, 240},
        .accentActive = Color{0, 170, 180},
        .border   = Color{45, 55, 80},
        .success  = Color{0, 230, 130},
        .error    = Color{255, 75, 100},
        .warning  = Color{255, 195, 50},
        .radius   = 12,
        .font     = { FontFamily::Pixel, 14, false, 0, 0 },
    };
}

// ── Registry ─────────────────────────────────────────────────────────

std::unordered_map<std::string, std::shared_ptr<Theme>>& Theme::registry() {
    static std::unordered_map<std::string, std::shared_ptr<Theme>> reg;
    return reg;
}

void Theme::registerTheme(std::shared_ptr<Theme> t) {
    if (!t) return;
    registry()[t->name] = std::move(t);
}

std::shared_ptr<Theme> Theme::get(const std::string& name) {
    auto& r = registry();
    auto it = r.find(name);
    return it != r.end() ? it->second : nullptr;
}

static std::shared_ptr<Theme> gActive; // null until first setActive

std::shared_ptr<Theme> Theme::active() {
    if (!gActive) gActive = std::make_shared<Theme>("Dark", makeDark());
    return gActive;
}

void Theme::setActive(std::shared_ptr<Theme> t) {
    gActive = std::move(t);
}

} // namespace gui
} // namespace eng
