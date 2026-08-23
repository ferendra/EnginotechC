// EnginotechC++ — GUI Widget Implementation

#include "widget.h"
#include <algorithm>
#include <cmath>

namespace eng {
namespace gui {

// Resolve a color member against theme or fall back to explicit value
Color Widget::resolveTheme(Color Widget::*member) const {
    if (themeOverride_) return themeOverride_->panelBg;
    if (!themeDriven) return this->*member;
    const ThemeColors& tc = Theme::active()->c;
    // Use explicit if non-zero (non-default), else theme
    Color base = this->*member;
    bool isDefault = (base.r == 0 && base.g == 0 && base.b == 0);
    return isDefault ? tc.panelBg : base;
}

// ── Widget base ───────────────────────────────────────────────────
void Widget::drawRoundedRect(Surface& s, Color fill, Color border, int thickness) {
    int r = (borderRadius > 0) ? borderRadius : Theme::active()->c.radius;
    int x = rect_.x, y = rect_.y, w = rect_.w, h = rect_.h;
    r = std::min(r, std::min(w / 2, h / 2));

    // Fill interior
    s.fillRect(x + thickness, y + thickness, w - 2 * thickness, h - 2 * thickness, fill);

    // Border
    if (thickness > 0) {
        // Top
        for (int i = 0; i < r; ++i) {
            s.fillCircle(x + r - i, y + r, i + thickness, border);
        }
        // Bottom
        for (int i = 0; i < r; ++i) {
            s.fillCircle(x + r - i, y + h - r, i + thickness, border);
        }
        // Top-left arc
        for (int dy = -r; dy <= 0; ++dy) {
            int dx = -(int)std::sqrt((double)r * r - (double)dy * dy);
            for (int t = 0; t < thickness; ++t)
                s.setPixel(x + r + dx + t, y + r + dy, border);
        }
        // Top-right arc
        for (int dy = -r; dy <= 0; ++dy) {
            int dx = (int)std::sqrt((double)r * r - (double)dy * dy);
            for (int t = 0; t < thickness; ++t)
                s.setPixel(x + r + dx + t, y + r + dy, border);
        }
        // Bottom-left arc
        for (int dy = 0; dy <= r; ++dy) {
            int dx = -(int)std::sqrt((double)r * r - (double)dy * dy);
            for (int t = 0; t < thickness; ++t)
                s.setPixel(x + r + dx + t, y + h - r + dy, border);
        }
        // Bottom-right arc
        for (int dy = 0; dy <= r; ++dy) {
            int dx = (int)std::sqrt((double)r * r - (double)dy * dy);
            for (int t = 0; t < thickness; ++t)
                s.setPixel(x + r + dx + t, y + h - r + dy, border);
        }
        // Side lines
        s.drawVLine(x, y + r, h - 2 * r, border);
        s.drawVLine(x + w - 1, y + r, h - 2 * r, border);
        // Top/bottom lines
        s.drawHLine(x + r, y, w - 2 * r, border);
        s.drawHLine(x + r, y + h - 1, w - 2 * r, border);
    }
}

void Widget::drawTextCentered(Surface& s, const std::string& text, Color color, int yOff) {
    const ThemeColors& tc = Theme::active()->c;
    int scale = tc.font.size / 7 + 1;
    int fontH = s.textHeight(scale);
    int tw = s.textWidth(text, scale);
    int tx = rect_.x + (rect_.w - tw) / 2;
    int ty = rect_.y + (rect_.h - fontH) / 2 + yOff;
    if (tc.font.bold || fontBold) {
        s.drawTextConfiguredBold(tx, ty, text, color, tc.font);
    } else {
        s.drawTextConfigured(tx, ty, text, color, tc.font);
    }
}

bool Widget::onMouseDown(int mx, int my, int button) {
    if (!visible_ || !enabled_) return false;
    if (button == 1) {
        hovered_ = true;
        if (onClick) onClick();
        return true;
    }
    return false;
}

bool Widget::onMouseUp(int mx, int my, int button) {
    hovered_ = false;
    return false;
}

bool Widget::onMouseMove(int mx, int my) {
    hovered_ = isPointInside(mx, my);
    return false;
}

bool Widget::onKeyDown(uint32_t /*key*/)  { return false; }
bool Widget::onTextInput(char /*c*/)      { return false; }
void  Widget::onFocusGain()               {}
void  Widget::onFocusLoss()               {}

// ── Container ─────────────────────────────────────────────────────
bool Container::onMouseDown(int mx, int my, int button) {
    // Process from front to back (topmost first)
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if ((*it)->onMouseDown(mx - rect_.x, my - rect_.y, button)) {
            return true;
        }
    }
    return Widget::onMouseDown(mx, my, button);
}

bool Container::onMouseUp(int mx, int my, int button) {
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if ((*it)->onMouseUp(mx - rect_.x, my - rect_.y, button)) return true;
    }
    return Widget::onMouseUp(mx, my, button);
}

bool Container::onMouseMove(int mx, int my) {
    bool inside = rect_.contains(mx, my);
    for (auto& c : children_) {
        c->onMouseMove(mx - rect_.x, my - rect_.y);
    }
    return Widget::onMouseMove(mx, my);
}

bool Container::onKeyDown(uint32_t key) {
    if (focusedChild_) {
        if (focusedChild_->onKeyDown(key)) return true;
    }
    // Tab cycling
    if (key == SpecialKey::Tab) {
        if (!children_.empty()) {
            int idx = 0;
            if (focusedChild_) {
                for (int i = 0; i < (int)children_.size(); ++i)
                    if (children_[i].get() == focusedChild_) { idx = i; break; }
            }
            int next = (idx + 1) % (int)children_.size();
            if (focusedChild_) focusedChild_->onFocusLoss();
            focusedChild_ = children_[next].get();
            focusedChild_->onFocusGain();
        }
        return true;
    }
    return false;
}

bool Container::onTextInput(char c) {
    if (focusedChild_) return focusedChild_->onTextInput(c);
    return false;
}

void Container::onFocusGain() {
    if (focusedChild_) focusedChild_->onFocusGain();
}

void Container::onFocusLoss() {
    if (focusedChild_) focusedChild_->onFocusLoss();
    focusedChild_ = nullptr;
}

void Container::draw(Surface& surface) {
    // Draw background
    drawRoundedRect(surface, bgColor, borderColor, 1);
    // Draw children
    for (auto& c : children_) {
        if (c->isVisible()) {
            c->draw(surface);
        }
    }
}

// ── VBox ──────────────────────────────────────────────────────────
void VBox::draw(Surface& surface) {
    // Layout children
    int y = rect_.y;
    for (auto& c : children_) {
        c->setPos(rect_.x, y);
        c->setSize(rect_.w, c->height());
        y += c->height() + spacing;
    }
    Container::draw(surface);
}

// ── HBox ──────────────────────────────────────────────────────────
void HBox::draw(Surface& surface) {
    // Layout children
    int x = rect_.x;
    for (auto& c : children_) {
        c->setPos(x, rect_.y);
        c->setSize(c->width(), rect_.h);
        x += c->width() + spacing;
    }
    Container::draw(surface);
}

} // namespace gui
} // namespace eng

