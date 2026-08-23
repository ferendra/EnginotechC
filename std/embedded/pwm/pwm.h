// EnginotechC++ — Embedded PWM HAL
// Portable PWM abstraction for Arduino and ESP32

#ifndef ENG_EMBEDDED_PWM_H
#define ENG_EMBEDDED_PWM_H

#include <cstdint>
#include <map>

namespace eng {
namespace embedded {
namespace pwm {

struct PWMPin {
    int channel;
    int pin;
    uint32_t freq_hz;
    uint16_t duty_percent;

    void set_frequency(uint32_t hz);
    void set_duty(uint16_t percent);
    void set_duty_ticks(uint16_t ticks, uint16_t maxTicks);
    void stop();
    void start();
    bool is_running() const;
};

// Open PWM on a pin with given frequency
PWMPin open(int pin, uint32_t frequencyHz = 1000);

// Get the max duty cycle for current target
uint16_t maxDutyCycle();

} // namespace pwm
} // namespace embedded
} // namespace eng

#endif // ENG_EMBEDDED_PWM_H
