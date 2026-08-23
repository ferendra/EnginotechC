// EnginotechC++ — Embedded Bluetooth/BLE HAL (ESP32 only)
#ifndef ENG_EMBEDDED_BT_H
#define ENG_EMBEDDED_BT_H

#include <cstdint>
#include <string>

namespace eng {
namespace embedded {
namespace bt {

enum class Mode { DISABLED, BLE_SERVER, BLE_CLIENT };

// Initialize BLE
bool begin(Mode mode = Mode::BLE_SERVER);

// Set device name
void setName(const std::string& name);

// Start advertising
void advertise(bool persistent = true);

// Stop advertising
void stopAdvertise();

// Check if BLE is initialized
bool isInitialized();

} // namespace bt
} // namespace embedded
} // namespace eng

#endif // ENG_EMBEDDED_BT_H
