// EnginotechC++ — GUI Window
// Creates/closes/resizes an OS window and blits the software framebuffer.
//
// Backends:
//   - X11: loaded at runtime via dlopen("libX11.so.6") — no dev headers needed.
//   - Headless: no OS window; present() is a no-op (used for tests / CI).
// Auto-selects headless when X11 or DISPLAY is unavailable.

#ifndef ENG_GUI_WINDOW_H
#define ENG_GUI_WINDOW_H

#include <string>

#include "renderer.h"
#include "events.h"

namespace eng {
namespace gui {

class Window {
public:
    enum class Backend { Auto, X11, Headless };

    Window() = default;
    ~Window() { close(); }

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Returns false if the requested backend is unavailable.
    bool create(const std::string& title, int width, int height,
                Backend backend = Backend::Auto);
    void close();

    bool isOpen() const { return open_; }
    int width() const { return w_; }
    int height() const { return h_; }
    const std::string& backendName() const { return backendName_; }

    Surface& surface() { return surface_; }

    // Pumps OS events into `out`. Returns false once the window should close.
    bool poll(EventQueue& out);

    // Blits the current surface to the screen (no-op when headless).
    void present();

    bool shouldClose() const;

private:
    bool createX11(const std::string& title);
    void destroyX11();
    void recreateSurface(int w, int h);

    bool open_ = false;
    int w_ = 0, h_ = 0;
    std::string backendName_;
    Surface surface_;
    void* drv_ = nullptr;   /* CGuiDriver* (C driver, opaque here) */
};

} // namespace gui
} // namespace eng

#endif // ENG_GUI_WINDOW_H
