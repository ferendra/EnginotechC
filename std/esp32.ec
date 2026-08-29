// esp32 - ESP32-specific types and functions

import embedded.gpio;
import embedded.system;
import embedded.wifi;
import embedded.bluetooth;

fn esp_restart() -> void { }
fn esp_deep_sleep(us: int) -> void { }
fn esp_sleep_enable_timer_wakeup(us: int) -> void { }
fn esp_sleep_enable_ext0_wakeup(pin: int, level: int) -> void { }
fn esp_sleep_enable_ext1_wakeup(mask: int, level: int) -> void { }
fn esp_sleep_enable_touch_wakeup() -> void { }
fn esp_sleep_enable_ulp_wakeup() -> void { }
fn esp_get_free_heap_size() -> int { return 0; }
fn esp_get_chip_id() -> string { return ""; }
fn esp_get_cpu_freq_mhz() -> int { return 240; }

struct Esp32Pin {
    pin: int;
    mode: PinMode;
}