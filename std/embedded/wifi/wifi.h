// EnginotechC++ — Embedded WiFi HAL (ESP32 only)
// Portable WiFi abstraction for ESP32

#ifndef ENG_EMBEDDED_WIFI_H
#define ENG_EMBEDDED_WIFI_H

#include <cstdint>
#include <string>

namespace eng {
namespace embedded {
namespace wifi {

enum class Status { DISCONNECTED, CONNECTING, CONNECTED, FAILED };

struct WiFiConfig {
    std::string ssid;
    std::string password;
    bool connectOnce = true;
};

// Initialize and connect to WiFi
bool begin(const std::string& ssid, const std::string& password);

// Disconnect from WiFi
void end();

// Get current connection status
Status getStatus();

// Get IP address as string
std::string localIP();

// Check if connected
bool isConnected();

// Reconnect if disconnected
void reconnect();

// Get MAC address
std::string macAddress();

} // namespace wifi
} // namespace embedded
} // namespace eng

#endif // ENG_EMBEDDED_WIFI_H
