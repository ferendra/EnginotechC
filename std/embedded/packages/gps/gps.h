// EnginotechC++ — GPS Package (NMEA over UART)

#ifndef ENG_GPS_H
#define ENG_GPS_H

#include <cstdint>

namespace eng {
namespace gps {

enum class FixType : uint8_t {
    NO_FIX = 0,
    FIX_2D = 2,
    FIX_3D = 3
};

struct GPSFix {
    float latitude;          // Degrees
    float longitude;         // Degrees
    float altitude;          // Meters
    float speed_knots;       // Knots
    float speed_kmh;         // km/h
    float course_deg;        // Course over ground
    uint8_t satellites;
    uint8_t hour, minute, second;
    uint8_t day, month;
    uint16_t year;
    FixType fix_type;
};

struct GPS {
    int uart_num;
    int rx_pin;
    int tx_pin;
    uint32_t baud;
};

GPS open(int uart_num, int tx_pin, int rx_pin, uint32_t baud = 9600);
void close(GPS& g);
void update(GPS& g);
GPSFix getFix(GPS& g);
bool hasFix(GPS& g);

// Convenient methods
double latitude(GPS& g);
double longitude(GPS& g);
double altitude(GPS& g);

} // namespace gps
} // namespace eng

#endif // ENG_GPS_H
