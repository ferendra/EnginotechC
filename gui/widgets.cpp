// EnginotechC++ — GUI Widgets implementation

#include "widgets.h"
#include <algorithm>

namespace eng {
namespace gui {

/* ---------------- label ---------------- */

void Label::draw(Surface& s) {
    if (!visible) return;
    int px = x;
    for (char ch : text) {
        s.drawChar(px, y, ch, color, scale);
        px += 6 * scale;
    }
}

/* ---------------- button ---------------- */

void Button::draw(Surface& s) {
    if (!visible) return;
    Color face = pressed_ ? pressColor : hovered_ ? hoverColor : faceColor;
    s.fillRect(x, y, w, h, face);
    s.drawRect(x, y, w, h, borderColor);
    if (!text.empty()) {
        int tw = s.textWidth(text);
        int tx = x + (w - tw) / 2;
        int ty = y + (h - s.textHeight() + 2) / 2;
        s.drawText(tx, ty, text, textColor);
    }
}

bool Button::handle(const Event& e) {
    switch (e.type) {
        case EventType::MouseMove:
            hovered_ = contains(e.mouseX, e.mouseY);
            return false;
        case EventType::MouseDown:
            if (e.button == 1 && contains(e.mouseX, e.mouseY)) {
                pressed_ = true;
                return true;
            }
            break;
        case EventType::MouseUp:
            if (e.button == 1 && pressed_) {
                bool fire = contains(e.mouseX, e.mouseY);
                pressed_ = false;
                if (fire && onClick) onClick();
                return fire;
            }
            break;
        default:
            break;
    }
    return false;
}

/* ---------------- checkbox ---------------- */

void Checkbox::draw(Surface& s) {
    if (!visible) return;
    s.fillRect(x, y, 14, 14, Color::White);
    s.drawRect(x, y, 14, 14, boxBorderColor);
    if (checked) {
        s.drawLine(x + 3, y + 7, x + 6, y + 10, Color::Black);
        s.drawLine(x + 6, y + 10, x + 11, y + 3, Color::Black);
    }
    if (!text.empty()) s.drawText(x + 20, y + 4, text, Color::Black);
}

bool Checkbox::handle(const Event& e) {
    if (e.type == EventType::MouseDown && e.button == 1 &&
        contains(e.mouseX, e.mouseY)) {
        checked = !checked;
        if (onChanged) onChanged(checked);
        return true;
    }
    return false;
}

/* ---------------- textbox ---------------- */

void TextBox::insertChar(char ch) {
    text.insert(text.begin() + static_cast<long>(caret_), ch);
    ++caret_;
}

void TextBox::backspace() {
    if (caret_ == 0) return;
    text.erase(text.begin() + static_cast<long>(caret_) - 1);
    --caret_;
}

void TextBox::draw(Surface& s) {
    if (!visible) return;
    s.fillRect(x, y, w, h, bgColor);
    s.drawRect(x, y, w, h, focused ? borderColorFocused : borderColorUnfocused);

    // Clip the visible portion so long text scrolls left of the caret.
    int maxChars = std::max(0, (w - 6) / 6);
    size_t start = 0;
    if ((int)text.size() > maxChars) {
        start = (int)text.size() > caret_ + (size_t)maxChars
                    ? caret_ - maxChars / 2
                    : text.size() - maxChars;
        if (start > text.size()) start = 0;
    }
    std::string view = text.substr(start, (size_t)maxChars);
    s.drawText(x + 3, y + 6, view, textColor);

    if (focused) {
        int cx = x + 3 + static_cast<int>(caret_ - start) * 6;
        if (cx < x + w - 2) s.drawVLine(cx, y + 3, h - 6, Color::Black);
    }
}

bool TextBox::handle(const Event& e) {
    switch (e.type) {
        case EventType::MouseDown:
            focused = contains(e.mouseX, e.mouseY);
            if (focused) caret_ = text.size();
            return focused;
        case EventType::KeyDown:
            if (!focused) return false;
            switch (e.key) {
                case SpecialKey::Backspace: backspace(); return true;
                case SpecialKey::Left:
                    if (caret_ > 0) --caret_;
                    return true;
                case SpecialKey::Right:
                    if (caret_ < text.size()) ++caret_;
                    return true;
                case SpecialKey::Return:
                    if (onSubmit) onSubmit(text);
                    return true;
                case SpecialKey::Escape:
                    focused = false;
                    return true;
                default:
                    return false;
            }
        case EventType::TextInput:
            if (focused && e.text) {
                insertChar(e.text);
                return true;
            }
            return false;
        default:
            return false;
    }
}

/* ---------------- container ---------------- */

void Container::dispatch(const Event& e) {
    switch (e.type) {
        case EventType::KeyDown:
        case EventType::TextInput:
            if (focused_ && focused_->visible) focused_->handle(e);
            return;
        case EventType::MouseDown: {
            // Everyone sees it (lets textboxes lose focus), then re-resolve focus.
            for (auto& w : widgets_)
                if (w->visible) w->handle(e);
            focused_ = nullptr;
            for (auto& w : widgets_) {
                if (w->visible && w->contains(e.mouseX, e.mouseY)) {
                    w->handle(e);   // second pass lets hit widget react as "hit"
                    focused_ = w.get();
                }
            }
            return;
        }
        default:
            for (auto it = widgets_.rbegin(); it != widgets_.rend(); ++it) {
                if ((*it)->visible && (*it)->handle(e)) break;
            }
            return;
    }
}

void Container::drawAll(Surface& s) {
    for (auto& w : widgets_) w->draw(s);
}

} // namespace gui
} // namespace eng
