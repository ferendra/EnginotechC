// EnginotechC++ — WS2812 NeoPixel Package
// Addressable RGB LED strip driver

#ifndef ENG_WS2812_H
#define ENG_WS2812_H

#include <cstdint>
#include <vector>

namespace eng {
namespace ws2812 {

struct Pixel {
    uint8_t r, g, b;
};

struct WS2812 {
    int pin;
    int num_leds;
    std::vector<Pixel> pixels;
};

WS2812 open(int pin, int num_leds);
void setPixel(WS2812& leds, int idx, uint8_t r, uint8_t g, uint8_t b);
void clear(WS2812& leds);
void show(WS2812& leds);
void fill(WS2812& leds, uint8_t r, uint8_t g, uint8_t b);

// Predefined colors
void rainbow(WS2812& leds, int offset);
void fadeAll(WS2812& leds, int amount);

} // namespace ws2812
} // namespace eng

#endif // ENG_WS2812_H
