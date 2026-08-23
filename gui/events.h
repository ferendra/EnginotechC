// EnginotechC++ — GUI Events
// Unified user-interaction model: mouse, keyboard, window lifecycle.
// Backends (X11/headless) translate OS events into these.

#ifndef ENG_GUI_EVENTS_H
#define ENG_GUI_EVENTS_H

#include <cstdint>
#include <string>
#include <vector>

namespace eng {
namespace gui {

enum class EventType {
    None,
    MouseMove,     // mouse x/y changed
    MouseDown,     // button pressed  (button: 1=left, 2=middle, 3=right)
    MouseUp,       // button released
    MouseWheel,    // delta = +1 up / -1 down
    KeyDown,       // key pressed     (key = ASCII if printable, else special code)
    KeyUp,         // key released
    TextInput,     // text character ready for widgets
    WindowResize,
    WindowClose,   // user asked to close the window
};

struct SpecialKey {
    static constexpr uint32_t None = 0;
    static constexpr uint32_t Return = 0x01000001;
    static constexpr uint32_t Backspace = 0x01000002;
    static constexpr uint32_t Delete = 0x01000003;
    static constexpr uint32_t Escape = 0x01000004;
    static constexpr uint32_t Left = 0x01000005;
    static constexpr uint32_t Right = 0x01000006;
    static constexpr uint32_t Up = 0x01000007;
    static constexpr uint32_t Down = 0x01000008;
    static constexpr uint32_t Tab = 0x01000009;
    static constexpr uint32_t Home = 0x0100000A;
    static constexpr uint32_t End = 0x0100000B;
};

struct Event {
    EventType type = EventType::None;

    // mouse
    int mouseX = -1, mouseY = -1;
    int button = 0;        // 1 left / 2 middle / 3 right
    int wheelDelta = 0;    // +1 up, -1 down

    // keyboard
    uint32_t key = 0;      // ASCII char or SpecialKey code
    char text = '\0';      // printable character for TextInput

    // window
    int width = 0, height = 0;
};

class EventQueue {
public:
    void push(const Event& e) { queue_.push_back(e); }
    bool pop(Event& out) {
        if (queue_.empty()) return false;
        out = queue_.front();
        queue_.erase(queue_.begin());
        return true;
    }
    bool empty() const { return queue_.empty(); }
    void clear() { queue_.clear(); }

private:
    std::vector<Event> queue_;
};

// Tracks current mouse position/buttons and pressed-key set for convenience.
class InputState {
public:
    int mouseX() const { return mx_; }
    int mouseY() const { return my_; }
    bool mouseDown(int btn = 1) const { return buttons_ & (1u << btn); }
    bool keyDown(uint32_t key) const { return keys_.count(key) != 0; }

    void apply(const Event& e) {
        switch (e.type) {
            case EventType::MouseMove:
                mx_ = e.mouseX; my_ = e.mouseY;
                break;
            case EventType::MouseDown:
                buttons_ |= (1u << e.button);
                break;
            case EventType::MouseUp:
                buttons_ &= ~(1u << e.button);
                break;
            case EventType::KeyDown:
                keys_.insert(e.key);
                break;
            case EventType::KeyUp:
                keys_.erase(e.key);
                break;
            default:
                break;
        }
    }

private:
    int mx_ = -1, my_ = -1;
    unsigned buttons_ = 0;
    struct KeySet {
        std::vector<uint32_t> v;
        size_t count(uint32_t k) const {
            for (auto x : v) if (x == k) return 1;
            return 0;
        }
        void insert(uint32_t k) { if (!count(k)) v.push_back(k); }
        void erase(uint32_t k) {
            for (size_t i = 0; i < v.size(); ++i)
                if (v[i] == k) { v.erase(v.begin() + i); return; }
        }
    } keys_;
};

// Maps X11 keysyms to portable codes without needing X11 headers.
uint32_t translateKeysym(uint32_t keysym);

} // namespace gui
} // namespace eng

#endif // ENG_GUI_EVENTS_H
