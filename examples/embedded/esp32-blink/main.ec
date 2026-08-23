// EnginotechC++ — Embedded Example: ESP32 Blink
// A complete, buildable example for ESP32 DevKit

import esp32;
import embedded.gpio;
import embedded.system;

fn setup() {
    let led = gpio.output(2);
    led.high();
    uart.write("ESP32 blinking LED on GPIO 2\n");
}

fn loop() {
    let led = gpio.output(2);
    led.high();
    delay(500);
    led.low();
    delay(500);
}

fn main() {
    setup();
    loop();
}
