// EnginotechC++ — Relay Implementation
#include "relay.h"
#include "../../gpio/gpio.h"

namespace eng {
namespace relay {

Relay connect(int pin, bool active_high) {
    Relay r{};
    r.pin = pin;
    r.state = false;
    r.active_high = active_high;
    off(r);  // Start in off state
    return r;
}

void on(Relay& r) {
    r.state = true;
    eng::embedded::gpio::digitalWrite(static_cast<eng::embedded::gpio::PinType>(r.pin),
        r.active_high ? eng::embedded::gpio::PinState::HIGH : eng::embedded::gpio::PinState::LOW);
}

void off(Relay& r) {
    r.state = false;
    eng::embedded::gpio::digitalWrite(static_cast<eng::embedded::gpio::PinType>(r.pin),
        r.active_high ? eng::embedded::gpio::PinState::LOW : eng::embedded::gpio::PinState::HIGH);
}

void toggle(Relay& r) {
    if (r.state) off(r); else on(r);
}

bool isOn(Relay& r) { return r.state; }
bool isOff(Relay& r) { return !r.state; }

} // namespace relay
} // namespace eng


