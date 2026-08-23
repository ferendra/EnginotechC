/*
 * EnginotechC++ — CGUI Driver implementation (pure C99)
 *
 * Wraps raw X11 system-level calls. libX11 is loaded at RUNTIME via
 * dlopen so this builds without any X11 dev package. All X11 struct
 * layouts below match the x86-64 SysV ABI used by every Linux distro.
 */
#include "cgui_driver.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Minimal X11 ABI definitions (x86-64 Linux)                          */
/* ------------------------------------------------------------------ */

typedef struct {          /* common prefix of every XEvent            */
    int type;
    unsigned long serial;
    int send_event;
    void* display;
    unsigned long window;
} XAnyEv;                 /* sizeof == 40                             */

typedef struct {          /* KeyPress/KeyRelease/Button/Motion        */
    int type;
    unsigned long serial;
    int send_event;
    void* display;
    unsigned long window;
    unsigned long root;
    unsigned long subwindow;
    unsigned long time;
    int x, y;
    int x_root, y_root;
    unsigned int state;
    unsigned int detail;  /* keycode or button                        */
    int same_screen;
} XPointerEv;             /* sizeof == 96                             */

typedef struct {
    int type;
    unsigned long serial;
    int send_event;
    void* display;
    unsigned long window;
    int width, height;    /* offsets 56 / 60                          */
} XResizeEv;

typedef struct {
    int type;
    unsigned long serial;
    int send_event;
    void* display;
    unsigned long window;
    unsigned long message_type;   /* @40                              */
    int format;                   /* @48                              */
    long data[5];                 /* @56 (8-byte aligned union)       */
} XClientMsgEv;

enum {
    XEV_KEY_PRESS     = 2,
    XEV_KEY_RELEASE   = 3,
    XEV_BUTTON_PRESS  = 4,
    XEV_BUTTON_RELEASE= 5,
    XEV_MOTION        = 6,
    XEV_CONFIGURE     = 22,
    XEV_CLIENT_MSG    = 33
};

enum {  /* event masks */
    MASK_ALL = 0x00000001 | 0x00000002 |   /* key press/release       */
               0x00000004 | 0x00000008 |   /* button press/release    */
               0x00000040 |                /* pointer motion          */
               0x00008000 |                /* exposure                */
               0x00020000                  /* structure notify        */
};

#define ZPIXMAP 2

/* ------------------------------------------------------------------ */
/* Dynamically bound X11 entry points                                  */
/* ------------------------------------------------------------------ */

static void*              (*pXOpenDisplay)(const char*);
static int                (*pXCloseDisplay)(void*);
static int                (*pXDefaultScreen)(void*);
static unsigned long      (*pXRootWindow)(void*, int);
static void*              (*pXDefaultVisual)(void*, int);
static int                (*pXDefaultDepth)(void*, int);
static void*              (*pXDefaultGC)(void*, int);
static unsigned long      (*pXCreateSimpleWindow)(void*, unsigned long,
                                                  int, int, unsigned,
                                                  unsigned, unsigned,
                                                  unsigned long,
                                                  unsigned long);
static int                (*pXStoreName)(void*, unsigned long, const char*);
static int                (*pXSelectInput)(void*, unsigned long, long);
static int                (*pXMapWindow)(void*, unsigned long);
static int                (*pXPending)(void*);
static int                (*pXNextEvent)(void*, void*);
static int                (*pXFlush)(void*);
static int                (*pXDestroyWindow)(void*, unsigned long);
static unsigned long      (*pXInternAtom)(void*, const char*, int);
static int                (*pXSetWMProtocols)(void*, unsigned long,
                                              unsigned long*, int);
static unsigned long      (*pXLookupKeysym)(void*, int);
static void*              (*pXCreateImage)(void*, void*, unsigned, int,
                                           int, char*, unsigned, unsigned,
                                           int, int);
static int                (*pXPutImage)(void*, unsigned long, void*,
                                        void*, int, int, int, int,
                                        unsigned, unsigned);
static int                (*pXDestroyImage)(void*);

struct CGuiDriver {
    void* display;
    unsigned long window;
    void* gc;
    int screen;
    int depth;
    unsigned long delete_atom;
    int should_close;
    int width, height;
};

/* Load libX11 once and resolve every symbol we need. */
static int load_xlib(void)
{
    static void* lib = NULL;
    static int loaded = 0;

    if (loaded) return lib != NULL;
    loaded = 1;

    const char* names[] = { "libX11.so.6", "libX11.so", NULL };
    for (int i = 0; names[i] && !lib; ++i)
        lib = dlopen(names[i], RTLD_NOW | RTLD_GLOBAL);
    if (!lib) return 0;

    struct Sym { const char* name; void** out; };
    struct Sym syms[] = {
        { "XOpenDisplay",      (void**)&pXOpenDisplay },
        { "XCloseDisplay",     (void**)&pXCloseDisplay },
        { "XDefaultScreen",    (void**)&pXDefaultScreen },
        { "XRootWindow",       (void**)&pXRootWindow },
        { "XDefaultVisual",    (void**)&pXDefaultVisual },
        { "XDefaultDepth",     (void**)&pXDefaultDepth },
        { "XDefaultGC",        (void**)&pXDefaultGC },
        { "XCreateSimpleWindow",(void**)&pXCreateSimpleWindow },
        { "XStoreName",        (void**)&pXStoreName },
        { "XSelectInput",      (void**)&pXSelectInput },
        { "XMapWindow",        (void**)&pXMapWindow },
        { "XPending",          (void**)&pXPending },
        { "XNextEvent",        (void**)&pXNextEvent },
        { "XFlush",            (void**)&pXFlush },
        { "XDestroyWindow",    (void**)&pXDestroyWindow },
        { "XInternAtom",       (void**)&pXInternAtom },
        { "XSetWMProtocols",   (void**)&pXSetWMProtocols },
        { "XLookupKeysym",     (void**)&pXLookupKeysym },
        { "XCreateImage",      (void**)&pXCreateImage },
        { "XPutImage",         (void**)&pXPutImage },
        { "XDestroyImage",     (void**)&pXDestroyImage },
    };
    for (size_t i = 0; i < sizeof(syms)/sizeof(syms[0]); ++i) {
        *syms[i].out = dlsym(lib, syms[i].name);
        if (!*syms[i].out) return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

int cgui_supported(void)
{
    if (getenv("DISPLAY") == NULL || getenv("DISPLAY")[0] == '\0')
        return 0;
    return load_xlib();
}

CGuiDriver* cgui_open(const char* title, int width, int height)
{
    if (!load_xlib()) return NULL;

    void* dpy = pXOpenDisplay(NULL);
    if (!dpy) return NULL;

    CGuiDriver* drv = (CGuiDriver*)calloc(1, sizeof(CGuiDriver));
    if (!drv) { pXCloseDisplay(dpy); return NULL; }

    drv->display = dpy;
    drv->screen  = pXDefaultScreen(dpy);
    drv->depth   = pXDefaultDepth(dpy, drv->screen);
    drv->width   = width;
    drv->height  = height;

    unsigned long parent = pXRootWindow(dpy, drv->screen);
    drv->window = pXCreateSimpleWindow(dpy, parent, 10, 10,
                                       (unsigned)width, (unsigned)height,
                                       1, 0x333333UL, 0x1E1E28UL);
    if (!drv->window) {
        free(drv);
        pXCloseDisplay(dpy);
        return NULL;
    }

    drv->gc = pXDefaultGC(dpy, drv->screen);

    pXStoreName(dpy, drv->window, title ? title : "engc-gui");
    pXSelectInput(dpy, drv->window, MASK_ALL);
    pXMapWindow(dpy, drv->window);

    drv->delete_atom = pXInternAtom(dpy, "WM_DELETE_WINDOW", 0);
    if (drv->delete_atom)
        pXSetWMProtocols(dpy, drv->window, &drv->delete_atom, 1);

    pXFlush(dpy);
    return drv;
}

void cgui_close(CGuiDriver* drv)
{
    if (!drv) return;
    if (drv->display) {
        if (drv->window)
            pXDestroyWindow(drv->display, drv->window);
        pXCloseDisplay(drv->display);
    }
    free(drv);
}

int cgui_should_close(CGuiDriver* drv)
{
    return drv ? drv->should_close : 0;
}

void cgui_query_size(CGuiDriver* drv, int* w, int* h)
{
    if (w) *w = drv ? drv->width : 0;
    if (h) *h = drv ? drv->height : 0;
}

int cgui_poll(CGuiDriver* drv, CGuiEvent* ev)
{
    if (!drv || !ev) return CGUI_EV_NONE;

    char raw[192];
    memset(raw, 0, sizeof(raw));

    while (pXPending(drv->display) > 0) {
        pXNextEvent(drv->display, raw);
        int type = *(int*)raw;

        switch (type) {
            case XEV_MOTION: {
                const XPointerEv* e = (const XPointerEv*)raw;
                ev->type = CGUI_EV_MOUSE_MOVE;
                ev->x = e->x; ev->y = e->y;
                return ev->type;
            }
            case XEV_BUTTON_PRESS: {
                const XPointerEv* e = (const XPointerEv*)raw;
                if (e->detail >= 4 && e->detail <= 5) {   /* wheel */
                    ev->type = CGUI_EV_MOUSE_WHEEL;
                    ev->button = (e->detail == 4) ? 1 : -1;
                    ev->x = e->x; ev->y = e->y;
                } else {
                    ev->type = CGUI_EV_MOUSE_DOWN;
                    ev->button = (e->detail <= 3) ? (int)e->detail : 1;
                    ev->x = e->x; ev->y = e->y;
                }
                return ev->type;
            }
            case XEV_BUTTON_RELEASE: {
                const XPointerEv* e = (const XPointerEv*)raw;
                if (e->detail >= 4 && e->detail <= 5)
                    continue;                              /* ignore wheel release */
                ev->type = CGUI_EV_MOUSE_UP;
                ev->button = (e->detail <= 3) ? (int)e->detail : 1;
                ev->x = e->x; ev->y = e->y;
                return ev->type;
            }
            case XEV_KEY_PRESS:
            case XEV_KEY_RELEASE: {
                const XPointerEv* e = (const XPointerEv*)raw;
                ev->key = pXLookupKeysym((void*)raw, 0);
                ev->type = (type == XEV_KEY_PRESS) ? CGUI_EV_KEY_DOWN
                                                   : CGUI_EV_KEY_UP;
                ev->x = e->x; ev->y = e->y;
                return ev->type;
            }
            case XEV_CONFIGURE: {
                const XResizeEv* e = (const XResizeEv*)raw;
                if (e->width != drv->width || e->height != drv->height) {
                    drv->width = e->width;
                    drv->height = e->height;
                    ev->type = CGUI_EV_WINDOW_RESIZE;
                    ev->width = e->width;
                    ev->height = e->height;
                    return ev->type;
                }
                continue;
            }
            case XEV_CLIENT_MSG: {
                const XClientMsgEv* e = (const XClientMsgEv*)raw;
                if (drv->delete_atom &&
                    e->message_type == drv->delete_atom) {
                    drv->should_close = 1;
                    ev->type = CGUI_EV_CLOSE;
                    return ev->type;
                }
                continue;
            }
            default:
                continue;   /* Expose etc. — caller redraws every frame */
        }
    }
    return CGUI_EV_NONE;
}

void cgui_blit(CGuiDriver* drv, const unsigned char* bgra,
               int w, int h)
{
    if (!drv || !bgra || w <= 0 || h <= 0) return;

    size_t nbytes = (size_t)w * (size_t)h * 4u;
    char* tmp = (char*)malloc(nbytes);
    if (!tmp) return;
    memcpy(tmp, bgra, nbytes);   /* already BGRA from the C++ side */

    void* img = pXCreateImage(drv->display,
                              pXDefaultVisual(drv->display, drv->screen),
                              (unsigned)drv->depth, ZPIXMAP, 0,
                              tmp, (unsigned)w, (unsigned)h, 32, 0);
    if (!img) { free(tmp); return; }

    pXPutImage(drv->display, drv->window, drv->gc, img,
               0, 0, 0, 0, (unsigned)w, (unsigned)h);
    pXDestroyImage(img);         /* frees tmp as well */
    pXFlush(drv->display);
}

const char* cgui_backend_name(void)
{
    return "x11";
}
