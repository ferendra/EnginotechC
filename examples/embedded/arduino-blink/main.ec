// EnginotechC++ — Embedded Example: Arduino Blink
// A complete, buildable example for Arduino Uno

import arduino;

fn main() {
    pinMode(13, PinMode.OUTPUT);

    while (true) {
        digitalWrite(13, PinState.HIGH);
        delay(500);

        digitalWrite(13, PinState.LOW);
        delay(500);
    }
}