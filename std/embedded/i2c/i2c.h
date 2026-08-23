// EnginotechC++ — Embedded I2C HAL
// Portable I2C (TWI) abstraction for Arduino and ESP32

#ifndef ENG_EMBEDDED_I2C_H
#define ENG_EMBEDDED_I2C_H

#include <cstdint>
#include <cstddef>

namespace eng {
namespace embedded {
namespace i2c {

struct I2CBus {
    int bus;
    uint32_t frequencyHz;

    bool begin();
    bool end();
    bool beginTransmission(uint8_t address);
    size_t write(const uint8_t* data, size_t len);
    size_t write(uint8_t data);
    bool requestFrom(uint8_t address, size_t len);
    int read();
    bool stop();
    bool transmit();

    uint32_t getFrequency() const;
    void setFrequency(uint32_t hz);
};

struct I2CDevice {
    I2CBus* bus;
    uint8_t address;

    size_t write(const uint8_t* data, size_t len);
    size_t write(uint8_t reg, const uint8_t* data, size_t len);
    bool read(uint8_t* buf, size_t len);
    bool readReg(uint8_t reg, uint8_t* buf, size_t len);
};

// Open an I2C bus (0 = primary SDA/SCL, 1 = secondary)
I2CBus open(int bus = 0, uint32_t frequencyHz = 100000);

// Create a device handle on an existing bus
I2CDevice device(I2CBus& bus, uint8_t address);

} // namespace i2c
} // namespace embedded
} // namespace eng

#endif // ENG_EMBEDDED_I2C_H
