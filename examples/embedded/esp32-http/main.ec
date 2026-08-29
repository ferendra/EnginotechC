// EnginotechC++ — Embedded Example: ESP32 WiFi
// WiFi connectivity demo

import esp32;
import embedded.gpio;
import embedded.system;
import embedded.uart;
import embedded.wifi;

fn setup() {
    pinMode(2, PinMode.OUTPUT);
    digitalWrite(2, PinState.LOW);

    uart_open_baud(0, 115200);
    uart_write(0, "Starting ESP32 WiFi Demo...\n");

    // Connect to WiFi
    let config = WifiConfig {
        ssid: "YOUR_SSID",
        password: "YOUR_PASSWORD",
        mode: WifiMode.STA
    };
    wifi_init(WifiMode.STA);
    wifi_connect(config);

    while !wifi_is_connected() {
        uart_write(0, ".");
        delay(100);
    }
    uart_write(0, "\nConnected! IP: " + wifi_get_ip() + "\n");

    digitalWrite(2, PinState.HIGH);
    uart_write(0, "WiFi connected at " + wifi_get_ip() + "\n");
}

fn loop() {
    // Check connection periodically
    if !wifi_is_connected() {
        uart_write(0, "WiFi disconnected!\n");
    }
    delay(1000);
}

fn main() {
    setup();
    while (true) {
        loop();
    }
}