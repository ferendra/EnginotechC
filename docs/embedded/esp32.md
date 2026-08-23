# EnginotechC++ — ESP32 Development Guide

## Overview

EC supports ESP32 family boards through the ESP32 backend. Generates code for
Xtensa (ESP32) or RISC-V (ESP32-S3, ESP32-C3) architectures.

## Supported Boards

| Board | CPU | Clock | Flash | RAM | PSRAM | WiFi | BLE |
|-------|-----|-------|-------|-----|-------|------|-----|
| ESP32 DevKit | ESP32-D0WDQ6 | 240MHz | 4MB | 512KB | 0 | Yes | Yes |
| ESP32-S3 | ESP32-S3-FR-2 | 240MHz | 4MB | 512KB | 8MB | Yes | Yes |
| ESP32-C3 | ESP32-C3-FR-1 | 160MHz | 2MB | 40KB | 0 | Yes | Yes |

## Pin Mapping

### ESP32 DevKit
| Function | Pins |
|----------|------|
| Digital I/O | 0, 2, 4, 5, 12-19, 21-23, 25-27, 32-39 |
| PWM | All digital pins |
| Analog Input | 36-39 (ADC1), 34-35 (ADC2, WiFi shared) |
| I2C SDA | 21 |
| I2C SCL | 22 |
| SPI MOSI | 23 |
| SPI MISO | 19 |
| SPI SCK | 18 |
| SPI SS | 5 |
| UART RX | 3 (GPIO3) |
| UART TX | 1 (GPIO1) |
| Built-in LED | 2 (active high on most boards) |

### ESP32-S3
| Function | Pins |
|----------|------|
| Digital I/O | 0-21, 33-46 |
| PWM | All digital pins |
| Analog Input | 1-5, 6-8 (ADC2) |
| I2C SDA | 8 |
| I2C SCL | 9 |
| SPI (HSPI) MOSI | 40 |
| SPI (HSPI) MISO | 39 |
| SPI (HSPI) SCK | 41 |
| SPI (VSPI) MOSI | 11 |
| SPI (VSPI) MISO | 12 |
| SPI (VSPI) SCK | 13 |

### ESP32-C3
| Function | Pins |
|----------|------|
| Digital I/O | 0-15 |
| PWM | All digital pins |
| Analog Input | 0-4 (ADC1) |
| I2C SDA | 6 |
| I2C SCL | 7 |
| SPI MOSI | 11 |
| SPI MISO | 12 |
| SPI SCK | 13 |

## Example: WiFi Connection

```ec
import embedded.wifi;
import embedded.uart;
import embedded.system;

fn setup() {
    uart.begin(115200);
    uart.write("Connecting to WiFi...\n");

    wifi.begin("YOUR_SSID", "YOUR_PASSWORD");

    while !wifi.isConnected() {
        uart.write(".");
        delay(100);
    }
    uart.write("\nConnected! IP: " + wifi.localIP() + "\n");
}

fn loop() {
    delay(1000);
}

fn main() {
    setup();
    loop();
}
```

## Example: HTTP Server

```ec
import embedded.wifi;
import embedded.uart;
import http;

fn setup() {
    wifi.begin("YOUR_SSID", "YOUR_PASSWORD");
    while !wifi.isConnected() {
        delay(100);
    }
    uart.write("Server at: " + wifi.localIP() + "\n");

    http.serverStart(80);
    http.routeGet("/", fn(req, res) {
        res.html("<h1>EC Web Server</h1><p>Uptime: " + str(system.millis()) + "ms</p>");
    });
    http.routeGet("/status", fn(req, res) {
        res.json("{\"free_heap\":" + str(system.freeHeap()) + "}");
    });
}

fn loop() {
    http.serverTick();
    delay(10);
}

fn main() {
    setup();
    loop();
}
```

## Example: I2C Sensor (BME280)

```ec
import embedded.i2c;
import embedded.uart;

fn setup() {
    uart.begin(115200);

    let bus = i2c.open(0, 100000);  // I2C0, 100kHz
    let sensor = i2c.device(bus, 0x76);  // BME280 address

    // Read temperature
    let temp = sensor.readTemp();
    uart.write("Temperature: " + str(temp) + "C\n");
}

fn loop() {
    delay(1000);
}

fn main() {
    setup();
    loop();
}
```

## Building for ESP32

```bash
# Create project
engc new my-esp32-app --target esp32 --board esp32-devkit

# Build
cd my-esp32-app
engc build --target esp32 --board esp32-devkit

# Flash
engc flash --board esp32-devkit --port /dev/ttyUSB0

# Monitor
engc monitor --baud 115200 --port /dev/ttyUSB0
```

## Deep Sleep Example

```ec
import embedded.sleep;
import embedded.system;

fn setup() {
    // Do some work
    print("Working...");
    delay(1000);
}

fn loop() {
    setup();
    // Deep sleep for 1 minute
    sleep.deepSleep(60000);
}

fn main() {
    setup();
    loop();
}
```

## Memory Notes

- **ESP32**: 520KB RAM, 4MB+ flash typical
- **ESP32-S3**: 520KB RAM + 8MB PSRAM, 4MB+ flash
- **ESP32-C3**: 40KB RAM (very constrained!), 2MB flash
- Use `embedded-min` profile for ESP32-C3
- Avoid heap allocation in interrupt handlers
