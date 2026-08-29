// EnginotechC++ — Embedded Example: Arduino Button + LED
// Read a button and control an LED

import arduino;
import embedded.gpio;
import embedded.uart;

fn setup() {
    pinMode(13, PinMode.OUTPUT);
    pinMode(2, PinMode.INPUT_PULLUP);

    uart_open_baud(0, 9600);
    uart_write(0, "Button test ready\n");
}

fn loop() {
    let btn_state = digitalRead(2);
    if btn_state == PinState.LOW {
        digitalWrite(13, PinState.HIGH);
        uart_write(0, "Button pressed\n");
        delay(200);  // Debounce
    } else {
        digitalWrite(13, PinState.LOW);
    }

    delay(10);
}

fn main() {
    setup();
    while (true) {
        loop();
    }
}