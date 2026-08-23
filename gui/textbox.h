// EnginotechC++ — GUI TextBox Widget
#pragma once
#include "widget.h"
#include <string>

namespace eng {
namespace gui {

class TextBox : public Widget {
public:
    std::string text = "";
    std::string placeholder = "";
    bool passwordMode = false;
    int cursorPos = 0;
    bool cursorVisible = true;

    void draw(Surface& surface) override;
    bool onKeyDown(uint32_t key) override;
    bool onTextInput(char c) override;
    bool onMouseDown(int mx, int my, int button) override;
};

} // namespace gui
} // namespace eng
