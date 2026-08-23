// EnginotechC++ — GUI Slider Widget
#pragma once
#include "widget.h"

namespace eng {
namespace gui {

class Slider : public Widget {
public:
    double value = 0.5;
    double min   = 0.0;
    double max   = 1.0;
    Color  trackColor  = Color{}; // defaults to theme.surface
    Color  fillColor   = Color{}; // defaults to theme.accent
    Color  handleColor = Color{}; // defaults to theme.fg

    void draw(Surface& surface) override;
    bool onMouseDown(int mx, int my, int button) override;
    bool onMouseMove(int mx, int my) override;
    bool onMouseUp  (int mx, int my, int button) override;

private:
    bool dragging_ = false;
    void updateValue(int mx);
};

} // namespace gui
} // namespace eng
