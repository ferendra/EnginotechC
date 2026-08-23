// EnginotechC++ — GUI Widget Framework
// Base classes and common types for UI widgets.

#pragma once
#include "renderer.h"
#include "theme.h"
#include "events.h"
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

namespace eng {
namespace gui {

// ── Layout hints ───────────────────────────────────────────────────
enum class Alignment { Left, Center, Right };
enum class FillMode { None, Horizontal, Vertical, Both };

struct Rect {
    int x = 0, y = 0, w = 0, h = 0;
    bool contains(int mx, int my) const {
        return mx >= x && mx < x + w && my >= y && my < y + h;
    }
};

// ── Widget base class ─────────────────────────────────────────────
class Widget {
public:
    using ClickCallback   = std::function<void()>;
    using ChangeCallback  = std::function<void()>;
    using TextCallback    = std::function<void(const std::string&)>;

    virtual ~Widget() = default;

    // Theme access
    static Theme& theme() { return *Theme::active(); }
    void setThemeOverride(const ThemeColors* t) { themeOverride_ = t; }
    void clearThemeOverride() { themeOverride_ = nullptr; }

    // Geometry
    void setRect(Rect r)       { rect_ = r; }
    const Rect& rect() const   { return rect_; }
    void setPos(int x, int y)  { rect_.x = x; rect_.y = y; }
    void setSize(int w, int h) { rect_.w = w; rect_.h = h; }
    int  x()  const { return rect_.x; }
    int  y()  const { return rect_.y; }
    int  width()  const { return rect_.w; }
    int  height() const { return rect_.h; }

    // Visibility & interaction
    void setVisible(bool v)     { visible_ = v; }
    bool isVisible() const      { return visible_; }
    void setEnabled(bool e)     { enabled_ = e; }
    bool isEnabled() const      { return enabled_; }
    void setFocus(bool f)       { focused_ = f; }
    bool isFocused() const      { return focused_; }

     // Styling (auto-resolved from theme when not overridden)
     Color bgColor  = Color{};   // resolved from theme.panelBg by default
     Color fgColor  = Color{};   // resolved from theme.fg by default
     Color borderColor = Color{}; // resolved from theme.border by default
     Color hoverColor  = Color{}; // resolved from theme.accentHover by default
     Color activeColor = Color{}; // resolved from theme.accentActive by default
     int  borderRadius = 0;      // 0 = use theme default
     bool themeDriven = true;    // if true, colors come from active theme
     bool fontBold = false;      // override theme bold setting for this widget

    // Callbacks
    ClickCallback   onClick;
    ChangeCallback  onChange;
    TextCallback    onText;

    // Event handling (returns true if consumed)
    virtual bool onMouseDown(int mx, int my, int button);
    virtual bool onMouseUp  (int mx, int my, int button);
    virtual bool onMouseMove(int mx, int my);
    virtual bool onKeyDown  (uint32_t key);
    virtual bool onTextInput(char c);
    virtual void  onFocusGain();
    virtual void  onFocusLoss();

    // Rendering
    virtual void draw(Surface& surface) = 0;

protected:
    Rect rect_ = {0, 0, 100, 30};
    bool visible_  = true;
    bool enabled_  = true;
    bool focused_  = false;
    bool hovered_  = false;

    // Helpers
    void drawRoundedRect(Surface& s, Color fill, Color border, int thickness = 1);
    void drawTextCentered(Surface& s, const std::string& text, Color color, int yOff = 0);
    bool isPointInside(int mx, int my) const { return rect_.contains(mx, my); }

    const ThemeColors* themeOverride_ = nullptr;
    Color resolveTheme(Color Widget::*member) const;
};

// ── Container widget (holds child widgets) ────────────────────────
class Container : public Widget {
public:
    void add(std::shared_ptr<Widget> w)  { children_.push_back(w); }
    void remove(Widget* w) {
        children_.erase(std::remove_if(children_.begin(), children_.end(),
            [w](const std::shared_ptr<Widget>& c){ return c.get() == w; }),
            children_.end());
    }
    const std::vector<std::shared_ptr<Widget>>& children() const { return children_; }
    std::vector<std::shared_ptr<Widget>> children() { return children_; }

    bool onMouseDown(int mx, int my, int button) override;
    bool onMouseUp  (int mx, int my, int button) override;
    bool onMouseMove(int mx, int my) override;
    bool onKeyDown  (uint32_t key) override;
    bool onTextInput(char c) override;
    void onFocusGain() override;
    void onFocusLoss() override;
    void draw(Surface& surface) override;

protected:
    std::vector<std::shared_ptr<Widget>> children_;
    Widget* focusedChild_ = nullptr;
};

// ── Simple layout managers ────────────────────────────────────────
class VBox : public Container {
public:
    int spacing = 8;
    void draw(Surface& surface) override;
};

class HBox : public Container {
public:
    int spacing = 8;
    void draw(Surface& surface) override;
};

} // namespace gui
} // namespace eng
