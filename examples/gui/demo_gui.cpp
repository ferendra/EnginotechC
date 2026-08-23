// EnginotechC++ — GUI Theme Demo
// Demonstrates theme switching: Dark, Light, Modern
// Build: g++ -std=c++20 demo_gui.cpp gui/*.cpp -o demo_gui -ldl -lX11

#include "../../gui/window.h"
#include "../../gui/widget.h"
#include "../../gui/button.h"
#include "../../gui/label.h"
#include "../../gui/textbox.h"
#include "../../gui/slider.h"
#include "../../gui/theme.h"
#include <iostream>
#include <memory>

using namespace eng::gui;

int main() {
    Window win;
    if (!win.create("EnginotechC++ — Theme Demo", 800, 580, Window::Backend::Auto)) {
        std::cerr << "Failed to create window\n";
        return 1;
    }

    // Register built-in themes
    Theme::registerTheme(std::make_shared<Theme>("Dark",   Theme::makeDark()));
    Theme::registerTheme(std::make_shared<Theme>("Light",  Theme::makeLight()));
    Theme::registerTheme(std::make_shared<Theme>("Modern", Theme::makeModern()));
    Theme::setActive(Theme::get("Dark"));

    bool quit = false;
    std::string currentTheme = "Dark";
    std::string inputText = "";
    double sliderVal = 0.5;
    int frameCount = 0;
    int mouseClickX = -1, mouseClickY = -1;
    bool mouseJustClicked = false;

    while (!quit && win.isOpen()) {
        EventQueue eq;
        win.poll(eq);

        Surface& surf = win.surface();
        const ThemeColors& tc = Theme::active()->c;

        surf.clear(tc.bg);
        frameCount++;

        // ── Title bar ──
        surf.fillGradientRect(0, 0, 800, 48, tc.surface, tc.bg);
        surf.drawText(20, 14, "EnginotechC++ — Theme Demo", tc.fg);
        surf.drawText(580, 14, "[ ESC to exit ]", tc.fgMuted);

        // ── Theme switcher buttons ──
        const char* themes[] = {"Dark", "Light", "Modern"};
        int btnW = 120, btnH = 36;
        for (int i = 0; i < 3; i++) {
            int bx = 20 + i * 132;
            int by = 58;
            bool active = (themes[i] == currentTheme);
            Color btnBg   = active ? tc.accent : tc.panelBg;
            Color btnFg   = active ? tc.fg : tc.fgMuted;
            Color btnBorder = active ? tc.accent : tc.border;

            surf.fillRoundedRect(bx, by, btnW, btnH, btnBg, 6);
            surf.drawRect(bx, by, btnW, btnH, btnBorder);
            surf.drawText(bx + (btnW - surf.textWidth(themes[i])) / 2,
                          by + (btnH - 8) / 2, themes[i], btnFg);

            // Handle click on theme buttons
            if (mouseJustClicked) {
                int mx = mouseClickX, my = mouseClickY;
                if (mx >= bx && mx < bx + btnW && my >= by && my < by + btnH) {
                    currentTheme = themes[i];
                    Theme::setActive(Theme::get(currentTheme));
                }
            }
        }

        // ── Left Panel: Buttons ──
        surf.fillRoundedRect(20, 110, 220, 240, tc.panelBg, 10);
        surf.drawText(32, 122, "Buttons", tc.fg);

        struct BtnDef { const char* label; Color bg; Color fg; };
        BtnDef btns[] = {
            {"Primary", tc.accent, tc.fg},
            {"Success", tc.success, tc.fg},
            {"Error",   tc.error, tc.fg},
            {"Warning", tc.warning, Color{30, 30, 30}},
        };
        int bBtnW = 180, bBtnH = 44;
        for (int i = 0; i < 4; i++) {
            int by = 142 + i * 52;
            surf.fillRoundedRect(30, by, bBtnW, bBtnH, btns[i].bg, 6);
            surf.drawText(30 + (bBtnW - surf.textWidth(btns[i].label)) / 2,
                          by + (bBtnH - 8) / 2, btns[i].label, btns[i].fg);
        }

        // ── Center Panel: Slider ──
        surf.fillRoundedRect(260, 110, 300, 120, tc.panelBg, 10);
        surf.drawText(272, 122, "Slider Control", tc.fg);

        int trackY = 155, trackH = 8;
        int trackX = 280, trackW = 260;
        surf.fillRect(trackX, trackY, trackW, trackH, tc.surface);
        int fillW = (int)(trackW * sliderVal);
        if (fillW > 0) surf.fillRect(trackX, trackY, fillW, trackH, tc.accent);
        surf.fillCircle(trackX + fillW, trackY + trackH / 2, 8, tc.fg);
        surf.drawText(280, 175, "Value: " + std::to_string((int)(sliderVal * 100)) + "%", tc.fgMuted);

        // ── Right Panel: TextBox ──
        surf.fillRoundedRect(580, 110, 200, 120, tc.panelBg, 10);
        surf.drawText(592, 122, "Text Input", tc.fg);
        surf.fillRoundedRect(592, 145, 176, 32, tc.surface, 4);
        surf.drawText(600, 152, inputText.empty() ? "Type here..." : inputText,
                      inputText.empty() ? tc.fgMuted : tc.fg);
        surf.drawText(592, 185, "Press Enter...", tc.fgMuted);

        // ── Bottom: Info Panel ──
        surf.fillRoundedRect(20, 370, 760, 190, tc.panelBg, 10);
        surf.drawText(32, 382, "Widget Gallery", tc.fg);

        surf.drawText(32, 410, "• Button   — click actions, hover/active states", tc.fgMuted);
        surf.drawText(32, 428, "• Label    — text display with color", tc.fgMuted);
        surf.drawText(32, 446, "• TextBox  — single-line text input with cursor", tc.fgMuted);
        surf.drawText(32, 464, "• Slider   — draggable value control", tc.fgMuted);
        surf.drawText(32, 482, "• Container — holds child widgets", tc.fgMuted);
        surf.drawText(32, 500, "• VBox/HBox — automatic layout managers", tc.fgMuted);

        surf.drawText(32, 528, "Click theme buttons above to switch • Press ESC to close", tc.fg);

        // ── Process events ──
        Event ev;
        while (eq.pop(ev)) {
            switch (ev.type) {
                case EventType::MouseWheel:
                    sliderVal += ev.wheelDelta * 0.05;
                    if (sliderVal > 1.0) sliderVal = 1.0;
                    if (sliderVal < 0.0) sliderVal = 0.0;
                    break;
                case EventType::MouseDown:
                    mouseClickX = ev.mouseX;
                    mouseClickY = ev.mouseY;
                    mouseJustClicked = true;
                    break;
                case EventType::MouseUp:
                    mouseJustClicked = false;
                    break;
                case EventType::KeyDown:
                    if (ev.key == SpecialKey::Escape) quit = true;
                    break;
                case EventType::WindowClose:
                    quit = true;
                    break;
                default:
                    break;
            }
        }

        win.present();
    }

    win.close();
    std::cout << "GUI theme demo closed. Final theme: " << currentTheme
              << ", Slider value: " << sliderVal << "\n";
    return 0;
}
