// EnginotechC++ — DHT11/DHT22 Sensor Package
// Temperature and humidity measurement from DHT sensors

#ifndef ENG_DHT_H
#define ENG_DHT_H

#include <cstdint>
#include <cmath>

namespace eng {
namespace dht {

enum class Type : uint8_t {
    DHT11 = 11,
    DHT22 = 22,
    DHT21 = 21,
    // AM2302 = alias for DHT21 (same chip)
};

struct Reading {
    float temperature_c;  // Celsius
    float humidity;       // Percentage
    bool valid;
};

// Connect DHT sensor to a GPIO pin
struct DHTSensor {
    int pin;
    Type type;
    uint32_t lastReadMs;
};

DHTSensor connect(int pin, Type type);
Reading read(DHTSensor& sensor);
float readTemperature(DHTSensor& sensor);
float readHumidity(DHTSensor& sensor);
float heatIndex(DHTSensor& sensor);  // Apparent temperature
float dewPoint(DHTSensor& sensor);   // Dew point in Celsius

} // namespace dht
} // namespace eng

#endif // ENG_DHT_H
