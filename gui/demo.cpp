// EnginotechC++ — GUI Demo: Showcase Improved Visuals & Font Configuration
// Run: ./demo_gui
// Demonstrates all widgets with modern theming and configurable fonts.

#include "gui/widgets.h"
#include "gui/theme.h"
#include "gui/window.h"
#include <cstdio>

using namespace eng::gui;

// Helper: print button state
void logMsg(const std::string& s) {
    printf("%s\n", s.c_str());
}

int main() {
    // ── Create window (headless, no X11 needed) ──────────────────────
    Window win;
    if (!win.create("Enginotech GUI Demo - Modern Theme", 800, 600, Window::Backend::Headless)) {
        printf("Failed to create window\n");
        return 1;
    }
    printf("Window created: %dx%d (headless)\n", win.width(), win.height());

    // ── Configure theme with custom font settings ─────────────────────
    // This is where you can adjust fonts from script/code:
    auto darkTheme = Theme::active();
    darkTheme->c.font = {
        .family   = FontFamily::Pixel,
        .size     = 16,           // Larger font size
        .bold     = false,        // Regular weight
        .lineHeight = 22,         // Custom line height
        .letterSpacing = 1        // Slight letter spacing
    };

    // ── Build UI ──────────────────────────────────────────────────────
    VBox root;
    root.setRect({0, 0, 800, 600});
    root.spacing = 12;

    // Title Label (bold, larger)
    auto title = std::make_shared<Label>();
    title->setRect({40, 30, 720, 40});
    title->text = "Enginotech GUI Engine";
    title->bold = true;
    title->labelColor = Theme::active()->c.fg;
    root.add(title);

    // Subtitle Label
    auto subtitle = std::make_shared<Label>();
    subtitle->setRect({40, 75, 720, 25});
    subtitle->text = "Modern Theme with Configurable Fonts";
    subtitle->labelColor = Theme::active()->c.fgMuted;
    root.add(subtitle);

    // Divider
    auto divider = std::make_shared<Label>();
    divider->setRect({40, 105, 720, 2});
    divider->text = "";
    root.add(divider);

    // ── Button Samples ────────────────────────────────────────────────
    auto btnAccent = std::make_shared<Button>();
    btnAccent->setRect({40, 120, 160, 40});
    btnAccent->text = "Primary Action";
    btnAccent->onClick = []() { logMsg("Primary Action clicked!"); };
    root.add(btnAccent);

    auto btnSuccess = std::make_shared<Button>();
    btnSuccess->setRect({210, 120, 120, 40});
    btnSuccess->text = "Success";
    btnSuccess->bgColor = Theme::active()->c.success;
    btnSuccess->onClick = []() { logMsg("Success clicked!"); };
    root.add(btnSuccess);

    auto btnError = std::make_shared<Button>();
    btnError->setRect({340, 120, 120, 40});
    btnError->text = "Delete";
    btnError->bgColor = Theme::active()->c.error;
    btnError->onClick = []() { logMsg("Delete clicked!"); };
    root.add(btnError);

    auto btnDisabled = std::make_shared<Button>();
    btnDisabled->setRect({470, 120, 120, 40});
    btnDisabled->text = "Disabled";
    btnDisabled->setEnabled(false);
    root.add(btnDisabled);

    // ── TextBox Samples ───────────────────────────────────────────────
    auto txtNormal = std::make_shared<TextBox>();
    txtNormal->setRect({40, 180, 350, 36});
    txtNormal->placeholder = "Enter your name...";
    txtNormal->onText = [](const std::string& s) {
        printf("TextBox changed: %s\n", s.c_str());
    };
    root.add(txtNormal);

    auto txtPassword = std::make_shared<TextBox>();
    txtPassword->setRect({40, 225, 350, 36});
    txtPassword->placeholder = "Password";
    txtPassword->passwordMode = true;
    root.add(txtPassword);

    // ── Slider Sample ─────────────────────────────────────────────────
    auto sliderVol = std::make_shared<Slider>();
    sliderVol->setRect({40, 280, 400, 30});
    sliderVol->value = 0.7;
    sliderVol->min = 0.0;
    sliderVol->max = 1.0;
    sliderVol->onChange = []() {
        printf("Volume changed: %.2f\n", 0.7);
    };
    root.add(sliderVol);

    // ── Info Label ────────────────────────────────────────────────────
    auto info = std::make_shared<Label>();
    info->setRect({40, 330, 720, 60});
    info->text = "Font: Pixel | Size: 16 | Bold: Off\nSpacing: 1px | Theme: Dark";
    info->labelColor = Theme::active()->c.fgMuted;
    info->bold = false;
    root.add(info);

    // ── Render & present ──────────────────────────────────────────────
    Surface& surf = win.surface();
    const ThemeColors& tc = Theme::active()->c;

    // Clear with background color
    surf.clear(tc.bg);

    // Draw root container
    root.draw(surf);

    // Save frame for verification
    surf.savePPM("/tmp/gui_demo_ppm.ppm");
    printf("Frame saved to /tmp/gui_demo_ppm.ppm\n");
    printf("Preview with: display /tmp/gui_demo_ppm.ppm\n");

    // Run event loop (headless - just one iteration for demo)
    EventQueue eq;
    bool running = true;
    int frame = 0;
    while (running && frame < 10) {
        win.poll(eq);
        // Simulate hover on first button
        if (frame == 5) {
            btnAccent->onMouseMove(btnAccent->x() + btnAccent->width() / 2, 
                                   btnAccent->y() + btnAccent->height() / 2);
        } else {
            btnAccent->onMouseMove(-1, -1);
        }
        surf.clear(tc.bg);
        root.draw(surf);
        ++frame;
    }

    // Final save
    surf.savePPM("/tmp/gui_demo_final.ppm");
    printf("Final frame saved to /tmp/gui_demo_final.ppm\n");

    win.close();
    printf("Demo complete!\n");
    return 0;
}
