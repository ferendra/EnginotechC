// EnginotechC++ — Embedded ADC HAL
// Portable ADC abstraction for Arduino and ESP32

#ifndef ENG_EMBEDDED_ADC_H
#define ENG_EMBEDDED_ADC_H

#include <cstdint>

namespace eng {
namespace embedded {
namespace adc {

struct ADCChannel {
    int pin;
    uint16_t bitResolution;

    uint16_t read() const;
    uint16_t readAverage(int samples = 4) const;
    float readVoltage() const;
    float readVoltage(float vref) const;
    void setResolution(int bits);
    void setAttenuation(int attenuation);  // ESP32: 0=0dB, 1=2.5dB, 2=6dB, 3=11dB
};

// Open an ADC channel on the given pin
ADCChannel open(int pin);

// Get max ADC value for current resolution
uint16_t maxVal(int bits = 12);

} // namespace adc
} // namespace embedded
} // namespace eng

#endif // ENG_EMBEDDED_ADC_H
