// EnginotechC++ — SSD1306 OLED Display Implementation
#include "ssd1306.h"
#include <cstring>
#include <cstdio>

namespace eng {
namespace ssd1306 {

SSD1306 open(int i2c_bus, uint8_t addr, Size size) {
    SSD1306 d{};
    d.i2c_bus = i2c_bus;
    d.address = addr;
    d.width = 128;
    d.height = (size == Size::SIZE_128x32) ? 32 : 64;
    return d;
}

void clear(SSD1306&) {}
void display(SSD1306&) {}
void print(SSD1306&, int, int, const char*) {}
void println(SSD1306& d, int x, int y, const char* text) { print(d, x, y, text); }
void drawRect(SSD1306&, int, int, int, int) {}
void fillRect(SSD1306&, int, int, int, int) {}
void drawCircle(SSD1306&, int, int, int) {}
void invert(SSD1306&, bool) {}
void sleep(SSD1306&) {}
void wake(SSD1306&) {}
void setContrast(SSD1306&, uint8_t) {}
void scroll(SSD1306&, bool, int, int) {}

} // namespace ssd1306
} // namespace eng
