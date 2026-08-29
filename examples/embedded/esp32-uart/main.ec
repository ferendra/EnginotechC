// EnginotechC++ — Embedded Example: ESP32 GPIO + UART
// Combined GPIO and serial example

import esp32;
import embedded.gpio;
import embedded.system;
import embedded.uart;

fn setup() {
    pinMode(2, PinMode.OUTPUT);
    digitalWrite(2, PinState.HIGH);
    delay(200);
    digitalWrite(2, PinState.LOW);

    uart_open_baud(0, 115200);
    uart_write(0, "ESP32 GPIO+UART test\n");
    uart_write(0, "Chip: " + get_chip_id() + "\n");
    uart_write(0, "Freq: " + str(get_cpu_frequency()) + " MHz\n");
}

fn loop() {
    uart_write(0, "tick " + str(millis()) + "ms\n");
    delay(1000);
}

fn main() {
    setup();
    while (true) {
        loop();
    }
}