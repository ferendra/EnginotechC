// EnginotechC++ — Embedded Runtime Implementation (host fallback)
// On real hardware, these would use direct register access

#include "embedded_rt.h"
#include <chrono>
#include <thread>
#include <cstdio>

namespace eng {
namespace embedded_rt {
namespace {
    using Clock = std::chrono::steady_clock;
    Clock::time_point bootTime = Clock::now();
}  // anon

void delayMs(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void delayUs(uint32_t us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

uint32_t millis() {
    auto now = Clock::now();
    return static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now - bootTime).count());
}

uint32_t micros() {
    auto now = Clock::now();
    return static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(now - bootTime).count());
}

void pinMode(uint32_t, uint32_t) {}
void digitalWrite(uint32_t, uint32_t) {}
uint32_t digitalRead(uint32_t) { return 0; }

void interruptsEnable() {}
void interruptsDisable() {}
bool interruptsEnabled() { return true; }

void softReset() { std::abort(); }
size_t heapSize() { return 64ULL * 1024 * 1024; }
size_t freeHeap() { return heapSize(); }

uint32_t getResetReason() { return 1; }  // Power-on reset
const char* getChipModel() { return "EC-Host"; }
uint32_t getChipRevision() { return 0; }

bool isTargetArduino() { return false; }
bool isTargetESP32() { return false; }
bool isTargetESP32S3() { return false; }
bool isTargetESP32C3() { return false; }

} // namespace embedded_rt
} // namespace eng
