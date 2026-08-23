// EnginotechC++ — GUI Renderer (software rasterizer)
// Owns an RGBA pixel buffer and draws basic shapes/text into it.
// No external dependencies; blit to a Window via present().

#ifndef ENG_GUI_RENDERER_H
#define ENG_GUI_RENDERER_H

#include <cstdint>
#include <string>
#include <vector>

namespace eng {
namespace gui {

struct FontConfig;  // forward declaration

struct Color {
    uint8_t r = 0, g = 0, b = 0, a = 255;

    Color() = default;
    Color(uint8_t rr, uint8_t gg, uint8_t bb, uint8_t aa = 255)
        : r(rr), g(gg), b(bb), a(aa) {}

    static const Color White;
    static const Color Black;
    static const Color Gray;
    static const Color LightGray;
    static const Color DarkGray;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;

    bool operator==(const Color& o) const { return r == o.r && g == o.g && b == o.b && a == o.a; }
};

class Surface {
public:
    Surface() = default;
    Surface(int w, int h) { resize(w, h); }

    void resize(int w, int h);
    int width() const { return w_; }
    int height() const { return h_; }

    // raw pixel access (0xAARRGGBB)
    uint32_t& at(int x, int y) { return px_[y * w_ + x]; }
    const uint32_t& at(int x, int y) const { return px_[y * w_ + x]; }
    void setPixel(int x, int y, Color c);
    const uint32_t* data() const { return px_.data(); }   // for blitting

    // ---- primitives ----
    void clear(Color c);
    void fillRect(int x, int y, int w, int h, Color c);
    void drawRect(int x, int y, int w, int h, Color c);          // outline
    void drawLine(int x0, int y0, int x1, int y1, Color c);      // Bresenham
    void drawHLine(int x, int y, int w, Color c);
    void drawVLine(int x, int y, int h, Color c);
    void fillCircle(int cx, int cy, int radius, Color c);

    // ---- text ----
    // Draws a single ASCII char using the built-in 5x7 font.
    // Returns advance width (6 * scale px).
    int drawChar(int x, int y, char ch, Color c, int scale = 1);

    // Text drawing with full font configuration (bold, spacing, custom scale)
    int drawCharBold(int x, int y, char ch, Color c, int scale = 1);
    int drawTextConfigured(int x, int y, const std::string& s, Color c, const FontConfig& font);
    int drawTextConfiguredBold(int x, int y, const std::string& s, Color c, const FontConfig& font);

    int textWidth(const std::string& s, int scale = 1) const;   // 6 * length * scale - scale
    int textHeight(int scale = 1) const { return 8 * scale; }   // 7 rows + 1 spacing, scaled
    void drawText(int x, int y, const std::string& s, Color c, int scale = 1);

    // Gradient fills
    void fillGradientRect(int x, int y, int w, int h, Color top, Color bottom, bool vertical = true);
    void fillHGradientRect(int x, int y, int w, int h, Color left, Color right);

    // Rounded rectangle (filled)
    void fillRoundedRect(int x, int y, int w, int h, Color c, int radius = 8);

    // Debug/export: dump current frame as binary PPM (P6).
    bool savePPM(const std::string& path) const;

private:
    int w_ = 0, h_ = 0;
    std::vector<uint32_t> px_;
};

} // namespace gui
} // namespace eng

#endif // ENG_GUI_RENDERER_H
