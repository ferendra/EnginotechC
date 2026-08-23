// EnginotechC++ — Embedded Sleep HAL
#ifndef ENG_EMBEDDED_SLEEP_H
#define ENG_EMBEDDED_SLEEP_H

#include <cstdint>

namespace eng {
namespace embedded {
namespace sleep {

// Put the MCU into light sleep for `ms` milliseconds
void lightSleep(uint64_t ms);

// Put the MCU into deep sleep for `ms` milliseconds
// Deep sleep wakes via RTC or external pins
void deepSleep(uint64_t ms);

// Stop all sleep (wake up)
void wake();

// Get estimated current draw in uA (light sleep)
uint32_t estimateCurrentLight();

// Get estimated current draw in uA (deep sleep)
uint32_t estimateCurrentDeep();

} // namespace sleep
} // namespace embedded
} // namespace eng

#endif // ENG_EMBEDDED_SLEEP_H
