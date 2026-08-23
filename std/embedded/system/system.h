// EnginotechC++ — Embedded System HAL
// Core system utilities: delay, reset, watchdog, memory info

#ifndef ENG_EMBEDDED_SYSTEM_H
#define ENG_EMBEDDED_SYSTEM_H

#include <cstdint>
#include <cstddef>

namespace eng {
namespace embedded {
namespace system {

// Blocking delay in milliseconds
void delay(uint32_t ms);

// Blocking delay in microseconds
void delayMicroseconds(uint32_t us);

// Milliseconds since boot (non-blocking)
uint32_t millis();

// Microseconds since boot (non-blocking)
uint32_t micros();

// Soft reset the MCU
void reset();

// Get free heap memory in bytes
size_t freeHeap();

// Get total heap memory
size_t totalHeap();

// Get minimum free heap since boot (ESP32)
size_t minFreeHeap();

// Enable/disable watchdog timer
void watchdogEnable(uint32_t timeoutMs);
void watchdogFeed();
void watchdogDisable();

// Read unique device ID (64-bit)
uint64_t deviceUniqueId();

// Read CPU frequency in Hz
uint32_t cpuFrequencyHz();

} // namespace system
} // namespace embedded
} // namespace eng

#endif // ENG_EMBEDDED_SYSTEM_H
