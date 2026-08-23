# EnginotechC++ — Getting Started with Embedded Development

## Prerequisites

### For Arduino Targets
```bash
# Install llvm-mingw with AVR support (Windows)
# Download from: https://github.com/mstorsjo/llvm-mingw
# Or via WinGet: winget install MartinStorsjo.LLVM-MinGW

# Verify installation
avr-gcc --version
avrdude --version
```

### For ESP32 Targets
```bash
# Install ESP-IDF toolchain
# Option 1: Official ESP-IDF (recommended)
# https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/

# Option 2:llvm-mingw with ESP32 support
# Download from: https://github.com/mstorsjo/llvm-mingw

# Verify installation
xtensa-esp32-elf-gcc --version
esptool.py --version
```

### Common Dependencies
```bash
cmake --version   # Must be 3.16+
clang --version   # For LLVM IR generation
python --version  # For esptool.py
```

## Quick Start

### 1. Check Your Environment
```bash
engc doctor
```

### 2. Create a New Project
```bash
# Arduino project
engc new blink-arduino --target arduino --board arduino-uno

# ESP32 project
engc new blink-esp32 --target esp32 --board esp32-devkit
```

### 3. Build the Project
```bash
cd blink-esp32
engc build --target esp32 --board esp32-devkit
# Or use ec.toml:
engc build
```

### 4. Flash to Device
```bash
engc flash --board esp32-devkit --port COM3
# Auto-detect port:
engc flash --board esp32-devkit
```

### 5. Open Serial Monitor
```bash
engc monitor --baud 115200 --port COM3
```

## Project Structure

```
my-project/
├── ec.toml          # Project configuration
├── src/
│   └── main.ec      # Main source file
├── tests/
├── examples/
├── .gitignore
└── README.md
```

## ec.toml Configuration

```toml
[project]
name = "my-project"
version = "0.1.0"

[target]
target = "esp32"
board = "esp32-devkit"

[build]
optimization = "O2"
debug = false

[embedded]
runtime = "embedded-min"   # or "embedded-full"

[serial]
baud = 115200
```

## Available Boards

### Arduino
- `arduino-uno` — ATmega328P, 16MHz, 32KB flash, 2KB RAM
- `arduino-nano` — ATmega328P, 16MHz, 32KB flash, 2KB RAM
- `arduino-mega` — ATmega2560, 16MHz, 256KB flash, 8KB RAM

### ESP32
- `esp32-devkit` — ESP32-D0WDQ6, 240MHz, 4MB flash, 512KB RAM
- `esp32s3` — ESP32-S3, 240MHz, 4MB flash, 512KB RAM + 8MB PSRAM
- `esp32c3` — ESP32-C3, 160MHz, 2MB flash, 40KB RAM

## Import Statements

```ec
// Core embedded HAL
import embedded.gpio;
import embedded.uart;
import embedded.pwm;
import embedded.adc;
import embedded.i2c;
import embedded.spi;
import embedded.timer;
import embedded.interrupt;
import embedded.system;

// ESP32-only
import embedded.wifi;
import embedded.bluetooth;
import embedded.sleep;
```

## Minimal Example

```ec
import embedded.gpio;
import embedded.system;

fn setup() {
    let led = gpio.output(2);
    led.high();
    delay(100);
    led.low();
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
```

## Runtime Profiles

| Profile | Description | Use Case |
|---------|-------------|----------|
| `embedded-min` | Minimal runtime, no heap, static allocation only | Resource-constrained devices |
| `embedded-full` | Full runtime with heap, strings, collections | Devices with ample memory |

Set in `ec.toml`:
```toml
[embedded]
runtime = "embedded-min"
```

## CLI Reference

| Command | Description |
|---------|-------------|
| `engc build --target <t> --board <b>` | Compile for embedded target |
| `engc flash --board <b> [--port <p>]` | Upload firmware to board |
| `engc monitor [--baud <b>] [--port <p>]` | Open serial monitor |
| `engc doctor` | Check environment and toolchains |
| `engc new <name> --target <t> [--board <b>]` | Create new embedded project |
