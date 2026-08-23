// EnginotechC++ — GUI Window Implementation (Headless)
// Software framebuffer - no OS window needed for testing/rendering.

#include "window.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace eng {
namespace gui {

bool Window::create(const std::string& title, int width, int height, Backend backend) {
    (void)title;
    (void)backend;
    open_ = true;
    w_ = width; h_ = height;
    backendName_ = "Headless";
    surface_.resize(width, height);
    return true;
}

void Window::close() {
    open_ = false;
}

bool Window::poll(EventQueue& out) {
    (void)out;
    return open_;
}

void Window::present() {
    // No-op in headless mode — no file output to avoid cluttering the directory.
}

bool Window::shouldClose() const {
    return !open_;
}

} // namespace gui
} // namespace eng
