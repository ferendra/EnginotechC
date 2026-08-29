// embedded.timer - Timer HAL

enum TimerMode {
    ONESHOT,
    PERIODIC
}

struct TimerConfig {
    mode: TimerMode;
    interval_us: int;
    callback: fn() -> void;
}

fn timer_setup(timer_num: int, config: TimerConfig) -> Timer { }
fn timer_setup_simple(timer_num: int, interval_us: int, callback: fn() -> void) -> Timer { }

struct Timer {
    timer_num: int;
}

impl Timer {
    fn start(self) -> void { }
    fn stop(self) -> void { }
    fn set_interval(self, us: int) -> void { }
    fn attach_interrupt(self, callback: fn() -> void) -> void { }
}