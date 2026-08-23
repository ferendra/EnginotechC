// EnginotechC++ — Embedded Example: Arduino Button + LED
// Read a button and control an LED

import arduino;
import embedded.gpio;

fn setup() {
    let led = gpio.output(13);
    let btn = gpio.input(2);
    btn.setMode(embedded.gpio.Mode.INPUT_PULLUP);

    uart.begin(9600);
    uart.write("Button test ready\n");
}

fn loop() {
    let btn = gpio.input(2);
    let led = gpio.output(13);

    if btn.read() == embedded.gpio.PinState.LOW {
        led.high();
        uart.write("Button pressed\n");
        delay(200);  // Debounce
    } else {
        led.low();
    }

    delay(10);
}

fn main() {
    setup();
    loop();
}
