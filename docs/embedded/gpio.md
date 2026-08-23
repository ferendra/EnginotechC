# EnginotechC++ — GPIO Documentation

## Overview

The GPIO module provides digital input/output control for all supported boards.

## Types

### OutputPin
```ec
let led = gpio.output(2);
```

Methods:
- `led.high()` — Set pin HIGH
- `led.low()` — Set pin LOW
- `led.toggle()` — Toggle pin state
- `led.read()` — Read current output state
- `led.write(state)` — Write specific state

### InputPin
```ec
let btn = gpio.input(4);
let btnPullup = gpio.input(4, gpio.Mode.INPUT_PULLUP);
```

Methods:
- `btn.read()` — Read pin value (`gpio.PinState.HIGH` or `gpio.PinState.LOW`)
- `btn.setMode(mode)` — Change pull-up/pull-down configuration

### Mode Enum
```ec
gpio.Mode.INPUT        // Standard input
gpio.Mode.OUTPUT       // Standard output
gpio.Mode.INPUT_PULLUP // Input with internal pull-up
gpio.Mode.INPUT_PULLDOWN // Input with internal pull-down
```

### PinState Enum
```ec
gpio.PinState.HIGH  // Logic high (3.3V or 5V depending on board)
gpio.PinState.LOW   // Logic low (GND)
```

## Examples

### Blink LED
```ec
import embedded.gpio;

fn main() {
    let led = gpio.output(13);
    loop {
        led.high();
        delay(500);
        led.low();
        delay(500);
    }
}
```

### Read Button
```ec
import embedded.gpio;

fn main() {
    let led = gpio.output(13);
    let btn = gpio.input(2, gpio.Mode.INPUT_PULLUP);

    loop {
        if btn.read() == gpio.PinState.LOW {
            led.high();
        } else {
            led.low();
        }
        delay(50);
    }
}
```

### Direct Pin Functions
```ec
import embedded.gpio;

fn main() {
    gpio.pinMode(13, gpio.Mode.OUTPUT);
    gpio.digitalWrite(13, gpio.PinState.HIGH);
    let val = gpio.digitalRead(2);
}
```

## Board-Specific Notes

### Arduino
- Operating voltage: 5V
- Built-in LED: pin 13
- Max current per pin: 20mA
- Absolute max: 40mA

### ESP32
- Operating voltage: 3.3V
- Built-in LED: pin 2 (most boards)
- Max current per pin: 40mA
- Some pins (0, 2, 4, 12-15) have boot-mode constraints
- ADC-capable pins: 32-39 (ESP32), 1-5 (ESP32-S3), 0-4 (ESP32-C3)
