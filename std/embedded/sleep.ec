// embedded.sleep - Sleep/Power Management HAL (ESP32)

enum WakeupSource {
    EXT0,
    EXT1,
    TIMER,
    TOUCH,
    ULP
}

fn sleep_enable_timer_wakeup(us: int) -> void { }
fn sleep_disable_timer_wakeup() -> void { }

fn sleep_enable_ext0_wakeup(pin: int, level: int) -> void { }
fn sleep_disable_ext0_wakeup() -> void { }

fn sleep_enable_ext1_wakeup(mask: int, level: int) -> void { }
fn sleep_disable_ext1_wakeup() -> void { }

fn sleep_enable_touch_wakeup() -> void { }
fn sleep_disable_touch_wakeup() -> void { }

fn sleep_enable_ulp_wakeup() -> void { }
fn sleep_disable_ulp_wakeup() -> void { }

fn sleep_start(us: int) -> void { }
fn sleep_start_simple() -> void { }
fn sleep_get_wakeup_cause() -> WakeupSource { return WakeupSource.TIMER; }