// EnginotechC++ — Embedded Runtime Core
// Low-level runtime support for embedded targets
// Provides: delay(), pin init, interrupt enable, reset

#ifndef ENG_EMBEDDED_RUNTIME_H
#define ENG_EMBEDDED_RUNTIME_H

#include <cstdint>
#include <cstddef>

namespace eng {
namespace embedded_rt {

// ---- Clock / Delay ----
void delayMs(uint32_t ms);
void delayUs(uint32_t us);
uint32_t millis();
uint32_t micros();

// ---- Pin Configuration ----
void pinMode(uint32_t pin, uint32_t mode);  // 0=INPUT, 1=OUTPUT, 2=PULLUP, 3=PULLDOWN
void digitalWrite(uint32_t pin, uint32_t level);
uint32_t digitalRead(uint32_t pin);

// ---- Interrupts ----
void interruptsEnable();
void interruptsDisable();
bool interruptsEnabled();

// ---- Reset ----
void softReset();

// ---- Memory ----
size_t heapSize();
size_t freeHeap();

// ---- Boot info ----
uint32_t getResetReason();
const char* getChipModel();
uint32_t getChipRevision();

// ---- Target detection ----
bool isTargetArduino();
bool isTargetESP32();
bool isTargetESP32S3();
bool isTargetESP32C3();

} // namespace embedded_rt
} // namespace eng

#endif // ENG_EMBEDDED_RUNTIME_H
