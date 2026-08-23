// EnginotechC++ — Embedded SPI HAL
// Portable SPI abstraction for Arduino and ESP32

#ifndef ENG_EMBEDDED_SPI_H
#define ENG_EMBEDDED_SPI_H

#include <cstdint>
#include <cstddef>

namespace eng {
namespace embedded {
namespace spi {

enum class Mode { MODE0, MODE1, MODE2, MODE3 };
enum class BitOrder { MSBFIRST, LSBFIRST };

struct SPIBus {
    int bus;
    uint32_t frequencyHz;
    Mode mode;
    BitOrder bitOrder;
    int csPin;

    bool begin();
    void end();
    void setFrequency(uint32_t hz);
    void setMode(Mode m);
    void setBitOrder(BitOrder order);
    void beginTransaction();
    void endTransaction();
    uint8_t transfer(uint8_t data);
    void transferBlock(const uint8_t* out, uint8_t* in, size_t len);
};

// Open an SPI bus (0 = primary MOSI/MISO/SCK, 1 = secondary)
SPIBus open(int bus = 0, uint32_t frequencyHz = 1000000);

} // namespace spi
} // namespace embedded
} // namespace eng

#endif // ENG_EMBEDDED_SPI_H
