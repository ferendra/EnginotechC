// EnginotechC++ — Arduino Shim (runtime header)
// This file is meant to be INCLUDED IN THE GENERATED .ino sketch,
// NOT compiled into the compiler binary. It provides the runtime
// bridge from EC's embedded HAL to Arduino core APIs.
//
// When included in the compiler source tree, it uses stub types so
// the compiler itself compiles on host (Linux/macOS) without the
// Arduino SDK installed.

#pragma once

// When compiling the EC compiler itself (not the target sketch),
// ARDUINO_GEN is not defined — provide stub types.
#ifndef ARDUINO
#include <cstdint>
#include <string>

// Stub types for compiler host build
struct EcOutPin {
    int pin = 0;
    void high() {}
    void low() {}
    void toggle() {}
    bool read() const { return false; }
};
struct EcInPin {
    int pin = 0;
    int read() { return 0; }
    void setMode(int) {}
};
struct EcPwmPin {
    int pin = 0;
    void start(int, int) {}
    void stop() {}
    void setDuty(int) {}
};
inline EcOutPin ec_gpio_output(int p) { return {p}; }
inline EcInPin  ec_gpio_input(int p)  { return {p}; }
inline EcInPin  ec_gpio_input_pullup(int p) { return {p}; }
inline EcPwmPin ec_gpio_pwm(int p)   { return {p}; }

#else
// Real Arduino target build — include the actual Arduino core
#include <Arduino.h>

// Output pin wrapper
struct EcOutPin {
    int pin;
    void high()      { digitalWrite(pin, HIGH); }
    void low()       { digitalWrite(pin, LOW);  }
    void toggle()    { digitalWrite(pin, digitalRead(pin) ? LOW : HIGH); }
    bool read() const { return digitalRead(pin) == HIGH; }
};

// Input pin wrapper
struct EcInPin {
    int pin;
    int read()         { return digitalRead(pin); }
    void setMode(int m){ pinMode(pin, m); }
};

// PWM pin wrapper
struct EcPwmPin {
    int pin;
    void start(int freq, int duty) {
        pinMode(pin, OUTPUT);
        analogWriteFrequency(freq);
        analogWrite(pin, duty);
    }
    void stop()    { analogWrite(pin, 0); }
    void setDuty(int d) { analogWrite(pin, d); }
};

// GPIO factory helpers
static EcOutPin ec_gpio_output(int p)        { pinMode(p, OUTPUT); return {p}; }
static EcInPin  ec_gpio_input(int p)         { pinMode(p, INPUT);  return {p}; }
static EcInPin  ec_gpio_input_pullup(int p)  { pinMode(p, INPUT_PULLUP); return {p}; }
static EcPwmPin ec_gpio_pwm(int p)           { pinMode(p, OUTPUT); return {p}; }
#endif

