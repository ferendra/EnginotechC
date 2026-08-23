// EnginotechC++ — Embedded GPIO HAL
// Portable GPIO abstraction for Arduino (AVR) and ESP32 (Xtensa/RISC-V)

#ifndef ENG_EMBEDDED_GPIO_H
#define ENG_EMBEDDED_GPIO_H

#include <cstdint>
#include <functional>

namespace eng {
namespace embedded {
namespace gpio {

enum class Mode { INPUT, OUTPUT, INPUT_PULLUP, INPUT_PULLDOWN };
enum class PinState { LOW, HIGH };

// Platform-specific pin type
#ifdef ENG_TARGET_ARDUINO
using PinType = uint8_t;
#elif defined(ENG_TARGET_ESP32)
using PinType = int32_t;
#else
using PinType = int32_t;
#endif

// Output pin handle
struct OutputPin {
    PinType pin;
    void high();
    void low();
    void toggle();
    bool read() const;
    void write(PinState state);
};

// Input pin handle
struct InputPin {
    PinType pin;
    PinState read() const;
    void setMode(Mode mode);
};

// Configure a pin as output
OutputPin output(PinType pin);

// Configure a pin as input
InputPin input(PinType pin, Mode mode = Mode::INPUT);

// Get pin number from pin handle
PinType getPin(const OutputPin& pin);
PinType getPin(const InputPin& pin);

// Digital write/read
void digitalWrite(PinType pin, PinState state);
PinState digitalRead(PinType pin);

// Get the built-in LED pin for the target board
PinType builtinLed();

} // namespace gpio
} // namespace embedded
} // namespace eng

#endif // ENG_EMBEDDED_GPIO_H
