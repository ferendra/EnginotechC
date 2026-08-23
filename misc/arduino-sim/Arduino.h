// Arduino.h — Host simulation shim (testing ONLY, never shipped to devices).
// Emulates the Arduino core API surface used by EnginotechC++ generated
// sketches so they can be compiled and executed on a desktop for verification.

#ifndef ARDUINO_SIM_H
#define ARDUINO_SIM_H

#include <cstdint>
#include <cstdio>
#include <string>
#include <chrono>
#include <thread>

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define INPUT_PULLDOWN 3

class SimSerial {
public:
    void begin(long baud) {
        (void)baud;
        std::printf("[serial] begin %ld baud\n", baud);
    }
    void print(const char* s) { std::printf("%s", s); }
    void print(const std::string& s) { std::printf("%s", s.c_str()); }
    void print(int v) { std::printf("%d", v); }
    void print(double v) { std::printf("%g", v); }
    void println() { std::printf("\n"); }
    void println(const char* s) { std::printf("%s\n", s); }
    void println(const std::string& s) { std::printf("%s\n", s.c_str()); }
    void println(int v) { std::printf("%d\n", v); }
    void println(double v) { std::printf("%g\n", v); }
};

extern SimSerial Serial;

#endif // ARDUINO_SIM_H
