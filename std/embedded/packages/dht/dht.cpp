// EnginotechC++ — DHT Sensor Implementation
// Uses single-wire protocol to read DHT11/DHT22

#include "dht.h"
#include <chrono>
#include <thread>
#include <cstdio>

namespace eng {
namespace dht {
using namespace std::chrono;

static const uint32_t READ_INTERVAL_MS = 2000;  // Min 2s between reads

DHTSensor connect(int pin, Type type) {
    DHTSensor s{};
    s.pin = pin;
    s.type = type;
    s.lastReadMs = 0;
    return s;
}

// Simulated reading for host/testing — real implementation uses bit-banging
Reading read(DHTSensor& sensor) {
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    if (now_ms - sensor.lastReadMs < READ_INTERVAL_MS) {
        // Return last cached value
        static Reading cached;
        return cached;
    }
    sensor.lastReadMs = static_cast<uint32_t>(now_ms);

    Reading r{};
    r.valid = true;

    // Simulated values for testing
    // Real impl: bit-bang single-wire protocol
    switch (sensor.type) {
        case Type::DHT11:
            r.temperature_c = 25.0f + (rand() % 10) * 0.1f;
            r.humidity = 55.0f + (rand() % 20) * 0.1f;
            break;
        case Type::DHT22:
        case Type::DHT21:
            r.temperature_c = 25.0f + (rand() % 50) * 0.1f;
            r.humidity = 50.0f + (rand() % 30) * 0.1f;
            break;
    }
    return r;
}

float readTemperature(DHTSensor& sensor) {
    return read(sensor).temperature_c;
}

float readHumidity(DHTSensor& sensor) {
    return read(sensor).humidity;
}

// Heat index (apparent temperature) in Celsius
float heatIndex(DHTSensor& sensor) {
    float t = readTemperature(sensor);
    float h = readHumidity(sensor);
    // Simplified heat index formula (for T >= 27°C and RH >= 40%)
    if (t < 27.0f || h < 40.0f) return t;
    float hi = -8.784695f
               + 1.61139411f * t
               + 2.338549f * h
               - 0.14611605f * t * h
               - 0.012308094f * t * t
               - 0.016424828f * h * h
               + 0.002211732f * t * t * h
               + 0.00072546f * t * h * h
               - 0.000003582f * t * t * h * h;
    return hi;
}

// Dew point in Celsius
float dewPoint(DHTSensor& sensor) {
    float t = readTemperature(sensor);
    float h = readHumidity(sensor);
    // Magnus formula
    const float a = 17.27f, b = 237.7f;
    float alpha = (a * t) / (b + t) + logf(h / 100.0f);
    return (b * alpha) / (a - alpha);
}

} // namespace dht
} // namespace eng
