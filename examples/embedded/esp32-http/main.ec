// EnginotechC++ — Embedded Example: ESP32 WiFi HTTP
// Full IoT demo: WiFi + HTTP server + sensor reading

import esp32;
import embedded.gpio;
import embedded.uart;
import embedded.wifi;
import embedded.system;

fn setup() {
    let led = gpio.output(2);
    led.low();

    uart.begin(115200);
    uart.write("Starting ESP32 IoT Demo...\n");

    // Connect to WiFi
    wifi.begin("YOUR_SSID", "YOUR_PASSWORD");

    while !wifi.isConnected() {
        uart.write(".");
        delay(100);
    }
    uart.write("\nConnected! IP: " + wifi.localIP() + "\n");

    // Start HTTP server on port 80
    http.serverStart(80);
    http.routeGet("/temperature", fn(req, res) {
        let temp = readTemperature();
        res.json("{\"temperature\":" + str(temp) + "}");
    });

    http.routeGet("/", fn(req, res) {
        res.html("<h1>ESP32 IoT Demo</h1><a href='/temperature'>Temperature</a>");
    });

    led.high();
    uart.write("HTTP server running at " + wifi.localIP() + "\n");
}

fn loop() {
    http.serverTick();
    delay(100);
}

fn readTemperature() -> float {
    // Read from ADC channel (simulated)
    let sensor = adc.open(34);
    return sensor.readVoltage();
}

fn main() {
    setup();
    loop();
}
