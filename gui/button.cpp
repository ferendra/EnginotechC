// EnginotechC++ — GUI Button Implementation

#include "button.h"
#include "theme.h"

namespace eng {
namespace gui {

void Button::draw(Surface& surface) {
    const ThemeColors& tc = Theme::active()->c;
    int r = (borderRadius > 0) ? borderRadius : tc.radius;

    // Resolve colors
    Color bg   = (bgColor.r || themeOverride_) ? bgColor  : tc.panelBg;
    Color fg   = (fgColor.r || themeOverride_) ? fgColor  : tc.fg;
    Color border = (borderColor.r || themeOverride_) ? borderColor : tc.border;
    Color hover  = (hoverColor.r || themeOverride_) ? hoverColor : tc.accentHover;
    Color active = (activeColor.r || themeOverride_) ? activeColor : tc.accentActive;

    // Hover/active state
    if (hovered_ && enabled_)       bg = hover;
    if (focused_ && enabled_)       bg = active;
    if (!enabled_)                  fg = tc.fgMuted;

    // Accent button variant
    bool isAccent = themeDriven && !bgColor.r && !themeOverride_;
    if (isAccent) {
        bg = tc.accent;
        fg = tc.fg;
        if (hovered_ && enabled_) bg = tc.accentHover;
        if (focused_ && enabled_) bg = tc.accentActive;
    }

    // Shadow (subtle offset below)
    int shadowY = rect_.y + 3;
    int shadowAlpha = enabled_ ? 30 : 10;
    Color shadowCol{(uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)shadowAlpha};
    surface.fillRect(rect_.x + 2, shadowY, rect_.w - 2, 4, shadowCol);

    // Fill
    surface.fillRoundedRect(rect_.x, rect_.y, rect_.w, rect_.h, bg, r);

    // Border
    if (!isAccent || borderColor.r) {
        surface.drawRect(rect_.x, rect_.y, rect_.w, rect_.h, border);
    }

    // Highlight (top edge)
    Color highlight = Color{255, 255, 255, 20};
    surface.drawHLine(rect_.x + r, rect_.y, rect_.w - 2 * r, highlight);

    // Text with font config
    int scale = tc.font.size / 7 + 1;
    int fontH = surface.textHeight(scale);
    int ty = rect_.y + (rect_.h - fontH) / 2;
    if (tc.font.bold || fontBold) {
        surface.drawTextConfiguredBold(rect_.x + rect_.w / 2, ty, text, fg, tc.font);
    } else {
        surface.drawTextConfigured(rect_.x + rect_.w / 2, ty, text, fg, tc.font);
    }
}

} // namespace gui
} // namespace eng
