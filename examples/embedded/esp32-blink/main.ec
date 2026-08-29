// EnginotechC++ — Embedded Example: ESP32 Blink
// A complete, buildable example for ESP32 DevKit

import esp32;
import embedded.gpio;
import embedded.system;
import embedded.uart;

fn setup() {
    pinMode(2, PinMode.OUTPUT);
    uart_open_baud(0, 115200);
    uart_write(0, "ESP32 blinking LED on GPIO 2\n");
}

fn loop() {
    digitalWrite(2, PinState.HIGH);
    delay(500);
    digitalWrite(2, PinState.LOW);
    delay(500);
}

fn main() {
    setup();
    while (true) {
        loop();
    }
}