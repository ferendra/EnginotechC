// embedded.wifi - WiFi HAL (ESP32)

enum WifiMode {
    STA,
    AP,
    STA_AP
}

enum WifiAuth {
    OPEN,
    WEP,
    WPA_PSK,
    WPA2_PSK,
    WPA_WPA2_PSK
}

struct WifiConfig {
    ssid: string;
    password: string;
    mode: WifiMode;
    channel: int;
    auth: WifiAuth;
    hidden: bool;
    max_conn: int;
}

fn wifi_init(mode: WifiMode) -> bool { return false; }
fn wifi_connect(config: WifiConfig) -> bool { return false; }
fn wifi_connect_simple(ssid: string, password: string) -> bool { return false; }
fn wifi_disconnect() -> void { }
fn wifi_is_connected() -> bool { return false; }
fn wifi_get_ip() -> string { return ""; }
fn wifi_get_mac() -> string { return ""; }
fn wifi_scan() -> array<WifiAp> { return []; }
fn wifi_set_hostname(name: string) -> void { }
fn wifi_get_hostname() -> string { return ""; }

struct WifiAp {
    ssid: string;
    rssi: int;
    auth: WifiAuth;
    channel: int;
    mac: array<byte>;
    hidden: bool;
}

struct WifiConfigAP {
    ssid: string;
    password: string;
    channel: int;
    auth: WifiAuth;
    hidden: bool;
    max_conn: int;
}

fn wifi_start_ap(config: WifiConfigAP) -> bool { return false; }
fn wifi_stop_ap() -> void { }