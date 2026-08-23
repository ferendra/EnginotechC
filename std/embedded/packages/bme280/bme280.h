// EnginotechC++ — BME280 Sensor Package
// Barometric pressure, temperature, humidity (I2C/SPI)

#ifndef ENG_BME280_H
#define ENG_BME280_H

#include <cstdint>

namespace eng {
namespace bme280 {

enum class I2CAddr : uint8_t {
    ADDR_GND = 0x76,  // SDO pin connected to GND
    ADDR_VCC = 0x77   // SDO pin connected to VCC
};

struct Reading {
    float temperature_c;
    float pressure_hpa;   // Hectopascals
    float altitude_m;     // Meters (sea level reference)
    float humidity_pct;
    bool valid;
};

struct BME280 {
    int i2c_bus;          // I2C bus number
    uint8_t address;      // I2C address
    int32_t sea_level_hp; // Sea level pressure reference
};

BME280 open(int i2c_bus, I2CAddr addr = I2CAddr::ADDR_GND, int32_t sea_level_hp = 101325);
Reading read(BME280& sensor);
float temperature(BME280& sensor);
float pressure(BME280& sensor);
float altitude(BME280& sensor);
float humidity(BME280& sensor);
void setSealevelHPa(float hpa);
bool isPresent(BME280& sensor);

} // namespace bme280
} // namespace eng

#endif // ENG_BME280_H
