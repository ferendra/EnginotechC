// embedded.spi - SPI HAL

enum SpiMode {
    MODE0,
    MODE1,
    MODE2,
    MODE3
}

struct SpiConfig {
    mode: SpiMode;
    frequency: int;
    bit_order: int;
}

fn spi_open(bus: int, config: SpiConfig) -> SpiDevice { }
fn spi_open_simple(bus: int) -> SpiDevice { }

struct SpiDevice {
    bus: int;
}

impl SpiDevice {
    fn transfer(self, data: array<byte>) -> array<byte> { return []; }
    fn write(self, data: array<byte>) -> void { }
    fn read(self, len: int) -> array<byte> { return []; }
    fn set_frequency(self, freq: int) -> void { }
    fn set_mode(self, mode: SpiMode) -> void { }
}