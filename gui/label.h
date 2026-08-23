// EnginotechC++ — GUI Label Widget
#pragma once
#include "widget.h"

namespace eng {
namespace gui {

class Label : public Widget {
public:
    std::string text = "";
    Color labelColor = Color{}; // empty = use theme fg
    bool bold = false;

    void draw(Surface& surface) override;
};

} // namespace gui
} // namespace eng
