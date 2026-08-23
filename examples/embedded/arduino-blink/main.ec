// EnginotechC++ — Embedded Example: Arduino Blink
// A complete, buildable example for Arduino Uno

// EC source: examples/embedded/arduino-blink/main.ec
fn main() {
    let led = gpio.output(13);

    loop {
        led.high();
        delay(500);

        led.low();
        delay(500);
    }
}

// The above uses EC's loop{} construct which compiles to:
// while(1) { ... } in C/LLVM IR
