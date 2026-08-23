// EnginotechC++ — RFID Implementation
#include "rfid.h"
#include <cstdio>
#include <cstring>

namespace eng {
namespace rfid {

RFIDReader open(int spi_bus, int pin_ss, int pin_rst) {
    RFIDReader r{};
    r.spi_bus = spi_bus;
    r.pin_ss = pin_ss;
    r.pin_rst = pin_rst;
    r.card_present = false;
    return r;
}

bool isCard(RFIDReader& reader) {
    (void)reader;
    return false;
}

bool selectCard(RFIDReader& reader) {
    (void)reader;
    return false;
}

UID readUID(RFIDReader& reader) {
    (void)reader;
    UID u{};
    u.size = 4;
    u.bytes[0] = 0xDE;
    u.bytes[1] = 0xAD;
    u.bytes[2] = 0xBE;
    u.bytes[3] = 0xEF;
    u.sak = 0;
    return u;
}

bool authenticate(RFIDReader&, uint8_t, uint8_t, uint8_t*) { return false; }
std::vector<uint8_t> readBlock(RFIDReader&, uint8_t) { return {}; }
bool writeBlock(RFIDReader&, uint8_t, const std::vector<uint8_t>&) { return false; }

} // namespace rfid
} // namespace eng
