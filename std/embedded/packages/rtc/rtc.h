// EnginotechC++ — RTC Package (DS1307/DS3231)

#ifndef ENG_RTC_H
#define ENG_RTC_H

#include <cstdint>

namespace eng {
namespace rtc {

struct DateTime {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day_of_week;
};

struct RTCModule {
    int i2c_bus;
    uint8_t address;
};

RTCModule open(int i2c_bus, uint8_t addr = 0x68);
DateTime now(RTCModule& rtc);
void adjust(RTCModule& rtc, DateTime dt);
bool lostPower(RTCModule& rtc);
float getTemperature(RTCModule& rtc);  // DS3231 only

} // namespace rtc
} // namespace eng

#endif // ENG_RTC_H
