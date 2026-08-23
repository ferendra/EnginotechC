// EnginotechC++ — RFID MFRC522 Package (SPI)

#ifndef ENG_RFID_H
#define ENG_RFID_H

#include <cstdint>
#include <vector>

namespace eng {
namespace rfid {

struct UID {
    uint8_t size;
    uint8_t bytes[10];
    uint32_t sak;
};

struct RFIDReader {
    int spi_bus;
    int pin_rst;   // Reset pin
    int pin_ss;    // SS/CS pin
    bool card_present;
};

RFIDReader open(int spi_bus, int pin_ss, int pin_rst = -1);
bool isCard(RFIDReader& reader);
bool selectCard(RFIDReader& reader);
UID readUID(RFIDReader& reader);
bool authenticate(RFIDReader& reader, uint8_t block, uint8_t key_type, uint8_t* key);
std::vector<uint8_t> readBlock(RFIDReader& reader, uint8_t block);
bool writeBlock(RFIDReader& reader, uint8_t block, const std::vector<uint8_t>& data);

} // namespace rfid
} // namespace eng

#endif // ENG_RFID_H
