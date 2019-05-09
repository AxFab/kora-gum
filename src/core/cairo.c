/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 */
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <kora/gum/events.h>
#include <kora/gum/cells.h>
#include <kora/css.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#define _PwNano_ 1000000000

typedef struct xinfo {
    Display *d;
    Window w;
    GC gc;
    int s;
    XImage *i;
} xinfo_t;

struct GUM_window {
    cairo_t *ctx;
    cairo_surface_t *srf;
};

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

long long gum_system_time()
{
    clock_t ticks = clock();
    ticks *= _PwNano_ / CLOCKS_PER_SEC;
    return ticks;
}

struct MwmHints {
    unsigned long flags;
    unsigned long function;
    unsigned long decoration;
    long input_mode;
    unsigned long status;
};

enum {
    MWH_HINTS_FUNCTIONS = (1L << 0),
    MWH_HINTS_DECORATIONS = (1L << 1),

    MWH_FUNC_ALL = (1L << 0),
    MWH_FUNC_RESIZE = (1L << 1),
    MWH_FUNC_MOVE = (1L << 2),
    MWH_FUNC_MINIMZE = (1L << 3),
    MWH_FUNC_MAXIMIZE = (1L << 4),
    MWH_FUNC_CLOSE = (1L << 5),
};

GUM_window *__lastWin = NULL;

GUM_window *gum_create_surface(int width, int height)
{
    Display *dsp;
    Drawable da;
    int screen;
    cairo_surface_t *sfc;

    if ((dsp = XOpenDisplay(NULL)) == NULL) {
        fprintf(stderr, "Unable to open X11 display\n");
        return NULL;
    }
    screen = DefaultScreen(dsp);
    da = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp), 0, 0, width, height, 0, 0, 0);
    XSelectInput(dsp, da,
                 ButtonMotionMask |  ButtonPressMask | ButtonReleaseMask | // Mouse
                 KeyPressMask | KeyReleaseMask | KeymapStateMask | // Keyboard
                 EnterWindowMask | LeaveWindowMask | PointerMotionMask |
                 // Button1MotionMask | Button2MotionMask | Button3MotionMask |
                 // Button4MotionMask | Button5MotionMask |
                 VisibilityChangeMask | StructureNotifyMask | ResizeRedirectMask |
                 SubstructureNotifyMask | SubstructureRedirectMask | FocusChangeMask |
                 PropertyChangeMask | ColormapChangeMask |
                 ExposureMask);

    // Make the window, frameless
    Atom mwmHintsProperty = XInternAtom(dsp, "_MOTIF_WM_HINTS", 0);
    struct MwmHints hints;
    hints.flags = MWH_HINTS_DECORATIONS;
    hints.decoration = 0;
    XChangeProperty(dsp, da, mwmHintsProperty, mwmHintsProperty, 32,
                    PropModeReplace, (unsigned char *)&hints, 5);

    XMapWindow(dsp, da);

    sfc = cairo_xlib_surface_create(dsp, da, DefaultVisual(dsp, screen), width, height);
    cairo_xlib_surface_set_size(sfc, width, height);

    timer_setup();

    GUM_window *win = (GUM_window *)malloc(sizeof(GUM_window));
    win->srf = sfc;
    win->ctx = cairo_create(sfc);
    __lastWin = win;
    return win;
}

void gum_destroy_surface(GUM_window *win)
{
    cairo_destroy(win->ctx);
    Display *dsp = cairo_xlib_surface_get_display(win->srf);
    cairo_surface_destroy(win->srf);
    XCloseDisplay(dsp);
    free(win);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

#define MIN_ALPHA 0x1000000
#define M_PI 3.141592653589793


static void gum_draw_path(void *ctx, GUM_cell *cell, GUM_skin *skin)
{
    if (cell->path) {
        cairo_new_path(ctx);
        cairo_append_path(ctx, (cairo_path_t *)cell->path);
        return;
    }

    int sz = MIN(cell->box.w, cell->box.h);
    int r_top_left = CSS_GET_UNIT(skin->r_top_left, skin->u_top_left, 96, 0.75, sz);
    int r_top_right = CSS_GET_UNIT(skin->r_top_right, skin->u_top_right, 96, 0.75, sz);
    int r_bottom_right = CSS_GET_UNIT(skin->r_bottom_right, skin->u_bottom_right, 96, 0.75, sz);
    int r_bottom_left = CSS_GET_UNIT(skin->r_bottom_left, skin->u_bottom_left, 96, 0.75, sz);

    cairo_new_path(ctx);
    cairo_move_to(ctx, cell->box.x + r_top_left, cell->box.y);
    cairo_line_to(ctx, cell->box.x + cell->box.w - r_top_right, cell->box.y);
    cairo_arc(ctx, cell->box.x + cell->box.w - r_top_right, cell->box.y + r_top_right, r_top_right, -M_PI / 2.0, 0.0);
    cairo_line_to(ctx, cell->box.x + cell->box.w, cell->box.y + cell->box.h - r_bottom_right);
    cairo_arc(ctx, cell->box.x + cell->box.w - r_bottom_right, cell->box.y + cell->box.h - r_bottom_right, r_bottom_right, 0.0, M_PI / 2.0);
    cairo_line_to(ctx, cell->box.x + r_bottom_left, cell->box.y + cell->box.h);
    cairo_arc(ctx, cell->box.x + r_bottom_left, cell->box.y + cell->box.h - r_bottom_left, r_bottom_left, M_PI / 2.0, M_PI);
    cairo_line_to(ctx, cell->box.x, cell->box.y + r_top_left);
    cairo_arc(ctx, cell->box.x + r_top_left, cell->box.y + r_top_left, r_top_left, M_PI, 3 * M_PI / 2.0);

    cell->path = cairo_copy_path(ctx);
}

static cairo_pattern_t *gum_build_gradient(GUM_cell *cell, GUM_skin *skin)
{
    if (cell->gradient)
        return (cairo_pattern_t *)cell->gradient;

    cairo_pattern_t *grad;

    if (skin->grad_angle == 90)
        grad = cairo_pattern_create_linear(cell->box.x + cell->box.w, 0, cell->box.x, 0);
    else if (skin->grad_angle == 270)
        grad = cairo_pattern_create_linear(cell->box.x, 0, cell->box.x + cell->box.w, 0);
    else
        grad = cairo_pattern_create_linear(0, cell->box.y, 0, cell->box.y + cell->box.h);

    cairo_pattern_add_color_stop_rgb(grad, 0.0, //0, 0.5, 0.5, 0);
                                     ((skin->bgcolor >> 16) & 255) / 255.0,
                                     ((skin->bgcolor >> 8) & 255) / 255.0,
                                     ((skin->bgcolor >> 0) & 255) / 255.0);
    // cairo_pattern_add_color_stop_rgb(grad, 0.25, 0.5, 0.5, 0);
    // ((skin->grcolor >> 16) & 255) / 255.0,
    // ((skin->grcolor >> 8) & 255) / 255.0,
    // ((skin->grcolor >> 0) & 255) / 255.0);
    cairo_pattern_add_color_stop_rgb(grad, 1.0, //0, 0.5, 0.5, 0);
                                     ((skin->grcolor >> 16) & 255) / 255.0,
                                     ((skin->grcolor >> 8) & 255) / 255.0,
                                     ((skin->grcolor >> 0) & 255) / 255.0);

    cell->gradient = grad;
    return grad;
}

void gum_draw_cell(GUM_window *win, GUM_cell *cell, bool top)
{
    cairo_t *ctx = win->ctx;
    GUM_skin *skin = gum_skin(cell);
    if (skin == NULL)
        return;

    if (cell->cachedSkin != skin) {
        cell->path = NULL;
        cell->gradient = NULL;
        cell->cachedSkin = skin;
    }

    if (cell->image == NULL && cell->img_src != NULL)
        cell->image = gum_load_image(cell->img_src);

    if (skin->bgcolor >= MIN_ALPHA || skin->brcolor >= MIN_ALPHA || cell->image) {

        gum_draw_path(ctx, cell, skin);

        if (cell->image) {
            cairo_surface_t *img = (cairo_surface_t *)cell->image;
            int img_sz = MAX(cairo_image_surface_get_width(img), cairo_image_surface_get_height(img));
            double rt = (double)MAX(cell->box.w, cell->box.h) / (double)img_sz;
            cairo_save(ctx);
            cairo_translate(ctx, cell->box.x, cell->box.y);
            cairo_scale(ctx, rt, rt);
            cairo_set_source_surface(ctx, img, 0, 0);
            cairo_fill_preserve(ctx);
            cairo_restore(ctx);

        } else if (skin->grcolor >= MIN_ALPHA) {

            cairo_pattern_t *grad = gum_build_gradient(cell, skin);
            cairo_set_source(ctx, grad);
            cairo_fill_preserve(ctx);

        } else if (skin->bgcolor >= MIN_ALPHA) {
            cairo_set_source_rgb(ctx, //0.8, 0.8, 0.8);
                                 ((skin->bgcolor >> 16) & 255) / 255.0,
                                 ((skin->bgcolor >> 8) & 255) / 255.0,
                                 ((skin->bgcolor >> 0) & 255) / 255.0);
            cairo_fill_preserve(ctx);
        }

        if (skin->brcolor >= MIN_ALPHA) {
            cairo_set_line_width(ctx, 1.0);
            cairo_set_source_rgb(ctx, //0.1, 0.1, 0.1);
                                 ((skin->brcolor >> 16) & 255) / 255.0,
                                 ((skin->brcolor >> 8) & 255) / 255.0,
                                 ((skin->brcolor >> 0) & 255) / 255.0);
            cairo_stroke(ctx);
        }
    }

    // fprintf(stderr, " -- %s\n", cell->text);
    if (cell->text) {
        cairo_set_source_rgb(ctx, //0.8, 0.8, 0.8);
                             ((skin->txcolor >> 16) & 255) / 255.0,
                             ((skin->txcolor >> 8) & 255) / 255.0,
                             ((skin->txcolor >> 0) & 255) / 255.0);

        cairo_text_extents_t extents;

        const char *ffamily = skin->font_family ? skin->font_family : "Sans";
        cairo_select_font_face(ctx, ffamily, CAIRO_FONT_SLANT_NORMAL, 0);
        cairo_set_font_size(ctx, (float)skin->font_size);
        cairo_text_extents(ctx, cell->text, &extents);
        int tx = cell->box.x;
        int ty = cell->box.y;

        if (skin->align == 2)
            tx += cell->box.w - (extents.width + extents.x_bearing);
        else if (skin->align == 0)
            tx += cell->box.w / 2 - (extents.width / 2 + extents.x_bearing);

        if (skin->valign == 2)
            ty += cell->box.h - (extents.height + extents.y_bearing);
        else if (skin->valign == 0)
            ty += cell->box.h / 2 - (extents.height / 2 + extents.y_bearing);

        cairo_move_to(ctx, tx, ty);
        cairo_show_text(ctx, cell->text);
    }
}


cairo_surface_t *srf = NULL;
cairo_t *ctx;

void gum_text_size(const char *text, int *w, int *h, GUM_skin *skin)
{
    if (srf == NULL) {
        srf = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 10, 10);
        ctx = cairo_create(srf);
    }
    const char *ffamily = skin->font_family ? skin->font_family : "Sans";
    fprintf(stderr, "Select Font %s %d\n", ffamily, skin->font_size);
    cairo_select_font_face(ctx, ffamily, CAIRO_FONT_SLANT_NORMAL, 0);
    cairo_set_font_size(ctx, (float)skin->font_size);

    cairo_text_extents_t extents;
    cairo_text_extents(ctx, text, &extents);
    *w = extents.width + extents.x_bearing;
    *h = extents.height + extents.y_bearing;
}


void gum_draw_scrolls(GUM_window *win, GUM_cell *cell)
{
    cairo_t *ctx = win->ctx;
    if (cell->state & GUM_CELL_OVERFLOW_X) {
        cairo_new_path(ctx);
        cairo_rectangle(ctx,
                        cell->box.x,
                        cell->box.y + cell->box.h - 7,
                        cell->box.w - 7, 7);
        cairo_set_source_rgb(ctx, 0.7, 0.7, 0.7);
        cairo_fill(ctx);

        int sz = cell->box.cw * (cell->box.w - 7) / cell->box.ch_w;
        int st = cell->box.sx * (cell->box.w - 7) / cell->box.ch_w;

        cairo_new_path(ctx);
        cairo_rectangle(ctx,
                        cell->box.x + st,
                        cell->box.y + cell->box.h - 7,
                        sz, 7);
        cairo_set_source_rgb(ctx, 66.0 / 255.0, 165.0 / 255.0, 245.0 / 255.0);
        cairo_fill(ctx);
    }

    if (cell->state & GUM_CELL_OVERFLOW_Y) {
        cairo_new_path(ctx);
        cairo_rectangle(ctx,
                        cell->box.x + cell->box.w - 7,
                        cell->box.y, 7, cell->box.h - 7);
        cairo_set_source_rgb(ctx, 0.7, 0.7, 0.7);
        cairo_fill(ctx);

        int sz = cell->box.ch * (cell->box.h - 7) / cell->box.ch_h;
        int st = cell->box.sy * (cell->box.h - 7) / cell->box.ch_h;

        cairo_new_path(ctx);
        cairo_rectangle(ctx,
                        cell->box.x + cell->box.w - 7,
                        cell->box.y + st, 7, sz);
        cairo_set_source_rgb(ctx, 66.0 / 255.0, 165.0 / 255.0, 245.0 / 255.0);
        cairo_fill(ctx);
    }
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

#include <kora/keys.h>

int key_unicode(int kcode, int state)
{
    int r = 0;
    if ((state & 3) == 0 || ((state & 3) == 3))
        r += 1;
    if (state & 8)
        r += 2;
    return keyboard_layout_US[kcode][r];
}


void timer_handler (int signum)
{
    XEvent e;
    memset(&e, 0, sizeof(e));
    XExposeEvent *ee = &e;
    ee->type = Expose;
    ee->x = 0;
    ee->y = 0;
    ee->width = cairo_xlib_surface_get_width(__lastWin->srf);
    ee->height = cairo_xlib_surface_get_height(__lastWin->srf);

    Display *dsp = cairo_xlib_surface_get_display(__lastWin->srf);
    Window w = (Window)cairo_xlib_surface_get_drawable (__lastWin->srf);
    XSendEvent(dsp, w, False, ExposureMask, &e);
}

struct sigaction sa;
struct itimerval timer;

void timer_setup()
{

    /* Install timer_handler as the signal handler for SIGVTALRM. */
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &timer_handler;
    sigaction (SIGALRM, &sa, NULL);

    /* Configure the timer to expire after 250 msec... */
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 20000;
    /* ... and every 250 msec after that. */
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 20000;
    /* Start a virtual timer. It counts down whenever this process is
    executing. */
    setitimer (ITIMER_REAL, &timer, NULL);
}

void gum_fill_context(GUM_window *win, GUM_gctx *ctx)
{
    ctx->dpi_x = 96;
    ctx->dpi_y = 96;
    ctx->dsp_x = 2; //0.75;
    ctx->dsp_y = 2; //0.75;
    ctx->width = cairo_xlib_surface_get_width(win->srf);
    ctx->height = cairo_xlib_surface_get_height(win->srf);
}


int gum_event_poll(GUM_window *win, GUM_event *event, int timeout)
{
    int unicode;
    XEvent e;
    XMotionEvent *motion = (XMotionEvent *)&e;
    XButtonEvent *btn = (XButtonEvent *)&e;
    XKeyEvent *key = (XKeyEvent *)&e;
    XResizeRequestEvent *resz = (XResizeRequestEvent *)&e;

    do {
        XNextEvent(cairo_xlib_surface_get_display(win->srf), &e);
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
            if (btn->button == 4) {
                // printf("Event Wheel Up\n");
                event->type = GUM_EV_WHEEL_UP;
            } else if (btn->button == 5) {
                // printf("Event Wheel Down\n");
                event->type = GUM_EV_WHEEL_DOWN;
            } else {
                // printf("Event ButtonPress <%x.%x>\n", btn->state, btn->button);
                event->type = GUM_EV_BTN_PRESS;
                event->param0 = btn->button;
                event->param1 = btn->state;
            }
            break;
        case ButtonRelease:
            if (btn->button == 4 || btn->button == 5)
                event->type = -1;

            else {
                // printf("Event ButtonRelease <%x.%x>\n", btn->state, btn->button);
                event->type = GUM_EV_BTN_RELEASE;
                event->param0 = btn->button;
                event->param1 = btn->state;
            }
            break;
        case ResizeRequest:
            event->type = GUM_EV_RESIZE;
            cairo_xlib_surface_set_size(win->srf, resz->width, resz->height);
            event->param0 = resz->width;
            event->param1 = resz->height;
            break;
        case EnterNotify:
            // printf("EnterNotify\n");
            event->type = -1;
            break;
        case LeaveNotify:
            // printf("LeaveNotify\n");
            event->type = -1;
            break;
        case FocusIn:
        case FocusOut:
        case KeymapNotify:
        case GraphicsExpose:
        case NoExpose:
        case CirculateRequest:
        case ConfigureRequest:
        case MapRequest:
        case CirculateNotify:
        case ConfigureNotify:
        case CreateNotify:
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
            // printf("Event %d\n", e.type);
            event->type = -1;
            break;
        }
    } while (event->type == -1);
    return 0;
}


void gum_start_paint(GUM_window *win, int x, int y)
{
    cairo_push_group(win->ctx);
    cairo_set_source_rgb(win->ctx, 1, 1, 1);
    cairo_paint(win->ctx);
    cairo_reset_clip(win->ctx);
    // cairo_translate(win->ctx, x, y);
}

void gum_end_paint(GUM_window *win)
{
    cairo_pop_group_to_source(win->ctx);
    cairo_paint(win->ctx);
    cairo_surface_flush(win->srf);
}

void gum_push_clip(GUM_window *win, GUM_box *box)
{
    cairo_save(win->ctx);
    cairo_new_path(win->ctx);
    cairo_rectangle(win->ctx, box->cx, box->cy, box->cw, box->ch);
    cairo_clip(win->ctx);
    cairo_translate(win->ctx, box->cx - box->sx, box->cy - box->sy);
}

void gum_pop_clip(GUM_window *win, GUM_box *box)
{
    cairo_translate(win->ctx, box->sx - box->cx, box->sy - box->cy);
    cairo_restore(win->ctx);
}

void gum_resize_win(GUM_window *win, int width, int height)
{
    cairo_xlib_surface_set_size(win->srf, width, height);
}


void *gum_load_image(const char *name)
{
    cairo_surface_t *img = cairo_image_surface_create_from_png(name);
    if (cairo_surface_status(img) != 0)
        return NULL;
    return img;
}

void gum_do_visual(GUM_cell *cell, GUM_window *win, GUM_sideruler *inval)
{
    gum_paint(win, cell);
}
