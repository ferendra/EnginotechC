// EnginotechC++ — Servo Motor Package
// PWM-based servo control

#ifndef ENG_SERVO_H
#define ENG_SERVO_H

#include <cstdint>

namespace eng {
namespace servo {

struct Servo {
    int pin;
    int channel;
    uint16_t min_pulse_us;  // 500-1500 typical
    uint16_t max_pulse_us;  // 1500-2500 typical
    uint32_t freq_hz;       // 50Hz typical
};

Servo attach(int pin, uint16_t min_us = 500, uint16_t max_us = 2500);
void detach(Servo& s);
void write(Servo& s, int angle);      // 0-180 degrees
int read(Servo& s);                   // Current angle
bool attached(Servo& s);

} // namespace servo
} // namespace eng

#endif // ENG_SERVO_H
