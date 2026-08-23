// EnginotechC++ — Embedded Interrupt Implementation

#include "interrupt.h"
#include <vector>
#include <map>

namespace eng {
namespace embedded {
namespace interrupt {

struct Handler {
    Edge edge;
    ISRCallback callback;
};

static std::map<int, Handler> handlers;
static bool gInterruptsEnabled = true;

int attach(int pin, Edge edge, ISRCallback callback) {
    if (callback == nullptr) return -1;
    handlers[pin] = {edge, std::move(callback)};
    // Platform-specific: configure GPIO interrupt on this pin
    return pin;
}

void detach(int pin) {
    handlers.erase(pin);
}

void enable() {
    gInterruptsEnabled = true;
}

void disable() {
    gInterruptsEnabled = false;
}

bool isEnabled() {
    return gInterruptsEnabled;
}

} // namespace interrupt
} // namespace embedded
} // namespace eng
