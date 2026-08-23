// EnginotechC++ — WS2812 NeoPixel Implementation
#include "ws2812.h"
#include <cmath>
#include <cstdio>

namespace eng {
namespace ws2812 {

WS2812 open(int pin, int num_leds) {
    WS2812 l{};
    l.pin = pin;
    l.num_leds = num_leds;
    l.pixels.assign(num_leds, {0, 0, 0});
    return l;
}

void setPixel(WS2812& leds, int idx, uint8_t r, uint8_t g, uint8_t b) {
    if (idx < 0 || idx >= leds.num_leds) return;
    leds.pixels[idx] = {r, g, b};
}

void clear(WS2812& leds) {
    for (auto& p : leds.pixels) { p.r = p.g = p.b = 0; }
}

void show(WS2812& leds) {
    // In production: bit-bang GPIO with WS2812 timing
    printf("[WS2812] Updated %d LEDs on pin %d\n", leds.num_leds, leds.pin);
}

void fill(WS2812& leds, uint8_t r, uint8_t g, uint8_t b) {
    for (auto& p : leds.pixels) { p.r = r; p.g = g; p.b = b; }
    show(leds);
}

void rainbow(WS2812& leds, int offset) {
    for (int i = 0; i < leds.num_leds; ++i) {
        int hue = (i * 360 / leds.num_leds + offset) % 360;
        float s = hue / 60.0f;
        float v = 1.0f;
        float h = s - (int)s;
        uint8_t r = v * 255, g = v * 255, b = v * 255;
        // Simplified HSV -> RGB
        int hi = (int)s % 6;
        float f = s - (int)s;
        float p = v * (1.0f - s);
        float q = v * (1.0f - f * s);
        float t = v * (1.0f - (1.0f - f) * s);
        switch(hi) {
            case 0: r=v*255; g=t*255; b=p*255; break;
            case 1: r=q*255; g=v*255; b=p*255; break;
            case 2: r=p*255; g=v*255; b=t*255; break;
            case 3: r=p*255; g=q*255; b=v*255; break;
            case 4: r=t*255; g=p*255; b=v*255; break;
            case 5: r=v*255; g=p*255; b=q*255; break;
        }
        setPixel(leds, i, (uint8_t)r, (uint8_t)g, (uint8_t)b);
    }
    show(leds);
}

void fadeAll(WS2812& leds, int amount) {
    for (auto& p : leds.pixels) {
        p.r = (p.r > amount) ? p.r - amount : 0;
        p.g = (p.g > amount) ? p.g - amount : 0;
        p.b = (p.b > amount) ? p.b - amount : 0;
    }
    show(leds);
}

} // namespace ws2812
} // namespace eng
