# EnginotechC++ GUI Improvements

## Summary of Changes

This update improves the GUI to have a modern, polished look with configurable fonts that can be adjusted from script/code.

---

## New Features

### 1. Font Configuration System
- **FontConfig struct** added to `ThemeColors`
- Configurable properties:
  - `family`: Pixel, Mono, or Sans (currently Pixel/Bitmap)
  - `size`: Base pixel size per character (default: 14)
  - `bold`: Enable bold text rendering
  - `lineHeight`: Custom line height (0 = auto)
  - `letterSpacing`: Extra pixels between characters (default: 0)

### 2. Improved Visual Design

#### Button Widget
- Added **drop shadow** for depth
- **Highlight border** on top edge for 3D effect
- **Bold text support** via font config
- Better hover/focus states with accent colors

#### TextBox Widget
- **Focus ring**: Accent-colored border when focused
- **Shadow** effect for depth
- **Inner highlight** line for subtle 3D look
- **Better padding** and font-aware sizing
- Placeholder text in muted color

#### Slider Widget
- **Rounded track** with subtle border
- **Larger handle** with shadow and highlight
- **Gradient effect** on fill
- More polished visual appearance

#### Label Widget
- **Font-size aware** text rendering
- **Vertical centering** based on font metrics
- **Bold support** via `fontBold` flag

### 3. Renderer Enhancements
- **Bold font variant**: 5x7 bitmap with thicker strokes
- **Configurable text drawing** with `drawTextConfigured()` and `drawTextConfiguredBold()`
- **Letter spacing** support
- **Scale-aware** text height calculation

---

## How to Configure Fonts

### From C++ Code

```cpp
// Get the active theme and modify font settings
auto theme = eng::gui::Theme::active();
theme->c.font = {
    .family       = eng::gui::FontFamily::Pixel,
    .size         = 18,           // Larger text
    .bold         = false,        // Regular weight
    .lineHeight   = 24,           // Custom line height
    .letterSpacing = 1            // Slight spacing between chars
};
```

### For Specific Widgets

```cpp
// Make a specific widget bold
auto btn = std::make_shared<Button>();
btn->fontBold = true;  // Override theme bold setting

// Change individual widget font size
auto label = std::make_shared<Label>();
label->fontBold = false;
```

---

## Theme Options

Three built-in themes available:
- `Theme::dark()` - Dark theme (default)
- `Theme::light()` - Light theme
- `Theme::modern()` - Modern cyan-accent theme

---

## Demo

Run the demo to see all improvements:
```bash
cd /home/ferendra-putra/Downloads/EngineSoft/enginotech-cpp
./demo_gui
```

Output files:
- `/tmp/gui_demo_ppm.ppm` - Initial frame
- `/tmp/gui_demo_final.ppm` - Final frame with all widgets

View with:
```bash
display /tmp/gui_demo_final.ppm
```

---

## Files Modified

1. **theme.h** - Added FontConfig struct and enum
2. **theme.cpp** - Updated theme factories with default fonts
3. **renderer.h** - Added bold font methods and scaled text API
4. **renderer.cpp** - Implemented bold bitmap font and configured text drawing
5. **widget.h** - Added fontBold member
6. **widget.cpp** - Updated drawTextCentered for font config
7. **button.cpp** - Added shadow, highlight, and font-aware text
8. **label.cpp** - Updated for font scaling and centering
9. **textbox.cpp** - Added focus ring, shadow, and font-aware sizing
10. **slider.cpp** - Improved track and handle visuals

---

## Technical Notes

- The embedded bitmap font is 5x7 pixels per character
- Scale factor is calculated as: `font.size / 7 + 1`
- Bold variant uses thicker bitmaps for the same glyphs
- All text rendering respects the current theme's FontConfig
- Backward compatible: existing code works without changes

---

## Example: Custom Font Script

```cpp
#include "gui/theme.h"
#include "gui/widgets.h"
#include "gui/window.h"

using namespace eng::gui;

int main() {
    // Create window
    Window win;
    win.create("Custom Fonts", 800, 600);
    
    // Configure fonts
    Theme::active()->c.font = {
        .family   = FontFamily::Pixel,
        .size     = 20,      // Larger default size
        .bold     = false,
        .letterSpacing = 2
    };
    
    // Create widgets
    auto title = std::make_shared<Label>();
    title->text = "My Application";
    title->fontBold = true;
    
    auto subtitle = std::make_shared<Label>();
    subtitle->text = "With custom fonts";
    subtitle->labelColor = Color{150, 150, 160};  // Muted color
    
    // Run event loop...
    return 0;
}
```

---

## Generated Demo Output

- Image: `/tmp/gui_demo_final.ppm` (800x600)
- Format: Netpbm PPM (rawbits)
- Shows: Title, buttons, textboxes, slider, and info text
- All with modern styling and font configuration
