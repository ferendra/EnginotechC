// EnginotechC++ — Embedded GPIO HAL Implementation
// Platform-specific implementations for Arduino and ESP32

#include "gpio.h"
#include <cstdlib>

namespace eng {
namespace embedded {
namespace gpio {

// ---- Default (generic) implementations ----
// These will be overridden by platform-specific .cpp files at link time

static PinState pinStates[256];

OutputPin output(PinType pin) {
    OutputPin p{};
    p.pin = pin;
    pinStates[pin] = PinState::LOW;
    return p;
}

InputPin input(PinType pin, Mode mode) {
    InputPin p{};
    p.pin = pin;
    (void)mode;
    return p;
}

void digitalWrite(PinType pin, PinState state) {
    pinStates[pin] = state;
}

PinState digitalRead(PinType pin) {
    return pinStates[pin];
}

PinType builtinLed() {
    return 13;  // Default on most boards
}

void OutputPin::high() {
    digitalWrite(pin, PinState::HIGH);
}

void OutputPin::low() {
    digitalWrite(pin, PinState::LOW);
}

void OutputPin::toggle() {
    write(read() ? PinState::LOW : PinState::HIGH);
}

bool OutputPin::read() const {
    return digitalRead(pin) == PinState::HIGH;
}

void OutputPin::write(PinState state) {
    digitalWrite(pin, state);
}

PinState InputPin::read() const {
    return digitalRead(pin);
}

void InputPin::setMode(Mode mode) {
    (void)mode;
    // Platform-specific pull-up/pull-down configuration
}

PinType getPin(const OutputPin& pin) { return pin.pin; }
PinType getPin(const InputPin& pin) { return pin.pin; }

} // namespace gpio
} // namespace embedded
} // namespace eng
