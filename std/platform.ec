// platform - platform detection

struct PlatformInfo {
    name: string;
    is_windows: bool;
    is_posix: bool;
    is_embedded: bool;
    is_arduino: bool;
    is_esp32: bool;
}

fn is_windows() -> bool { return false; }
fn is_posix() -> bool { return false; }
fn is_embedded() -> bool { return true; }
fn is_arduino() -> bool { return false; }
fn is_esp32() -> bool { return false; }
fn platform_info() -> PlatformInfo { 
    return PlatformInfo { 
        name: "embedded", 
        is_windows: false, 
        is_posix: false, 
        is_embedded: true, 
        is_arduino: false, 
        is_esp32: false 
    }; 
}