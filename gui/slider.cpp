// EnginotechC++ — GUI Slider Implementation

#include "slider.h"
#include "theme.h"
#include <cmath>

namespace eng {
namespace gui {

void Slider::draw(Surface& surface) {
    const ThemeColors& tc = Theme::active()->c;
    int trackY = rect_.y + rect_.h / 2 - 4;
    int trackH = 8;
    int r = 4; // track corner radius

    Color trackColor = (this->trackColor.r) ? this->trackColor : tc.surface;
    Color fillColor  = (this->fillColor.r)  ? this->fillColor  : tc.accent;
    Color handleColor = (this->handleColor.r) ? this->handleColor : tc.fg;

    // Track background (rounded)
    int tx = rect_.x + 6;
    int tw = rect_.w - 12;
    surface.fillRoundedRect(tx, trackY, tw, trackH, trackColor, r);

    // Subtle border on track
    Color trackBorder = Color{tc.border.r, tc.border.g, tc.border.b, 80};
    surface.drawRect(tx, trackY, tw, trackH, trackBorder);

    // Fill up to current value
    double ratio = (value - min) / (max - min);
    if (ratio > 1.0) ratio = 1.0;
    if (ratio < 0.0) ratio = 0.0;
    int fillW = (int)(tw * ratio);
    if (fillW > 0) {
        // Gradient from left (accent) to slightly lighter
        surface.fillRoundedRect(tx, trackY, fillW, trackH, fillColor, r);
    }

    // Handle
    int handleX = tx + fillW;
    int handleR = 8;
    // Shadow under handle
    surface.fillCircle(handleX + 1, trackY + trackH / 2 + 2, handleR, Color{0, 0, 0, 40});
    // Handle body
    surface.fillCircle(handleX, trackY + trackH / 2, handleR, handleColor);
    // Highlight on handle
    surface.fillCircle(handleX - 2, trackY + trackH / 2 - 2, handleR / 2, Color{255, 255, 255, 40});
}

bool Slider::onMouseDown(int mx, int my, int button) {
    if (!isPointInside(mx, my)) return false;
    if (button == 1) {
        dragging_ = true;
        updateValue(mx);
        if (onChange) onChange();
        return true;
    }
    return false;
}

bool Slider::onMouseMove(int mx, int my) {
    if (dragging_) {
        updateValue(mx);
        if (onChange) onChange();
    }
    return false;
}

bool Slider::onMouseUp(int mx, int my, int button) {
    dragging_ = false;
    return false;
}

void Slider::updateValue(int mx) {
    double ratio = (double)(mx - rect_.x - 4) / (rect_.w - 8);
    if (ratio > 1.0) ratio = 1.0;
    if (ratio < 0.0) ratio = 0.0;
    value = min + ratio * (max - min);
}

} // namespace gui
} // namespace eng
