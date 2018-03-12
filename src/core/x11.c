#include <X11/Xlib.h>
#include <kora/gum/display.h>
#include <kora/gum/events.h>
#include <kora/gum/rendering.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct xinfo {
    Display *d;
    Window w;
    GC gc;
    int s;
    XImage *i;
} xinfo_t;


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void* gum_surface_info(GUM_surface *img)
{
    xinfo_t *info = (xinfo_t*)malloc(sizeof(xinfo_t));
    info->d = XOpenDisplay(NULL);
    if (info->d == NULL) {
        fprintf(stderr, "Unable to open X11 display\n");
        return NULL;
    }

    info->s = DefaultScreen(info->d);
    info->w = XCreateSimpleWindow(
        info->d,
        RootWindow(info->d, info->s),
        10, 10, img->width, img->height, 1,
        BlackPixel(info->d, info->s),
        WhitePixel(info->d, info->s)
        );

    XSelectInput(info->d, info->w,
        ButtonMotionMask |  ButtonPressMask | ButtonReleaseMask | // Mouse
        // KeyPressMask | KeyReleaseMask |  // Keyboard
        // KeymapStateMask |
        // EnterWindowMask | LeaveWindowMask | PointerMotionMask |
        // PointerMotionHintMask |
        // Button1MotionMask | Button2MotionMask | Button3MotionMask |
        // Button4MotionMask | Button5MotionMask |
        // VisibilityChangeMask | StructureNotifyMask | ResizeRedirectMask |
        // SubstructureNotifyMask | SubstructureRedirectMask | FocusChangeMask |
        // PropertyChangeMask | ColormapChangeMask |
        ExposureMask);

    XMapWindow(info->d, info->w);
    info->gc = XDefaultGC(info->d, info->s);

    info->i = XCreateImage(info->d, NULL, 4, XYBitmap, 0, img->pixels, img->width, img->height, 0, img->wstep);
    return info;
}

void gum_free_surface_info(void *data)
{
    xinfo_t *info = (xinfo_t*)data;
    XCloseDisplay(info->d);
    free(info);
}

void gum_invalid_surface(GUM_surface *win, int x, int y, int w, int h)
{
    xinfo_t *info = (xinfo_t*)win->info;

    // printf("Invalid <%d, %d, %d, %d>\n", x, y, width, height);
    XExposeEvent paint;
    memset(&paint, 0, sizeof(XExposeEvent));
    paint.type = Expose;
    paint.send_event = True;
    paint.display = info->d;
    paint.window = info->w;
    paint.x = x;
    paint.y = y;
    paint.width = w;
    paint.height = h;
    XSendEvent(info->d, info->w, True, ExposureMask, (XEvent*)&paint);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */


XFontStruct *font = NULL;
#define MIN_ALPHA 0x1000000

static unsigned int mix(unsigned int c1, unsigned int c2, float F)
{
    int r1 = (c1 >> 16) & 0xFF;
    int g1 = (c1 >> 8) & 0xFF;
    int b1 = (c1) & 0xFF;
    int r2 = (c2 >> 16) & 0xFF;
    int g2 = (c2 >> 8) & 0xFF;
    int b2 = (c2) & 0xFF;
    int r0 = r1 + (r2 - r1) * F;
    int g0 = g1 + (g2 - g1) * F;
    int b0 = b1 + (b2 - b1) * F;
  return (r0 << 16) | (g0 << 8) | b0;
}

static void X_FillRectangle(xinfo_t *info, unsigned int color, int x, int y, int w, int h)
{
    XSetForeground(info->d, info->gc, color & 0xFFFFFF);
    XFillRectangle(info->d, info->w, info->gc, x, y, w, h);
}
static void X_DrawRectangle(xinfo_t *info, unsigned int color, int x, int y, int w, int h)
{
    XSetForeground(info->d, info->gc, color & 0xFFFFFF);
    XDrawRectangle(info->d, info->w, info->gc, x, y, w-1, h-1);
}
static void X_GradRectangle(xinfo_t *info, unsigned int c1, unsigned int c2, int x, int y, int w, int h)
{
    int s;
    for (s = 0; s < h; ++s) {
    float F = (float)s / (h-1);
        XSetForeground(info->d, info->gc, mix(c1, c2, F));
        XFillRectangle(info->d, info->w, info->gc, x, y + s, w, 1);
    }
}

static void X_LoadFont(xinfo_t *info)
{
    font = XLoadQueryFont(info->d, "-*-helvetica-*-r-*-*-12-*-*-*-*-*-*-*");
    /* If the font could not be loaded, revert to the "fixed" font. */
    if (!font) {
        fprintf (stderr, "unable to load font %s: using fixed\n", "Helvetica");
        font = XLoadQueryFont(info->d, "fixed");
    }
}

static void X_FontSize(GUM_cell *cell, GUM_skin *skin, int lg, int *tx, int *ty) {

    int direction, ascent, descent;
    XCharStruct overall;
    XTextExtents(font, cell->text, lg, &direction, &ascent, &descent, &overall);

    if (skin->align == 2)
        *tx += cell->box.cw - overall.width;
    else if (skin->align == 0)
        *tx += (cell->box.cw - overall.width) / 2;

    if (skin->valign == 2)
        *ty += cell->box.ch + (ascent - descent);
    else if (skin->valign == 0)
        *ty += (cell->box.ch + (ascent - descent)) / 2;
}

void gum_draw_cell(void *data, GUM_cell *cell, int x, int y)
{
    xinfo_t *info = (xinfo_t*)data;

    if (font == NULL)
        X_LoadFont(info);

    GUM_skin *skin = gum_skin(cell);
    if (skin == NULL)
        return;

    if (skin->bgcolor > MIN_ALPHA) {
        if (skin->grcolor > MIN_ALPHA)
            X_GradRectangle(info, skin->bgcolor, skin->grcolor,
                cell->box.x + x, cell->box.y + y, cell->box.w, cell->box.h);
        else
            X_FillRectangle(info, skin->bgcolor,
                cell->box.x + x, cell->box.y + y, cell->box.w, cell->box.h);
    }

    if (skin->brcolor > MIN_ALPHA)
        X_DrawRectangle(info, skin->brcolor,
            cell->box.x + x, cell->box.y + y, cell->box.w, cell->box.h);

    if (skin->incolor > MIN_ALPHA)
        X_DrawRectangle(info, skin->incolor,
            cell->box.x + x + 1, cell->box.y + y + 1, cell->box.w - 2, cell->box.h - 2);

    if (skin->shcolor > MIN_ALPHA)
        X_FillRectangle(info, skin->shcolor,
            cell->box.x + x, cell->box.y + y + cell->box.h, cell->box.w, 1);


    if (font != NULL && cell->text != NULL) {
        int lg = strlen(cell->text);

        XSetForeground(info->d, info->gc, skin->txcolor);
        XSetFont(info->d, info->gc, font->fid);

        int tx = cell->box.x + x;
        int ty = cell->box.y + y;
        X_FontSize(cell, skin, lg, &tx, &ty);
        XDrawString(info->d, info->w, info->gc, tx, ty, cell->text, lg);
    }

    // Borders
    // XSetForeground(info->d, info->gc, 0xFF0000);
    // XDrawRectangle(info->d, info->w, info->gc,
    //     cell->box.x + x, cell->box.y + y, cell->box.w - 1, cell->box.h - 1);

}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

#include <kora/keys.h>

int key_unicode(int kcode, int state) {
  int r = 0;
  if ((state & 3) == 0 || ((state & 3) == 3))
    r += 1;
  if (state & 8)
    r += 2;
  return keyboard_layout_US[kcode][r];
}



int gum_event_poll(void *data, GUM_event *event, int timeout)
{
    xinfo_t *info = (xinfo_t*)data;

    int unicode;
    XEvent e;
    XMotionEvent *motion = (XMotionEvent *)&e;
    XButtonEvent *btn = (XButtonEvent *)&e;
    XKeyEvent *key = (XKeyEvent *)&e;

    XNextEvent(info->d, &e);
    switch (e.type) {
    case Expose:
        event->type = GUM_EV_EXPOSE;
        // printf("Event Expose\n");
        break;

    case KeyPress:
        event->type = GUM_EV_KEY_PRESS;
        unicode = key_unicode(key->keycode, key->state);
        // printf("Event KeyPress <%x.%x-%x>\n", key->state, key->keycode, unicode);
        event->param0 = unicode;
        event->param1 = (key->state << 16) | key->keycode;
        break;
    case KeyRelease:
        event->type = GUM_EV_KEY_RELEASE;
        unicode = key_unicode(key->keycode, key->state);
        // printf("Event KeyRelease <%x.%x-%x>\n", key->state, key->keycode, unicode);
        event->param0 = unicode;
        event->param1 = (key->state << 16) | key->keycode;
        break;
    case MotionNotify:
        event->type = GUM_EV_MOTION;
        event->param0 = motion->x;
        event->param1 = motion->y;
        // printf("Event Motion <%d, %d>\n", motion->x, motion->y);
        break;
    case ButtonPress:
        // printf("Event ButtonPress <%x.%x>\n", btn->state, btn->button);
        event->type = GUM_EV_BTN_PRESS;
        event->param0 = btn->button;
        event->param1 = btn->state;
        break;
    case ButtonRelease:
        // printf("Event ButtonRelease <%x.%x>\n", btn->state, btn->button);
        event->type = GUM_EV_BTN_RELEASE;
        event->param0 = btn->button;
        event->param1 = btn->state;
        break;
    case EnterNotify:
    case LeaveNotify:
    case FocusIn:
    case FocusOut:
    case KeymapNotify:
    case GraphicsExpose:
    case NoExpose:
    case CirculateRequest:
    case ConfigureRequest:
    case MapRequest:
    case ResizeRequest:
    case CirculateNotify:
    case ConfigureNotify:
    case CreateNotify:
    case DestroyNotify:
    case GravityNotify:
    case MapNotify:
    case MappingNotify:
    case ReparentNotify:
    case UnmapNotify:
    case VisibilityNotify:
    case ColormapNotify:
    case ClientMessage:
    case PropertyNotify:
    case SelectionClear:
    case SelectionNotify:
    case SelectionRequest:
    default:
        printf("Event %d\n", e.type);
        event->type = 0;
        break;
    }
    return 0;
}

