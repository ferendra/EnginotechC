// EnginotechC++ — Power Management Package (ESP32 Deep Sleep)
// Low-power modes: deep sleep, light sleep, wake sources

#ifndef ENG_POWER_H
#define ENG_POWER_H

#include <cstdint>
#include <cstddef>

namespace eng {
namespace power {

// Sleep modes
enum class Mode : uint8_t {
    LIGHT_SLEEP = 0,
    DEEP_SLEEP  = 1
};

struct DeepSleepConfig {
    uint64_t duration_us;     // Sleep duration in microseconds
    uint32_t wakeup_gpio;     // GPIO to wake on (0 = none)
    uint32_t wakeup_level;    // 0 = low, 1 = high
};

DeepSleepConfig deep_sleep_config(uint64_t duration_us,
                                   uint32_t wakeup_gpio = 0,
                                   uint32_t wakeup_level = 0);

void enter_deep_sleep(const DeepSleepConfig& cfg);
void enter_light_sleep(uint32_t duration_us);
uint32_t get_wakeup_reason();
void enable_gpio_wakeup(uint32_t gpio, uint32_t level);
void enable_timer_wakeup(uint64_t us);
void clear_wakeup_reason();

// Helper: sleep for N seconds
void sleep_seconds(uint32_t secs);
void sleep_minutes(uint32_t mins);

// Power stats
size_t get_deep_sleep_bb_mem_bytes();
size_t get_deep_sleep_fast_mem_bytes();
size_t get_deep_sleep_slow_mem_bytes();

} // namespace power
} // namespace eng

#endif // ENG_POWER_H
