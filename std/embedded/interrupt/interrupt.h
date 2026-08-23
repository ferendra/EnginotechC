// EnginotechC++ — Embedded Interrupt HAL
// Portable hardware interrupt abstraction
// IMPORTANT: ISR callbacks must be async-signal-safe (no malloc, no printf)

#ifndef ENG_EMBEDDED_INTERRUPT_H
#define ENG_EMBEDDED_INTERRUPT_H

#include <cstdint>
#include <functional>

namespace eng {
namespace embedded {
namespace interrupt {

enum class Edge { NONE, RISING, FALLING, CHANGE };

using ISRCallback = std::function<void()>;

// Attach an interrupt handler to a pin with specified edge detection
// Returns an interrupt handle ID (>= 0) or -1 on failure
int attach(int pin, Edge edge, ISRCallback callback);

// Detach an interrupt handler
void detach(int pin);

// Enable all interrupts (global interrupt enable)
void enable();

// Disable all interrupts (global interrupt disable)
void disable();

// Check if interrupts are currently enabled
bool isEnabled();

// Critical section: save interrupt state, disable, execute body, restore
template<typename F>
auto withInterruptsDisabled(F&& f) -> decltype(f()) {
    bool wasEnabled = isEnabled();
    disable();
    auto result = f();
    if (wasEnabled) enable();
    return result;
}

} // namespace interrupt
} // namespace embedded
} // namespace eng

#endif // ENG_EMBEDDED_INTERRUPT_H
