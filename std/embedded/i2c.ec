// embedded.i2c - I2C HAL

fn i2c_open(bus: int, address: int) -> I2cDevice { }
fn i2c_scan(bus: int) -> array<int> { return []; }

struct I2cDevice {
    bus: int;
    address: int;
}

impl I2cDevice {
    fn read(self, len: int) -> array<byte> { return []; }
    fn write(self, data: array<byte>) -> bool { return false; }
    fn write_read(self, write_data: array<byte>, read_len: int) -> array<byte> { return []; }
    fn read_reg(self, reg: int) -> byte { return 0; }
    fn write_reg(self, reg: int, value: byte) -> bool { return false; }
    fn read_reg16(self, reg: int) -> int { return 0; }
    fn write_reg16(self, reg: int, value: int) -> bool { return false; }
}