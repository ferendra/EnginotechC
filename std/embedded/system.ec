// embedded.system - System functions

fn millis() -> int { return 0; }
fn micros() -> int { return 0; }
fn delay(ms: int) -> void { }
fn delay_microseconds(us: int) -> void { }

fn reboot() -> void { }
fn deep_sleep(us: int) -> void { }

fn get_free_heap() -> int { return 0; }
fn get_chip_id() -> string { return ""; }
fn get_cpu_frequency() -> int { return 0; }

struct PlatformInfo {
    name: string;
    is_windows: bool;
    is_posix: bool;
    is_embedded: bool;
    is_arduino: bool;
    is_esp32: bool;
}

fn platform() -> PlatformInfo { return PlatformInfo { name: "", is_windows: false, is_posix: false, is_embedded: true, is_arduino: false, is_esp32: false }; }