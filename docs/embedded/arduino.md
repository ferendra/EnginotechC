# EnginotechC++ — Arduino Development Guide

## Overview

EC supports Arduino boards through the AVR backend. The compiler generates code
compatible with the Arduino core libraries.

## Supported Boards

| Board | MCU | Clock | Flash | RAM | UART | SPI | I2C |
|-------|-----|-------|-------|-----|------|-----|-----|
| Arduino Uno | ATmega328P | 16MHz | 32KB | 2KB | 1 | Yes | Yes |
| Arduino Nano | ATmega328P | 16MHz | 32KB | 2KB | 1 | Yes | Yes |
| Arduino Mega | ATmega2560 | 16MHz | 256KB | 8KB | 4 | Yes | Yes |

## Pin Mapping

### Arduino Uno/Nano
| Function | Pins |
|----------|------|
| Digital I/O | 0-13 |
| PWM | 3, 5, 6, 9, 10, 11 |
| Analog Input | A0-A5 (pins 14-19) |
| I2C SDA | 4 |
| I2C SCL | 5 |
| SPI MOSI | 11 |
| SPI MISO | 12 |
| SPI SCK | 13 |
| SPI SS | 10 |
| UART TX | 1 |
| UART RX | 0 |
| Built-in LED | 13 |

### Arduino Mega
| Function | Pins |
|----------|------|
| Digital I/O | 0-53 |
| PWM | 2-13, 44-46 |
| Analog Input | A0-A15 (pins 54-69) |
| I2C SDA | 20 |
| I2C SCL | 21 |
| SPI MOSI | 51 |
| SPI MISO | 50 |
| SPI SCK | 52 |
| SPI SS | 53 |
| UART TX/RX | 0/1, 18/19, 16/17, 14/15 |

## Example: Blink

```ec
import embedded.gpio;
import embedded.system;

fn setup() {
    let led = gpio.output(13);
}

fn loop() {
    let led = gpio.output(13);
    led.high();
    delay(500);
    led.low();
    delay(500);
}

fn main() {
    setup();
    loop();
}
```

## Example: Button Input

```ec
import embedded.gpio;

fn setup() {
    let led = gpio.output(13);
    let btn = gpio.input(2);
    btn.setMode(gpio.Mode.INPUT_PULLUP);
}

fn loop() {
    let btn = gpio.input(2);
    let led = gpio.output(13);

    if btn.read() == gpio.PinState.LOW {
        led.high();
    } else {
        led.low();
    }
    delay(50);
}

fn main() {
    setup();
    loop();
}
```

## Example: PWM LED Dimming

```ec
import embedded.pwm;
import embedded.system;

fn setup() {
    let pwm = pwm.open(9, 1000);  // Pin 9, 1kHz
}

fn loop() {
    let pwm = pwm.open(9, 1000);
    // Sweep from 0% to 100% duty
    for i in range(0, 100) {
        pwm.duty(i);
        delay(20);
    }
    // Sweep from 100% to 0% duty
    for i in range(100, 0, -1) {
        pwm.duty(i);
        delay(20);
    }
}

fn main() {
    setup();
    loop();
}
```

## Building for Arduino

```bash
# Create project
engc new my-arduino-app --target arduino --board arduino-uno

# Build
cd my-arduino-app
engc build --target arduino --board arduino-uno

# Flash
engc flash --board arduino-uno --port COM3

# Monitor
engc monitor --baud 115200 --port COM3
```

## Memory Constraints

| Board | Flash | RAM | EEPROM |
|-------|-------|-----|--------|
| Uno | 32 KB | 2 KB | 1 KB |
| Nano | 32 KB | 2 KB | 1 KB |
| Mega | 256 KB | 8 KB | 4 KB |

**Tips for memory-constrained boards:**
- Use `embedded-min` runtime profile
- Avoid heap allocation in hot loops
- Use `const` for string literals to store them in flash
- Keep global variables minimal
