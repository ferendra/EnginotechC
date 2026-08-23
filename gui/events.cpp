// EnginotechC++ — GUI Events implementation

#include "events.h"

namespace eng {
namespace gui {

// Standard X11 keysym values (stable ABI constants, safe to hardcode).
uint32_t translateKeysym(uint32_t k) {
    if (k >= ' ' && k <= '~') return k;          // ASCII range maps 1:1
    switch (k) {
        case 0xFF0D: return SpecialKey::Return;
        case 0xFF08: return SpecialKey::Backspace;
        case 0xFFFF: return SpecialKey::Delete;
        case 0xFF1B: return SpecialKey::Escape;
        case 0xFF51: return SpecialKey::Left;
        case 0xFF52: return SpecialKey::Up;
        case 0xFF53: return SpecialKey::Right;
        case 0xFF54: return SpecialKey::Down;
        case 0xFF09: return SpecialKey::Tab;
        case 0xFF50: return SpecialKey::Home;
        case 0xFF57: return SpecialKey::End;
        default:     return SpecialKey::None;
    }
}

} // namespace gui
} // namespace eng
