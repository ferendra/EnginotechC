// embedded.uart - UART HAL

enum UartParity {
    NONE,
    EVEN,
    ODD
}

enum UartStopBits {
    ONE,
    TWO
}

struct UartConfig {
    baudrate: int;
    dataBits: int;
    parity: UartParity;
    stopBits: UartStopBits;
}

fn uart_open(uartNum: int, config: UartConfig) -> UartPort { }
fn uart_open_baud(uartNum: int, baudrate: int) -> UartPort { }

struct UartPort {
    uartNum: int;
}

impl UartPort {
    fn write(self, data: string) -> void { }
    fn write_bytes(self, data: array<byte>) -> void { }
    fn read(self, len: int) -> string { return ""; }
    fn read_bytes(self, len: int) -> array<byte> { return []; }
    fn available(self) -> int { return 0; }
    fn flush(self) -> void { }
    fn close(self) -> void { }
}