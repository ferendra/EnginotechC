// EnginotechC++ — ESP32 Project: blink-esp32
import embedded.gpio;
import embedded.uart;
import embedded.system;

fn setup() {
    let led = gpio.output(2);
    led.high();
    uart.begin(115200);
    uart.write("Project " + str(") + name + R"(" started\n");
    uart.write("Chip: " + system.getChipModel() + "\n");
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
