// EnginotechC++ — Embedded Example: ESP32 GPIO + UART
// Combined GPIO and serial example

import esp32;
import embedded.gpio;
import embedded.uart;
import embedded.system;

fn setup() {
    let led = gpio.output(2);
    led.high();
    delay(200);
    led.low();

    uart.begin(115200);
    uart.write("ESP32 GPIO+UART test\n");
    uart.write("Chip: " + system.getChipModel() + "\n");
    uart.write("Freq: " + str(system.cpuFrequencyHz()) + " Hz\n");
}

fn loop() {
    uart.write("tick " + str(system.millis()) + "ms\n");
    delay(1000);
}

fn main() {
    setup();
    loop();
}
