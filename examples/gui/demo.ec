// EnginotechC++ — GUI Demo: Interactive Widget Showcase
// Run with: engc run --target native examples/gui/demo.ec

import gui;

fn main() {
    let win = gui.Window.new("EnginotechC++ GUI Demo", 700, 500);
    if (!win.is_open) {
        print("Failed to open window\n");
        return;
    }

    // ── Colors ──
    let bg = gui.Color.new(30, 32, 36);
    let panel = gui.Color.new(45, 47, 50);
    let accent = gui.Color.new(66, 133, 244);
    let text = gui.Color.new(230, 230, 230);
    let dim = gui.Color.new(150, 150, 150);

    // ── Window surface ──
    let surf = win.surface();
    surf.clear(bg);

    // ── Title bar ──
    surf.fillGradientRect(0, 0, 700, 40,
        gui.Color.new(40, 42, 48),
        gui.Color.new(30, 32, 36));
    surf.drawText(20, 12, "EnginotechC++ — GUI Demo", text);
    surf.drawText(580, 12, "[ x ]", dim);

    // ── Left Panel: Buttons ──
    let px = 20, py = 60;
    surf.fillRoundedRect(px, py, 200, 200, panel, 8);
    surf.drawText(px + 12, py + 10, "Buttons", text);

    let btnY = py + 40;
    let btns = [
        gui.Button.new("Click Me!", accent),
        gui.Button.new("Submit", gui.Color.new(52, 168, 83)),
        gui.Button.new("Cancel", gui.Color.new(220, 50, 50)),
    ];

    for (let i = 0; i < 3; i++) {
        let btn = btns[i];
        btn.set_pos(px + 20, btnY + i * 50);
        btn.set_size(160, 40);
        btn.draw(surf);
    }

    // ── Center Panel: Slider ──
    let cx = 240, cy = 60;
    surf.fillRoundedRect(cx, cy, 420, 100, panel, 8);
    surf.drawText(cx + 12, cy + 10, "Slider Control", text);

    let slider = gui.Slider.new();
    slider.set_pos(cx + 20, cy + 45);
    slider.set_size(380, 30);
    slider.value = 0.6;
    slider.draw(surf);

    surf.drawText(cx + 20, cy + 80, "Value: 0.60", dim);

    // ── Right Panel: TextBox ──
    let rx = 240, ry = 180;
    surf.fillRoundedRect(rx, ry, 420, 120, panel, 8);
    surf.drawText(rx + 12, ry + 10, "Text Input", text);

    let tb = gui.TextBox.new();
    tb.placeholder = "Type something here...";
    tb.text = "Hello GUI!";
    tb.set_pos(rx + 12, ry + 35);
    tb.set_size(396, 30);
    tb.draw(surf);

    surf.drawText(rx + 12, ry + 75, "Press Enter to submit", dim);

    // ── Bottom: Info ──
    surf.fillRoundedRect(20, 380, 660, 100, panel, 8);
    surf.drawText(32, 395, "Widget Types:", text);
    surf.drawText(32, 415, "  • Button  — clickable action", dim);
    surf.drawText(32, 430, "  • Label   — text display", dim);
    surf.drawText(32, 445, "  • TextBox — single-line input", dim);

    win.present();

    // ── Main loop ──
    while win.is_open {
        let evs = win.poll_events();
        for ev in evs {
            match ev.type {
                gui.EventType.MouseMove => { /* handle hover */ }
                gui.EventType.MouseDown => {
                    // Check button clicks
                    for (let i = 0; i < 3; i++) {
                        let btn = btns[i];
                        if btn.is_hovered(ev.mouse_x, ev.mouse_y) {
                            // Button clicked!
                        }
                    }
                }
                gui.EventType.KeyDown => {
                    if ev.key == gui.SpecialKey.Escape {
                        win.close();
                    }
                }
                gui.EventType.WindowClose => {
                    win.close();
                }
                _ => {}
            }
        }

        // Redraw
        surf.clear(bg);
        // ... redraw all widgets ...
        win.present();
    }
}
