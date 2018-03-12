#include <kora/gum/display.h>
#include <stddef.h>
#include <stdlib.h>


GUM_display *gum_display(const char* host)
{
    // TODO - Not supported, we only open local frame
    // We need a structure for: WHERE and HOW (more like a driver)
    return NULL;
}

void gum_close_display(GUM_display *display)
{
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

GUM_surface *gum_image(int width, int height, int format, void* pixels)
{
    GUM_surface *img = (GUM_surface*)malloc(sizeof(GUM_surface));
    img->width = width;
    img->height = height;
    img->wstep = ALIGN_UP(width * (format & 0x1F), 4);
    img->format = format;
    img->pixels = pixels != NULL ? pixels : malloc(img->height * img->wstep);
    img->xdpi = img->ydpi = 96;
    img->xdsp = img->ydsp = 0.75f;
    return img;
}

GUM_surface *gum_surface(GUM_display *display, int width, int height, int flags, int event_mask)
{
    GUM_surface *win = gum_image(width, height, 0x1004, NULL);
    win->info = gum_surface_info(win);
    return win;
}

void gum_close_surface(GUM_surface *win)
{
    gum_free_surface_info(win->info);
    free(win->pixels);
    free(win);
}
