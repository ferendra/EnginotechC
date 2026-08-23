// EnginotechC++ — Engine Core for Embedded
// Bridge between compiler output and embedded hardware runtime
// This file is compiled alongside user code for embedded targets

#ifndef ENG_ENGINECORE_EMBEDDED_H
#define ENG_ENGINECORE_EMBEDDED_H

#include <cstdint>
#include <cstddef>

// ---- Target macros (set by compiler based on --target / --board) ----
// ENG_TARGET_ARDUINO    — AVR-based Arduino boards
// ENG_TARGET_ESP32      — ESP32 Xtensa
// ENG_TARGET_ESP32S3    — ESP32-S3 RISC-V
// ENG_TARGET_ESP32C3    — ESP32-C3 RISC-V

#ifdef __AVR__
    #define ENG_TARGET_ARDUINO 1
    #define ENG_CPU_FREQ_HZ   F_CPU
#elif defined(ESP32)
    #define ENG_TARGET_ESP32 1
    #define ENG_CPU_FREQ_HZ   240000000
#elif defined(ESP32S3)
    #define ENG_TARGET_ESP32S3 1
    #define ENG_CPU_FREQ_HZ   240000000
#elif defined(ESP32C3)
    #define ENG_TARGET_ESP32C3 1
    #define ENG_CPU_FREQ_HZ   160000000
#else
    // Host / emulation mode
    #define ENG_TARGET_HOST 1
    #define ENG_CPU_FREQ_HZ   0
#endif

// ---- Platform includes ----
#ifdef ENG_TARGET_ARDUINO
    #include <avr/io.h>
    #include <avr/interrupt.h>
    #include <util/delay.h>
    #include <stdio.h>
#elif defined(ENG_TARGET_ESP32) || defined(ENG_TARGET_ESP32S3) || defined(ENG_TARGET_ESP32C3)
    // ESP-IDF headers would be included here
    // #include <esp_system.h>
    // #include <esp_wifi.h>
    // #include <driver/uart.h>
    // #include <driver/gpio.h>
    #include <stdio.h>
#endif

namespace eng {
namespace enginecore {

// ---- EC Runtime Entry Point ----
// Called before main() by the platform startup code
void ecInit();

// ---- Hardware Abstraction ----
void ecDelayMs(uint32_t ms);
void ecDelayUs(uint32_t us);
uint32_t ecMillis();
uint32_t ecMicros();

// GPIO
void ecPinMode(uint32_t pin, uint32_t mode);
void ecDigitalWrite(uint32_t pin, uint32_t level);
uint32_t ecDigitalRead(uint32_t pin);

// UART
void ecUartBegin(uint32_t port, uint32_t baud);
void ecUartWrite(uint32_t port, const char* s);
int ecUartRead(uint32_t port);
int ecUartAvailable(uint32_t port);

// PWM
void ecPwmSetup(uint32_t pin, uint32_t channel, uint32_t freq, uint32_t resolution);
void ecPwmWrite(uint32_t channel, uint32_t duty);
void ecPwmStop(uint32_t channel);

// ADC
uint32_t ecAdcRead(uint32_t pin);
void ecAdcSetResolution(uint32_t pin, uint32_t bits);

// Interrupts
void ecAttachInterrupt(uint32_t pin, void (*callback)(void), uint32_t edge);
void ecDetachInterrupt(uint32_t pin);
void ecInterruptsEnable();
void ecInterruptsDisable();
bool ecInterruptsEnabled();

// System
size_t ecFreeHeap();
size_t ecTotalHeap();
const char* ecChipModel();
uint32_t ecChipFreq();

} // namespace enginecore
} // namespace eng

// ---- C-compatible macros for generated code ----
#define EC_PIN_MODE(pin, mode)  eng::enginecore::ecPinMode(pin, mode)
#define EC_DIGITAL_WRITE(pin, val) eng::enginecore::ecDigitalWrite(pin, val)
#define EC_DIGITAL_READ(pin)    eng::enginecore::ecDigitalRead(pin)
#define EC_DELAY_MS(ms)         eng::enginecore::ecDelayMs(ms)
#define EC_DELAY_US(us)         eng::enginecore::ecDelayUs(us)
#define EC_MILLIS()             eng::enginecore::ecMillis()
#define EC_MICROS()             eng::enginecore::ecMicros()

#endif // ENG_ENGINECORE_EMBEDDED_H
