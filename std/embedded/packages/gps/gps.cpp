// EnginotechC++ — GPS Implementation
#include "gps.h"

namespace eng {
namespace gps {

GPS open(int uart_num, int tx_pin, int rx_pin, uint32_t baud) {
    GPS g{};
    g.uart_num = uart_num;
    g.tx_pin = tx_pin;
    g.rx_pin = rx_pin;
    g.baud = baud;
    return g;
}

void close(GPS&) {}

void update(GPS&) {
    // Real impl: parse NMEA $GPRMC/$GPGGA sentences
}

GPSFix getFix(GPS&) {
    GPSFix f{};
    f.latitude = -6.21462;
    f.longitude = 106.84513;
    f.altitude = 10.0;
    f.speed_knots = 0;
    f.speed_kmh = 0;
    f.course_deg = 0;
    f.satellites = 8;
    f.hour = 12;
    f.minute = 0;
    f.second = 0;
    f.day = 20;
    f.month = 8;
    f.year = 2026;
    f.fix_type = FixType::FIX_3D;
    return f;
}

bool hasFix(GPS&) { return true; }
double latitude(GPS& g) { return getFix(g).latitude; }
double longitude(GPS& g) { return getFix(g).longitude; }
double altitude(GPS& g) { return getFix(g).altitude; }

} // namespace gps
} // namespace eng
