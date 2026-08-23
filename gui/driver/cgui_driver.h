/*
 * EnginotechC++ — CGUI Driver (pure C, FFI-ready)
 *
 * Thin system-call level wrapper around X11, loaded at RUNTIME via
 * dlopen("libX11.so.6") so no X11 dev headers/libs are needed to BUILD.
 * Any language (C++, EC via FFI, Python via ctypes, ...) can bind to this.
 *
 * Build as C99 or later. Link only against -ldl (glibc >= 2.34 needs none).
 */
#ifndef CGUI_DRIVER_H
#define CGUI_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* ---- event codes (backend-neutral) ---- */
enum {
    CGUI_EV_NONE = 0,
    CGUI_EV_MOUSE_MOVE,
    CGUI_EV_MOUSE_DOWN,
    CGUI_EV_MOUSE_UP,
    CGUI_EV_MOUSE_WHEEL,
    CGUI_EV_KEY_DOWN,
    CGUI_EV_KEY_UP,
    CGUI_EV_WINDOW_RESIZE,
    CGUI_EV_CLOSE          /* WM_DELETE_WINDOW received */
};

typedef struct CGuiEvent {
    int type;
    int x, y;              /* mouse position */
    int width, height;     /* new size (WINDOW_RESIZE) */
    int button;            /* 1 left, 2 middle, 3 right; wheel: +1 up/-1 down */
    unsigned long key;     /* raw keysym */
} CGuiEvent;

/* Opaque handle. */
typedef struct CGuiDriver CGuiDriver;

/* Returns 1 when an X11 display connection is possible right now. */
int cgui_supported(void);

/* Opens a window; NULL on failure. */
CGuiDriver* cgui_open(const char* title, int width, int height);

void       cgui_close(CGuiDriver* drv);

/* Pumps one OS event; fills ev and returns its type (CGUI_EV_NONE if none). */
int        cgui_poll(CGuiDriver* drv, CGuiEvent* ev);

/* Non-zero once the user asked to close the window. */
int        cgui_should_close(CGuiDriver* drv);

/* Uploads a BGRA (little-endian XRGB) pixel buffer of w*h pixels. */
void       cgui_blit(CGuiDriver* drv, const unsigned char* bgra,
                     int w, int h);

/* Current client-area size (changes on WINDOW_RESIZE events). */
void       cgui_query_size(CGuiDriver* drv, int* w, int* h);

const char* cgui_backend_name(void);

#ifdef __cplusplus
}
#endif

#endif /* CGUI_DRIVER_H */
