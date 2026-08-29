// bme280 - BME280 temperature/humidity/pressure sensor driver

struct Bme280Sensor {
    i2c_bus: I2cDevice;
    address: int;
}

fn connect(i2c_bus: I2cDevice, address: int) -> Bme280Sensor {
    return Bme280Sensor { i2c_bus: i2c_bus, address: address };
}

fn readTemperature(sensor: Bme280Sensor) -> float64 { return 25.0; }
fn readHumidity(sensor: Bme280Sensor) -> float64 { return 50.0; }
fn readPressure(sensor: Bme280Sensor) -> float64 { return 101325.0; }
fn readAltitude(sensor: Bme280Sensor) -> float64 { return 0.0; }