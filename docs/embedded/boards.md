# EnginotechC++ — Board Definitions Reference

## Directory Structure

```
boards/
├── arduino-uno/
│   └── board.json
├── arduino-nano/
│   └── board.json
├── arduino-mega/
│   └── board.json
├── esp32-devkit/
│   └── board.json
├── esp32s3/
│   └── board.json
└── esp32c3/
    └── board.json
```

## board.json Schema

```json
{
    "name": "Human-readable name",
    "architecture": "avr|xtensa|riscv",
    "cpu": "MCU model name",
    "clock_hz": 16000000,
    "flash_bytes": 32768,
    "ram_bytes": 2048,
    "eeprom_bytes": 1024,
    "psram_bytes": 0,
    "builtin_led_pin": 13,
    "has_uart": true,
    "has_spi": true,
    "has_i2c": true,
    "has_wifi": false,
    "has_ble": false,
    "upload_tool": "avrdude|esptool",
    "upload_baud": "115200",
    "compiler_path": "avr-gcc|xtensa-esp32-elf-gcc",
    "linker_script": "",
    "pins": {
        "digital": [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13],
        "analog": [0, 1, 2, 3, 4, 5],
        "pwm": [3, 5, 6, 9, 10, 11]
    },
    "uart": {
        "port0": { "tx": 1, "rx": 0, "cts": -1, "rts": -1 }
    },
    "spi": {
        "mosi": 11, "miso": 12, "sck": 13, "ss": 10
    },
    "i2c": {
        "sda": 4, "scl": 5
    },
    "adc": {
        "resolution_bits": 10,
        "channels": 6,
        "vref_mv": 5000
    }
}
```

## Adding a New Board

1. Create directory: `boards/<board-id>/`
2. Create `board.json` with board specifications
3. Run `engc doctor` to verify detection
4. Test with a simple blink example

## Board IDs

| Board ID | Display Name | Architecture |
|----------|-------------|--------------|
| arduino-uno | Arduino Uno | AVR |
| arduino-nano | Arduino Nano | AVR |
| arduino-mega | Arduino Mega | AVR |
| esp32-devkit | ESP32 DevKit | Xtensa |
| esp32s3 | ESP32-S3 | RISC-V |
| esp32c3 | ESP32-C3 | RISC-V |
