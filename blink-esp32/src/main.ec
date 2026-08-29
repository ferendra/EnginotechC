// EnginotechC++ — ESP32 Project: blink-esp32
import embedded.gpio;
import embedded.uart;
import embedded.system;

fn setup() {
    pinMode(2, PinMode.OUTPUT);
    digitalWrite(2, PinState.HIGH);
    uart_open_baud(0, 115200);
    uart_write(0, "Project blink-esp32 started\n");
    uart_write(0, "Chip: " + get_chip_id() + "\n");
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