// embedded.gpio - GPIO HAL

enum PinMode {
    INPUT,
    OUTPUT,
    INPUT_PULLUP,
    INPUT_PULLDOWN,
    ANALOG
}

enum PinState {
    LOW,
    HIGH
}

fn gpio_output(pin: int) -> GpioPin { }
fn gpio_input(pin: int) -> GpioPin { }
fn gpio_analog(pin: int) -> GpioPin { }

struct GpioPin {
    pin: int;
    mode: PinMode;
}

impl GpioPin {
    fn write(self, state: PinState) -> void { }
    fn read(self) -> PinState { return PinState.LOW; }
    fn toggle(self) -> void { }
    fn mode(self, m: PinMode) -> void { }
}