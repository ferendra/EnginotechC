// EnginotechC++ — Embedded Timer HAL
// Portable hardware/software timer abstraction

#ifndef ENG_EMBEDDED_TIMER_H
#define ENG_EMBEDDED_TIMER_H

#include <cstdint>
#include <functional>
#include <vector>

namespace eng {
namespace embedded {
namespace timer {

using TimerCallback = std::function<void()>;

struct Timer {
    int id;
    uint64_t periodMs;
    bool periodic;
    TimerCallback callback;
    uint64_t lastFired;
    bool running;
};

// Create a periodic timer that calls callback every `ms` milliseconds
int every(uint64_t ms, TimerCallback cb);

// Create a one-shot timer that calls callback after `ms` milliseconds
int once(uint64_t ms, TimerCallback cb);

// Stop and remove a timer
void stop(int id);

// Check if a timer is running
bool isRunning(int id);

// Run all active timers (call from main loop)
void tick();

// Get remaining ms until next fire for a timer
uint64_t remaining(int id);

// Clear all timers
void clearAll();

} // namespace timer
} // namespace embedded
} // namespace eng

#endif // ENG_EMBEDDED_TIMER_H
