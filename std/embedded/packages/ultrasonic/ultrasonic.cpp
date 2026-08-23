// EnginotechC++ — Ultrasonic HC-SR04 Implementation
#include "ultrasonic.h"
#include "../../gpio/gpio.h"
#include "../../system/system.h"
#include <cstdint>

namespace eng {
namespace ultrasonic {

static Ultrasonic g_sensor;
static bool g_connected = false;

Ultrasonic connect(int trig_pin, int echo_pin) {
    Ultrasonic u{};
    u.trig_pin = trig_pin;
    u.echo_pin = echo_pin;
    g_sensor = u;
    g_connected = true;
    return u;
}

static uint32_t measure(Ultrasonic& u) {
    // Send 10us trigger pulse
    eng::embedded::gpio::digitalWrite(u.trig_pin, eng::embedded::gpio::PinState::LOW);
    eng::embedded::system::delay(2);  // 2ms
    eng::embedded::gpio::digitalWrite(u.trig_pin, eng::embedded::gpio::PinState::HIGH);
    eng::embedded::system::delay(10);  // 10us approx
    eng::embedded::gpio::digitalWrite(u.trig_pin, eng::embedded::gpio::PinState::LOW);

    // Measure echo pulse width
    uint32_t start = 0, end = 0;
    while (eng::embedded::gpio::digitalRead(u.echo_pin) == eng::embedded::gpio::PinState::LOW) {}
    start = eng::embedded::system::micros();
    while (eng::embedded::gpio::digitalRead(u.echo_pin) == eng::embedded::gpio::PinState::HIGH) {}
    end = eng::embedded::system::micros();
    return end - start;
}

uint32_t durationUs() { return g_connected ? measure(g_sensor) : 0; }
uint32_t durationUs(Ultrasonic& u) { return measure(u); }

float distanceCm() { return g_connected ? distanceCm(g_sensor) : 0; }
float distanceCm(Ultrasonic& u) {
    uint32_t dur = durationUs(u);
    return dur / 29.0f / 2.0f;  // Speed of sound = 343 m/s
}

float distanceIn() { return distanceCm() / 2.54f; }
float distanceIn(Ultrasonic& u) { return distanceCm(u) / 2.54f; }

} // namespace ultrasonic
} // namespace eng
