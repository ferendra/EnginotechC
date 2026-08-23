// EnginotechC++ — Embedded ADC Implementation

#include "adc.h"
#include <cstdlib>
#include <cmath>

namespace eng {
namespace embedded {
namespace adc {

uint16_t maxVal(int bits) {
    return (uint16_t)((1ULL << bits) - 1);
}

ADCChannel open(int pin) {
    ADCChannel ch{};
    ch.pin = pin;
#ifdef ENG_TARGET_ESP32
    ch.bitResolution = 12;
#else
    ch.bitResolution = 10;  // Arduino Uno ADC is 10-bit
#endif
    return ch;
}

uint16_t ADCChannel::read() const {
    // Platform-specific ADC read
    // Returns simulated value for host testing
    return static_cast<uint16_t>(rand() % (1ULL << bitResolution));
}

uint16_t ADCChannel::readAverage(int samples) const {
    uint32_t sum = 0;
    for (int i = 0; i < samples; ++i) {
        sum += read();
    }
    return static_cast<uint16_t>(sum / samples);
}

float ADCChannel::readVoltage() const {
    return readVoltage(3.3f);  // ESP32 default; Arduino would use 5.0f
}

float ADCChannel::readVoltage(float vref) const {
    return (float)read() * vref / (float)maxVal(bitResolution);
}

void ADCChannel::setResolution(int bits) {
    if (bits >= 9 && bits <= 16) {
        bitResolution = static_cast<uint16_t>(bits);
    }
}

void ADCChannel::setAttenuation(int attenuation) {
    (void)attenuation;
    // ESP32-specific: configure ADC attenuation
}

} // namespace adc
} // namespace embedded
} // namespace eng
