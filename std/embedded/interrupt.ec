// embedded.interrupt - Interrupt HAL

enum InterruptMode {
    RISING,
    FALLING,
    CHANGE,
    LOW,
    HIGH
}

fn interrupt_attach(pin: int, mode: InterruptMode, callback: fn() -> void) -> void { }
fn interrupt_detach(pin: int) -> void { }
fn interrupt_enable() -> void { }
fn interrupt_disable() -> void { }
fn in_interrupt_context() -> bool { return false; }