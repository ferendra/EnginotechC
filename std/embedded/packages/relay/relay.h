// EnginotechC++ — Relay Package
// Digital on/off control for relay modules

#ifndef ENG_RELAY_H
#define ENG_RELAY_H

namespace eng {
namespace relay {

struct Relay {
    int pin;
    bool state;
    bool active_high;  // true = HIGH triggers relay, false = LOW triggers
};

Relay connect(int pin, bool active_high = true);
void on(Relay& r);
void off(Relay& r);
void toggle(Relay& r);
bool isOn(Relay& r);
bool isOff(Relay& r);

} // namespace relay
} // namespace eng

#endif // ENG_RELAY_H
