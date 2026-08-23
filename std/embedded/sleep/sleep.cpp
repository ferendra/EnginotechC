// EnginotechC++ — Embedded Sleep Implementation

#include "sleep.h"
#include <thread>
#include <chrono>

namespace eng {
namespace embedded {
namespace sleep {

void lightSleep(uint64_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void deepSleep(uint64_t ms) {
    // On real MCU this would enter STOP mode; here just sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void wake() {}

uint32_t estimateCurrentLight() { return 5000; }   // 5mA estimate
uint32_t estimateCurrentDeep() { return 5; }       // 5uA estimate

} // namespace sleep
} // namespace embedded
} // namespace eng
