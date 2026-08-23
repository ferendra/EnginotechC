// EnginotechC++ — Embedded System Implementation

#include "system.h"
#include <chrono>
#include <thread>
#include <cstdio>
#include <cstdlib>

namespace eng {
namespace embedded {
namespace system {

using Clock = std::chrono::steady_clock;
static Clock::time_point bootTime = Clock::now();

void delay(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void delayMicroseconds(uint32_t us) {
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

void reset() {
    std::abort();
}

size_t freeHeap() {
    // Host fallback: approximate
    return 64ULL * 1024 * 1024;  // 64MB estimate
}

size_t totalHeap() {
    return freeHeap();
}

size_t minFreeHeap() {
    return freeHeap();
}

void watchdogEnable(uint32_t) {}
void watchdogFeed() {}
void watchdogDisable() {}

uint64_t deviceUniqueId() {
    return 0xDEADBEEF;
}

uint32_t cpuFrequencyHz() {
#ifdef ENG_TARGET_ESP32
    return 240000000;  // 240 MHz
#elif defined(ENG_TARGET_ARDUINO_UNO)
    return 16000000;   // 16 MHz
#else
    return 16000000;
#endif
}

} // namespace system
} // namespace embedded
} // namespace eng
