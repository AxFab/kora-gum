#ifndef _KORA_GUM_DISPLAY_H
#define _KORA_GUM_DISPLAY_H 1

#include <kora/gum/core.h>


#define GM_WIN_LOCAL_DISPLAY  "local"

struct GUM_display {
    int m;
};

struct GUM_surface {
    int width;
    int height;
    int wstep;
    int format;
    void *pixels;
    void *info;
    GUM_cell *cell;
    short xdpi, ydpi;
    float xdsp, ydsp;
};


GUM_display *gum_display(const char* host);
GUM_surface *gum_surface(GUM_display *display, int width, int height, int flags, int event_mask);
void gum_close_surface(GUM_surface *surface);
void gum_close_display(GUM_display *display);

void gum_paint(GUM_surface *win, GUM_cell *cell);
void gum_invalid_surface(GUM_surface *win, int x, int y, int w, int h);


#endif  /* _KORA_GUM_DISPLAY_H */
