// EnginotechC++ — GUI TextBox Implementation

#include "textbox.h"
#include "theme.h"
#include <algorithm>

namespace eng {
namespace gui {

void TextBox::draw(Surface& surface) {
    const ThemeColors& tc = Theme::active()->c;
    int r = (borderRadius > 0) ? borderRadius : tc.radius;

    Color bg   = (bgColor.r || themeOverride_) ? bgColor  : tc.surface;
    Color border = (borderColor.r || themeOverride_) ? borderColor : tc.border;
    Color fg   = (fgColor.r || themeOverride_) ? fgColor  : tc.fg;
    Color muted = tc.fgMuted;
    Color focusBorder = tc.accent;

    // Shadow
    int shadowAlpha = focused_ ? 50 : 20;
    Color shadowCol{(uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)shadowAlpha};
    surface.fillRect(rect_.x + 2, rect_.y + 3, rect_.w - 2, 4, shadowCol);

    // Background
    surface.fillRoundedRect(rect_.x, rect_.y, rect_.w, rect_.h, bg, r);

    // Border (accent when focused)
    Color effectiveBorder = (focused_ && enabled_) ? focusBorder : border;
    surface.drawRect(rect_.x, rect_.y, rect_.w, rect_.h, effectiveBorder);

    // Subtle inner highlight
    Color highlight = Color{255, 255, 255, 12};
    surface.drawHLine(rect_.x + r, rect_.y, rect_.w - 2 * r, highlight);

    std::string display = text;
    if (passwordMode) {
        display.assign(text.size(), '*');
    }
    if (display.empty() && !focused_) {
        display = placeholder;
    }

    Color textCol = enabled_ ? fg : muted;
    Color placeCol  = muted;

    // Use font config for sizing
    int scale = tc.font.size / 7 + 1;
    int fontH = surface.textHeight(scale);
    int x = rect_.x + 8;
    int y = rect_.y + (rect_.h - fontH) / 2;
    if (y < rect_.y + 2) y = rect_.y + 2;

    if (display.empty() && !text.empty()) {
        surface.drawText(x, y, display, textCol, scale);
    } else if (display.empty()) {
        surface.drawText(x, y, display, placeCol, scale);
    } else {
        surface.drawText(x, y, display, textCol, scale);
    }

    // Cursor
    if (focused_ && cursorVisible && enabled_) {
        int tw = surface.textWidth(display.substr(0, cursorPos), scale);
        int cx = x + tw;
        surface.drawVLine(cx, y, fontH - 1, tc.accent);
    }
}

bool TextBox::onMouseDown(int mx, int my, int button) {
    if (!isPointInside(mx, my)) return false;
    if (button == 1) {
        focused_ = true;
        // Click position -> cursor position (approximate)
        if (enabled_) {
            std::string show = passwordMode ? std::string(text.size(), '*') : text;
            int x = rect_.x + 6;
            int charW = 6;
            int clickRel = mx - x;
            cursorPos = std::max(0, std::min((int)text.size(), clickRel / charW));
        }
        return true;
    }
    return false;
}

bool TextBox::onKeyDown(uint32_t key) {
    if (!focused_ || !enabled_) return false;
    switch (key) {
        case SpecialKey::Left:
            cursorPos = std::max(0, cursorPos - 1);
            return true;
        case SpecialKey::Right:
            cursorPos = std::min((int)text.size(), cursorPos + 1);
            return true;
        case SpecialKey::Home:
            cursorPos = 0;
            return true;
        case SpecialKey::End:
            cursorPos = text.size();
            return true;
        case SpecialKey::Backspace:
            if (cursorPos > 0) {
                text.erase(cursorPos - 1, 1);
                --cursorPos;
                if (onChange) onChange();
                return true;
            }
        case SpecialKey::Delete:
            if (cursorPos < (int)text.size()) {
                text.erase(cursorPos, 1);
                if (onChange) onChange();
                return true;
            }
        case SpecialKey::Return:
            if (onText) onText(text);
            return true;
        default:
            return false;
    }
}

bool TextBox::onTextInput(char c) {
    if (!focused_ || !enabled_) return false;
    if (c >= 32 && c < 127) {
        text.insert(cursorPos, 1, c);
        ++cursorPos;
        if (onChange) onChange();
        return true;
    }
    return false;
}

} // namespace gui
} // namespace eng
