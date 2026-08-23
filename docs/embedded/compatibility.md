# EnginotechC++ — Embedded Target Compatibility Matrix

**Last Updated:** 2026-08-20  
**Status:** PHASE E1 — Architecture Foundation

## How to Read This Matrix

| Column | Meaning |
|--------|---------|
| Build | `engc build --target <target> --board <board>` compiles without error |
| Flash | `engc flash --board <board>` can upload firmware |
| GPIO | Digital read/write works |
| UART | Serial communication works |
| I2C | I2C bus communication works |
| SPI | SPI bus communication works |
| PWM | Pulse-width modulation works |
| ADC | Analog-to-digital conversion works |
| WiFi | WiFi station mode works (ESP32 only) |
| BLE | Bluetooth Low Energy works (ESP32 only) |
| Timer | Hardware/software timers work |
| Interrupt | External interrupts work |

Status values:
- **YES** — Verified on real hardware
- **COMPILE** — Compiles but not hardware-verified
- **PARTIAL** — Partial implementation, some features work
- **PLANNED** — Not yet implemented
- **N/A** — Not applicable to this board

## Arduino Boards

| Board | Build | Flash | GPIO | UART | I2C | SPI | PWM | ADC | WiFi | BLE | Timer | Interrupt |
|-------|-------|-------|------|------|-----|-----|-----|-----|------|-----|-------|-----------|
| Arduino Uno | COMPILE | COMPILE | COMPILE | COMPILE | COMPILE | COMPILE | PLANNED | COMPILE | N/A | N/A | PLANNED | PLANNED |
| Arduino Nano | COMPILE | COMPILE | COMPILE | COMPILE | COMPILE | COMPILE | PLANNED | COMPILE | N/A | N/A | PLANNED | PLANNED |
| Arduino Mega | COMPILE | COMPILE | COMPILE | COMPILE | COMPILE | COMPILE | PLANNED | COMPILE | N/A | N/A | PLANNED | PLANNED |

## ESP32 Boards

| Board | Build | Flash | GPIO | UART | I2C | SPI | PWM | ADC | WiFi | BLE | Timer | Interrupt |
|-------|-------|-------|------|------|-----|-----|-----|-----|------|-----|-------|-----------|
| ESP32 DevKit | COMPILE | COMPILE | COMPILE | COMPILE | COMPILE | COMPILE | PLANNED | COMPILE | PLANNED | PLANNED | PLANNED | PLANNED |
| ESP32-S3 | COMPILE | COMPILE | COMPILE | COMPILE | COMPILE | COMPILE | PLANNED | COMPILE | PLANNED | PLANNED | PLANNED | PLANNED |
| ESP32-C3 | COMPILE | COMPILE | COMPILE | COMPILE | COMPILE | COMPILE | PLANNED | COMPILE | PLANNED | PLANNED | PLANNED | PLANNED |

## Notes

- All boards marked COMPILE have been verified at the **architecture level** (code structure, API surface, target dispatch).
- Hardware verification (real board flashing and peripheral testing) is planned for Phase E24.
- PWM implementation is deferred to Phase E13.
- WiFi/BLE are ESP32-only features; Arduino boards show N/A.
