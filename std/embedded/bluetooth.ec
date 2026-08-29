// embedded.bluetooth - Bluetooth HAL (ESP32)

enum BleEvent {
    CONNECTED,
    DISCONNECTED,
    DATA_RECEIVED
}

fn bt_init(device_name: string) -> bool { return false; }
fn bt_deinit() -> void { }
fn bt_start_advertising() -> void { }
fn bt_stop_advertising() -> void { }
fn bt_is_connected() -> bool { return false; }
fn bt_set_callback(event: BleEvent, callback: fn(data: string) -> void) -> void { }
fn bt_send(data: string) -> bool { return false; }
fn bt_send_bytes(data: array<byte>) -> bool { return false; }