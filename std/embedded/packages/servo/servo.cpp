// EnginotechC++ — Servo Implementation
#include "servo.h"
#include <cmath>

namespace eng {
namespace servo {

Servo attach(int pin, uint16_t min_us, uint16_t max_us) {
    Servo s{};
    s.pin = pin;
    s.channel = pin;  // Simplified — real impl maps to PWM channel
    s.min_pulse_us = min_us;
    s.max_pulse_us = max_us;
    s.freq_hz = 50;
    return s;
}

void detach(Servo& s) { (void)s; }
void write(Servo& s, int angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    // Map angle to pulse width
    uint16_t pulse = s.min_pulse_us + (s.max_pulse_us - s.min_pulse_us) * angle / 180;
    // In production: write PWM compare value
    (void)pulse;
}

int read(Servo& s) { (void)s; return 0; }
bool attached(Servo& s) { return s.pin >= 0; }

} // namespace servo
} // namespace eng
