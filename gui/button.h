// EnginotechC++ — GUI Button Widget
#pragma once
#include "widget.h"

namespace eng {
namespace gui {

class Button : public Widget {
public:
    std::string text = "Button";

    void draw(Surface& surface) override;
};

} // namespace gui
} // namespace eng
