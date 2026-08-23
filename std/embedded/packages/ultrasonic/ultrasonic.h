// EnginotechC++ — Ultrasonic HC-SR04 Package
// Distance measurement via echo timer

#ifndef ENG_ULTRASONIC_H
#define ENG_ULTRASONIC_H

#include <cstdint>

namespace eng {
namespace ultrasonic {

struct Ultrasonic {
    int trig_pin;
    int echo_pin;
};

Ultrasonic connect(int trig_pin, int echo_pin);
float distanceCm();           // For connected global instance
float distanceIn();           // Inches
uint32_t durationUs();        // Echo pulse width in microseconds

// With explicit instance
float distanceCm(Ultrasonic& u);
float distanceIn(Ultrasonic& u);
uint32_t durationUs(Ultrasonic& u);

} // namespace ultrasonic
} // namespace eng

#endif // ENG_ULTRASONIC_H
