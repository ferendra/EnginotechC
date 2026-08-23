// EnginotechC++ — GUI Label Implementation

#include "label.h"

namespace eng {
namespace gui {

void Label::draw(Surface& surface) {
    const ThemeColors& tc = Theme::active()->c;
    Color c = enabled_
        ? (labelColor.r ? labelColor : tc.fg)
        : tc.fgMuted;

    // Calculate font scale from theme font config
    int scale = tc.font.size / 7 + 1;
    int fontH = surface.textHeight(scale);

    // Vertical centering
    int yOff = (rect_.h - fontH) / 2;
    int y = rect_.y + yOff;
    if (y < rect_.y) y = rect_.y;

    // Draw text with configured font
    if (bold) {
        surface.drawTextConfiguredBold(rect_.x + 6, y, text, c, tc.font);
    } else {
        surface.drawTextConfigured(rect_.x + 6, y, text, c, tc.font);
    }
}

} // namespace gui
} // namespace eng
