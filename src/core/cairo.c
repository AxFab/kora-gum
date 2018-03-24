#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
// #include <kora/gum/display.h>
#include <kora/gum/events.h>
#include <kora/gum/cells.h>
#include <kora/css.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

typedef struct xinfo {
    Display *d;
    Window w;
    GC gc;
    int s;
    XImage *i;
} xinfo_t;


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */


void *gum_create_surface(int width, int height)
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

    XMapWindow(dsp, da);

    sfc = cairo_xlib_surface_create(dsp, da, DefaultVisual(dsp, screen), width, height);
    cairo_xlib_surface_set_size(sfc, width, height);
    return sfc;
}

void gum_destroy_surface(void *win)
{
   Display *dsp = cairo_xlib_surface_get_display((cairo_surface_t*)win);
   cairo_surface_destroy((cairo_surface_t*)win);
   XCloseDisplay(dsp);
}

void gum_invalid_surface(void *win, int x, int y, int w, int h)
{
    Display *dsp = cairo_xlib_surface_get_display((cairo_surface_t*)win);
    Drawable da = cairo_xlib_surface_get_drawable((cairo_surface_t*)win);
    // printf("Invalid <%d, %d, %d, %d>\n", x, y, width, height);
    XExposeEvent paint;
    memset(&paint, 0, sizeof(XExposeEvent));
    paint.type = Expose;
    paint.send_event = True;
    paint.display = dsp;
    paint.window = da;
    paint.x = x;
    paint.y = y;
    paint.width = w;
    paint.height = h;
    XSendEvent(dsp, da, True, ExposureMask, (XEvent*)&paint);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

#define MIN_ALPHA 0x1000000
#define M_PI 3.141592653589793


void *gum_context(void *win)
{
    cairo_t *ctx = cairo_create((cairo_surface_t*)win);
    cairo_push_group(ctx);
    cairo_set_source_rgb(ctx, 1, 1, 1);
    cairo_paint(ctx);
    return ctx;
}

void gum_complete(void *win, void *ctx)
{
    cairo_pop_group_to_source((cairo_t*)ctx);
    cairo_paint((cairo_t*)ctx);
    cairo_surface_flush((cairo_surface_t*)win);
    cairo_destroy((cairo_t*)ctx);
}

void gum_draw_cell(void *ctx, GUM_cell *cell)
{
    GUM_skin *skin = gum_skin(cell);
    if (skin == NULL)
        return;

    if (cell->image == NULL && cell->img_src != NULL)
        cell->image = gum_load_image(cell->img_src);

    int sz = MIN(cell->box.w, cell->box.h);
    int r_top_left = CSS_GET_UNIT(skin->r_top_left, skin->u_top_left, 96, 0.75, sz);
    int r_top_right = CSS_GET_UNIT(skin->r_top_right, skin->u_top_right, 96, 0.75, sz);
    int r_bottom_right = CSS_GET_UNIT(skin->r_bottom_right, skin->u_bottom_right, 96, 0.75, sz);
    int r_bottom_left = CSS_GET_UNIT(skin->r_bottom_left, skin->u_bottom_left, 96, 0.75, sz);

    if (skin->bgcolor >= MIN_ALPHA || skin->brcolor >= MIN_ALPHA || cell->image) {

        cairo_new_path(ctx);
        cairo_move_to(ctx, cell->box.x + r_top_left, cell->box.y);
        cairo_line_to(ctx, cell->box.x + cell->box.w - r_top_right, cell->box.y);
        cairo_arc(ctx, cell->box.x + cell->box.w - r_top_right, cell->box.y + r_top_right, r_top_right, -M_PI/2.0, 0.0);
        cairo_line_to(ctx, cell->box.x + cell->box.w, cell->box.y + cell->box.h - r_bottom_right);
        cairo_arc(ctx, cell->box.x + cell->box.w - r_bottom_right, cell->box.y + cell->box.h - r_bottom_right, r_bottom_right, 0.0, M_PI/2.0);
        cairo_line_to(ctx, cell->box.x + r_bottom_left, cell->box.y + cell->box.h);
        cairo_arc(ctx, cell->box.x + r_bottom_left, cell->box.y + cell->box.h - r_bottom_left, r_bottom_left, M_PI/2.0, M_PI);
        cairo_line_to(ctx, cell->box.x, cell->box.y + r_top_left);
        cairo_arc(ctx, cell->box.x + r_top_left, cell->box.y + r_top_left, r_top_left, M_PI, 3*M_PI/2.0);

        if (cell->image) {
            cairo_surface_t *img = (cairo_surface_t*)cell->image;
            int img_sz = MAX(cairo_image_surface_get_width(img), cairo_image_surface_get_height(img));
            double rt = (double)MAX(cell->box.w, cell->box.h) / (double)img_sz;
            cairo_save(ctx);
            cairo_translate(ctx, cell->box.x, cell->box.y);
            cairo_scale(ctx, rt, rt);
            cairo_set_source_surface(ctx, img, 0, 0);
            cairo_fill_preserve(ctx);
            cairo_restore(ctx);

        } else if (skin->grcolor >= MIN_ALPHA) {

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

            // cairo_pattern_add_color_stop_rgb(grad, 0.5 * h0, 0.0, 0.5, 0.5);
            // cairo_pattern_add_color_stop_rgb(grad, 0.7 * h0, 0.0, 0.5, 0.0);
            // cairo_pattern_add_color_stop_rgb(grad, 1.0 * h0, 0.5, 0.0, 0.0);
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

        cairo_select_font_face(ctx, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(ctx, 10.0);
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

void gum_text_size(const char *text, int *w, int *h)
{
    if (srf == NULL) {
        srf = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 10, 10);
        ctx = cairo_create(srf);
    }
    cairo_select_font_face(ctx, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(ctx, 10.0);

    cairo_text_extents_t extents;
    cairo_text_extents(ctx, text, &extents);
    *w = extents.width + extents.x_bearing;
    *h = extents.height + extents.y_bearing;
}


void gum_draw_scrolls(void *ctx, GUM_cell *cell)
{
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
        cairo_set_source_rgb(ctx, 66.0/255.0, 165.0/255.0, 245.0/255.0);
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
        cairo_set_source_rgb(ctx, 66.0/255.0, 165.0/255.0, 245.0/255.0);
        cairo_fill(ctx);
    }
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


int gum_event_poll(void *win, GUM_event *event, int timeout)
{
    int unicode;
    XEvent e;
    XMotionEvent *motion = (XMotionEvent *)&e;
    XButtonEvent *btn = (XButtonEvent *)&e;
    XKeyEvent *key = (XKeyEvent *)&e;
    XResizeRequestEvent *resz = (XResizeRequestEvent *)&e;

    XNextEvent(cairo_xlib_surface_get_display((cairo_surface_t *)win), &e);
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
        if (btn->button == 4 || btn->button == 5) {
            event->type = -1;
        } else {
            // printf("Event ButtonRelease <%x.%x>\n", btn->state, btn->button);
            event->type = GUM_EV_BTN_RELEASE;
            event->param0 = btn->button;
            event->param1 = btn->state;
        }
        break;
    case ResizeRequest:
        event->type = GUM_EV_RESIZE;
        cairo_xlib_surface_set_size((cairo_surface_t*)win, resz->width, resz->height);
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
    return 0;
}


int gum_check_events(void *win, int block)
{
    char keybuf[8];
    KeySym key;
    XEvent e;

    for (;;)
    {
        if (block || XPending(cairo_xlib_surface_get_display((cairo_surface_t*)win)))
            XNextEvent(cairo_xlib_surface_get_display((cairo_surface_t*)win), &e);
        else
            return 0;

        switch (e.type)
        {
        case ButtonPress:
            return -e.xbutton.button;
        case KeyPress:
            XLookupString(&e.xkey, keybuf, sizeof(keybuf), &key, NULL);
            return key;
        default:
            fprintf(stderr, "Dropping unhandled XEevent.type = %d.\n", e.type);
        }
    }
}