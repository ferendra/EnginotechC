// arduino - Arduino-specific types and functions
// Re-exports embedded system functions with Arduino-compatible names

import embedded.gpio;
import embedded.system;

// Arduino-compatible constants
const LED_BUILTIN = 13;
const HIGH = PinState.HIGH;
const LOW = PinState.LOW;
const INPUT = PinMode.INPUT;
const OUTPUT = PinMode.OUTPUT;
const INPUT_PULLUP = PinMode.INPUT_PULLUP;

// Re-export embedded functions with Arduino names
fn pinMode(pin: int, mode: PinMode) -> void { }
fn digitalWrite(pin: int, value: PinState) -> void { }
fn digitalRead(pin: int) -> PinState { return PinState.LOW; }
fn analogRead(pin: int) -> int { return 0; }
fn analogWrite(pin: int, value: int) -> void { }

// Time functions are re-exported from embedded.system:
// delay, delayMicroseconds, millis, micros