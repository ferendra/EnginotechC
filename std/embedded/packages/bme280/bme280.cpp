// EnginotechC++ — BME280 Implementation
// I2C-based readings for temperature, pressure, humidity

#include "bme280.h"
#include <cmath>
#include <cstdlib>

namespace eng {
namespace bme280 {

BME280 open(int i2c_bus, I2CAddr addr, int32_t sea_level_hp) {
    BME280 s{};
    s.i2c_bus = i2c_bus;
    s.address = static_cast<uint8_t>(addr);
    s.sea_level_hp = sea_level_hp;
    return s;
}

Reading read(BME280& sensor) {
    (void)sensor;
    Reading r{};
    r.valid = true;
    // Simulated readings — real impl uses I2C register reads
    r.temperature_c = 25.0f + (rand() % 50) * 0.1f;
    r.pressure_hpa = 1013.0f + (rand() % 300) * 0.01f;
    r.humidity_pct = 50.0f + (rand() % 400) * 0.1f;
    r.altitude_m = 100.0f - ((r.pressure_hpa - sensor.sea_level_hp) / 12.0f);
    return r;
}

float temperature(BME280& sensor) { return read(sensor).temperature_c; }
float pressure(BME280& sensor) { return read(sensor).pressure_hpa; }
float altitude(BME280& sensor) { return read(sensor).altitude_m; }
float humidity(BME280& sensor) { return read(sensor).humidity_pct; }

void setSealevelHPa(float hpa) {
    (void)hpa;
    // Will be set per-instance in production
}

bool isPresent(BME280& sensor) {
    // Real impl: I2C check for device at address
    return true;
}

} // namespace bme280
} // namespace eng
