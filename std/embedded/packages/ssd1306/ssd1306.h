// EnginotechC++ — SSD1306 OLED Display Package
// Monochrome 128x64 / 128x32 OLED display driver

#ifndef ENG_SSD1306_H
#define ENG_SSD1306_H

#include <cstdint>
#include <string>

namespace eng {
namespace ssd1306 {

enum class Size : uint8_t {
    SIZE_128x64 = 0,
    SIZE_128x32 = 1
};

struct SSD1306 {
    int i2c_bus;
    uint8_t address;
    uint8_t width;
    uint8_t height;
    bool reversed;  // for some boards
};

SSD1306 open(int i2c_bus, uint8_t addr = 0x3C, Size size = Size::SIZE_128x64);
void clear(SSD1306& display);
void display(SSD1306& display);
void print(SSD1306& display, int x, int y, const char* text);
void println(SSD1306& display, int x, int y, const char* text);
void drawRect(SSD1306& display, int x, int y, int w, int h);
void fillRect(SSD1306& display, int x, int y, int w, int h);
void drawCircle(SSD1306& display, int x, int y, int r);
void invert(SSD1306& display, bool on);
void sleep(SSD1306& display);
void wake(SSD1306& display);
void setContrast(SSD1306& display, uint8_t contrast);
void scroll(SSD1306& display, bool horizontal, int start, int end);

} // namespace ssd1306
} // namespace eng

#endif // ENG_SSD1306_H
