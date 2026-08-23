// EnginotechC++ — Embedded Bluetooth Implementation

#include "bt.h"

namespace eng {
namespace embedded {
namespace bt {

bool begin(Mode) { return false; }
void setName(const std::string&) {}
void advertise(bool) {}
void stopAdvertise() {}
bool isInitialized() { return false; }

} // namespace bt
} // namespace embedded
} // namespace eng
