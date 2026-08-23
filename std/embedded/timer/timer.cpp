// EnginotechC++ — Embedded Timer Implementation

#include "timer.h"
#include <chrono>
#include <algorithm>

namespace eng {
namespace embedded {
namespace timer {

using Clock = std::chrono::steady_clock;
using Ms = std::chrono::milliseconds;

static std::vector<Timer> timers;
static int nextId = 0;

static uint64_t nowMs() {
    auto now = Clock::now();
    return std::chrono::duration_cast<Ms>(now.time_since_epoch()).count();
}

int every(uint64_t ms, TimerCallback cb) {
    Timer t{};
    t.id = nextId++;
    t.periodMs = ms;
    t.periodic = true;
    t.callback = std::move(cb);
    t.lastFired = nowMs();
    t.running = true;
    timers.push_back(t);
    return t.id;
}

int once(uint64_t ms, TimerCallback cb) {
    Timer t{};
    t.id = nextId++;
    t.periodMs = ms;
    t.periodic = false;
    t.callback = std::move(cb);
    t.lastFired = nowMs();
    t.running = true;
    timers.push_back(t);
    return t.id;
}

void stop(int id) {
    auto it = std::find_if(timers.begin(), timers.end(),
        [id](const Timer& t) { return t.id == id; });
    if (it != timers.end()) {
        it->running = false;
    }
}

bool isRunning(int id) {
    auto it = std::find_if(timers.begin(), timers.end(),
        [id](const Timer& t) { return t.id == id; });
    return it != timers.end() && it->running;
}

void tick() {
    uint64_t now = nowMs();
    for (auto& t : timers) {
        if (!t.running) continue;
        if (now - t.lastFired >= t.periodMs) {
            t.callback();
            t.lastFired = now;
            if (!t.periodic) {
                t.running = false;
            }
        }
    }
}

uint64_t remaining(int id) {
    auto it = std::find_if(timers.begin(), timers.end(),
        [id](const Timer& t) { return t.id == id; });
    if (it == timers.end() || !it->running) return 0;
    uint64_t now = nowMs();
    return (it->periodic || now < it->lastFired + it->periodMs)
        ? it->periodMs - (now - it->lastFired) : 0;
}

void clearAll() {
    timers.clear();
    nextId = 0;
}

} // namespace timer
} // namespace embedded
} // namespace eng
