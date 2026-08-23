// Host simulation runtime for EnginotechC++ Arduino sketches (TESTING ONLY).
// Implements the GPIO/delay surface of Arduino.h with stdout tracing and a
// virtual clock so generated sketches can be verified without hardware.

#include "Arduino.h"

SimSerial Serial;

static int pinStates[100] = {0};
static int pinModes[100] = {-1};
static long simClockMs = 0;
static long simDeadlineMs = -1; // when >0, delay() exits the program at deadline

void pinMode(int pin, int mode) {
    if (pin >= 0 && pin < 100) {
        pinModes[pin] = mode;
        std::printf("[gpio] pinMode(%d, %s)\n", pin,
                    mode == OUTPUT ? "OUTPUT" : mode == INPUT_PULLUP ? "INPUT_PULLUP" : "INPUT");
        std::fflush(stdout);
    }
}

void digitalWrite(int pin, int value) {
    if (pin >= 0 && pin < 100) {
        if (pinStates[pin] != value) {
            std::printf("[gpio] digitalWrite(%d, %s)\n", pin, value ? "HIGH" : "LOW");
            std::fflush(stdout);
        }
        pinStates[pin] = value;
    }
}

int digitalRead(int pin) {
    return (pin >= 0 && pin < 100) ? pinStates[pin] : 0;
}

static void advanceAndMaybeExit(long ms) {
    simClockMs += ms;
    if (simDeadlineMs < 0) {
        const char* env = std::getenv("EC_SIM_MS");
        simDeadlineMs = env ? std::atol(env) : 2000; // default: stop after 2 virtual seconds
    }
    if (simClockMs >= simDeadlineMs) {
        std::printf("[sim] time budget reached (%ld ms), exiting\n", simClockMs);
        std::fflush(stdout);
        std::exit(0);
    }
}

void delay(long ms) { advanceAndMaybeExit(ms); }
void delay(unsigned long ms) { advanceAndMaybeExit((long)ms); }
void delay(int ms) { advanceAndMaybeExit(ms); }

unsigned long millis() { return (unsigned long)simClockMs; }
