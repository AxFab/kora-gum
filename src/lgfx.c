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
#include <gum/events.h>
#include <gum/cells.h>
#include <kora/css.h>
#include <kora/gfx.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#define _PwNano_ 1000000000



struct GUM_window {
    bool redraw;
    gfx_t *gfx;
    gfx_clip_t clip;
};

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void* memrchr(const void* buf, int byte, size_t len)
{
    const char* ptr = (const char*)buf + len;
    while (ptr-- > (const char*)buf)
        if (*ptr == byte)
            return (void*)ptr;
    return NULL;
}


long long gum_system_time()
{
    clock_t ticks = clock();
    ticks *= _PwNano_ / CLOCKS_PER_SEC;
    return ticks;
}


GUM_window *gum_create_surface(int width, int height)
{
    //Display *dsp;
    //Drawable da;
    //int screen;
    //cairo_surface_t *sfc;

    //if ((dsp = XOpenDisplay(NULL)) == NULL) {
    //    fprintf(stderr, "Unable to open X11 display\n");
    //    return NULL;
    //}
    //screen = DefaultScreen(dsp);
    //da = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp), 0, 0, width, height, 0, 0, 0);
    //XSelectInput(dsp, da,
    //             ButtonMotionMask |  ButtonPressMask | ButtonReleaseMask | // Mouse
    //             KeyPressMask | KeyReleaseMask | KeymapStateMask | // Keyboard
    //             EnterWindowMask | LeaveWindowMask | PointerMotionMask |
    //             // Button1MotionMask | Button2MotionMask | Button3MotionMask |
    //             // Button4MotionMask | Button5MotionMask |
    //             VisibilityChangeMask | StructureNotifyMask | ResizeRedirectMask |
    //             SubstructureNotifyMask | SubstructureRedirectMask | FocusChangeMask |
    //             PropertyChangeMask | ColormapChangeMask |
    //             ExposureMask);

    //// Make the window, frameless
    //Atom mwmHintsProperty = XInternAtom(dsp, "_MOTIF_WM_HINTS", 0);
    //struct MwmHints hints;
    //hints.flags = MWH_HINTS_DECORATIONS;
    //hints.decoration = 0;
    //XChangeProperty(dsp, da, mwmHintsProperty, mwmHintsProperty, 32,
    //                PropModeReplace, (unsigned char *)&hints, 5);

    //XMapWindow(dsp, da);

    //sfc = cairo_xlib_surface_create(dsp, da, DefaultVisual(dsp, screen), width, height);
    //cairo_xlib_surface_set_size(sfc, width, height);

    //timer_setup();

    GUM_window *win = (GUM_window *)malloc(sizeof(GUM_window));
    win->gfx = gfx_create_window(NULL, width, height, 0);
    win->redraw = true;
    //win->srf = sfc;
    //win->ctx = cairo_create(sfc);
    //__lastWin = win;
    return win;
}

void gum_destroy_surface(GUM_window *win)
{
    //cairo_destroy(win->ctx);
    //Display *dsp = cairo_xlib_surface_get_display(win->srf);
    //cairo_surface_destroy(win->srf);
    //XCloseDisplay(dsp);
    gfx_close(win->gfx);
    free(win);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

#define MIN_ALPHA 0x1000000
#define M_PI 3.141592653589793


void gum_draw_cell(GUM_window *win, GUM_cell *cell, bool top)
{
    //cairo_t *ctx = win->ctx;
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
    gfx_clip_t clip;
    clip.left = cell->box.x + win->clip.left;
    clip.top = cell->box.y + win->clip.top;
    clip.right = clip.left + cell->box.w;
    clip.bottom = clip.top + cell->box.h;

    if (skin->bgcolor >= MIN_ALPHA) {
        gfx_fill(win->gfx, skin->bgcolor, GFX_COPY_BLEND, &clip);
        // gfx_rect(win->gfx, cell->box.x + win->clip.x, cell->box.y + win->clip.y, cell->box.w, cell->box.h, skin->bgcolor);
    }
    //if (skin->bgcolor >= MIN_ALPHA || skin->brcolor >= MIN_ALPHA || cell->image) {

    //    gum_draw_path(ctx, cell, skin);

    if (cell->image) {
        gfx_blit(win->gfx, (gfx_t*)cell->image, GFX_COPY_BLEND, &clip);
        //        cairo_surface_t *img = (cairo_surface_t *)cell->image;
        //        int img_sz = MAX(cairo_image_surface_get_width(img), cairo_image_surface_get_height(img));
        //        double rt = (double)MAX(cell->box.w, cell->box.h) / (double)img_sz;
        //        cairo_save(ctx);
        //        cairo_translate(ctx, cell->box.x, cell->box.y);
        //        cairo_scale(ctx, rt, rt);
        //        cairo_set_source_surface(ctx, img, 0, 0);
        //        cairo_fill_preserve(ctx);
        //        cairo_restore(ctx);
    }
    //    } else if (skin->grcolor >= MIN_ALPHA) {

    //        cairo_pattern_t *grad = gum_build_gradient(cell, skin);
    //        cairo_set_source(ctx, grad);
    //        cairo_fill_preserve(ctx);

    //    } else if (skin->bgcolor >= MIN_ALPHA) {
    //        cairo_set_source_rgb(ctx, //0.8, 0.8, 0.8);
    //                             ((skin->bgcolor >> 16) & 255) / 255.0,
    //                             ((skin->bgcolor >> 8) & 255) / 255.0,
    //                             ((skin->bgcolor >> 0) & 255) / 255.0);
    //        cairo_fill_preserve(ctx);
    //    }

    //    if (skin->brcolor >= MIN_ALPHA) {
    //        cairo_set_line_width(ctx, 1.0);
    //        cairo_set_source_rgb(ctx, //0.1, 0.1, 0.1);
    //                             ((skin->brcolor >> 16) & 255) / 255.0,
    //                             ((skin->brcolor >> 8) & 255) / 255.0,
    //                             ((skin->brcolor >> 0) & 255) / 255.0);
    //        cairo_stroke(ctx);
    //    }
    //}

    //// fprintf(stderr, " -- %s\n", cell->text);
    //if (cell->text) {
    //    cairo_set_source_rgb(ctx, //0.8, 0.8, 0.8);
    //                         ((skin->txcolor >> 16) & 255) / 255.0,
    //                         ((skin->txcolor >> 8) & 255) / 255.0,
    //                         ((skin->txcolor >> 0) & 255) / 255.0);

    //    cairo_text_extents_t extents;

    //    const char *ffamily = skin->font_family ? skin->font_family : "Sans";
    //    cairo_select_font_face(ctx, ffamily, CAIRO_FONT_SLANT_NORMAL, 0);
    //    cairo_set_font_size(ctx, (float)skin->font_size);
    //    cairo_text_extents(ctx, cell->text, &extents);
    //    int tx = cell->box.x;
    //    int ty = cell->box.y;

    //    if (skin->align == 2)
    //        tx += cell->box.w - (extents.width + extents.x_bearing);
    //    else if (skin->align == 0)
    //        tx += cell->box.w / 2 - (extents.width / 2 + extents.x_bearing);

    //    if (skin->valign == 2)
    //        ty += cell->box.h - (extents.height + extents.y_bearing);
    //    else if (skin->valign == 0)
    //        ty += cell->box.h / 2 - (extents.height / 2 + extents.y_bearing);

    //    cairo_move_to(ctx, tx, ty);
    //    cairo_show_text(ctx, cell->text);
    //}
}



void gum_text_size(const char *text, int *w, int *h, GUM_skin *skin)
{
    //if (srf == NULL) {
    //    srf = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 10, 10);
    //    ctx = cairo_create(srf);
    //}
    //const char *ffamily = skin->font_family ? skin->font_family : "Sans";
    //fprintf(stderr, "Select Font %s %d\n", ffamily, skin->font_size);
    //cairo_select_font_face(ctx, ffamily, CAIRO_FONT_SLANT_NORMAL, 0);
    //cairo_set_font_size(ctx, (float)skin->font_size);

    //cairo_text_extents_t extents;
    //cairo_text_extents(ctx, text, &extents);
    //*w = extents.width + extents.x_bearing;
    //*h = extents.height + extents.y_bearing;
}


void gum_draw_scrolls(GUM_window *win, GUM_cell *cell)
{
    //cairo_t *ctx = win->ctx;
    //if (cell->state & GUM_CELL_OVERFLOW_X) {
    //    cairo_new_path(ctx);
    //    cairo_rectangle(ctx,
    //                    cell->box.x,
    //                    cell->box.y + cell->box.h - 7,
    //                    cell->box.w - 7, 7);
    //    cairo_set_source_rgb(ctx, 0.7, 0.7, 0.7);
    //    cairo_fill(ctx);

    //    int sz = cell->box.cw * (cell->box.w - 7) / cell->box.ch_w;
    //    int st = cell->box.sx * (cell->box.w - 7) / cell->box.ch_w;

    //    cairo_new_path(ctx);
    //    cairo_rectangle(ctx,
    //                    cell->box.x + st,
    //                    cell->box.y + cell->box.h - 7,
    //                    sz, 7);
    //    cairo_set_source_rgb(ctx, 66.0 / 255.0, 165.0 / 255.0, 245.0 / 255.0);
    //    cairo_fill(ctx);
    //}

    //if (cell->state & GUM_CELL_OVERFLOW_Y) {
    //    cairo_new_path(ctx);
    //    cairo_rectangle(ctx,
    //                    cell->box.x + cell->box.w - 7,
    //                    cell->box.y, 7, cell->box.h - 7);
    //    cairo_set_source_rgb(ctx, 0.7, 0.7, 0.7);
    //    cairo_fill(ctx);

    //    int sz = cell->box.ch * (cell->box.h - 7) / cell->box.ch_h;
    //    int st = cell->box.sy * (cell->box.h - 7) / cell->box.ch_h;

    //    cairo_new_path(ctx);
    //    cairo_rectangle(ctx,
    //                    cell->box.x + cell->box.w - 7,
    //                    cell->box.y + st, 7, sz);
    //    cairo_set_source_rgb(ctx, 66.0 / 255.0, 165.0 / 255.0, 245.0 / 255.0);
    //    cairo_fill(ctx);
    //}
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

#include <kora/keys.h>


void gum_fill_context(GUM_window *win, GUM_gctx *ctx)
{
    ctx->dpi_x = 96;
    ctx->dpi_y = 96;
    ctx->dsp_x = 2; //0.75;
    ctx->dsp_y = 2; //0.75;
    ctx->width = win->gfx->width;
    ctx->height = win->gfx->height;
}

#define GET_X_LPARAM(s)   ((int)(short)((int32_t)(s) & 0xffff))
#define GET_Y_LPARAM(s)   ((int)(short)(((int32_t)(s) >> 16) & 0xffff))

int gum_event_poll(GUM_window *win, GUM_event *event, int timeout)
{
    gfx_msg_t msg;
    gfx_seat_t seat;
    memset(&seat, 0, sizeof(seat));
    do {
        gfx_poll(win->gfx, &msg);
        gfx_handle(win->gfx, &msg, &seat);
        event->type = -1;
        switch (msg.message) {
        case GFX_EV_QUIT:
            event->type = GUM_EV_DESTROY;
            break;
        case GFX_EV_MOUSEMOVE:
            event->type = GUM_EV_MOTION;
            event->param0 = GET_X_LPARAM(msg.param1);
            event->param1 = GET_Y_LPARAM(msg.param1);
            // printf("Mouse at %dx%d\n", event->param0, event->param1);
            break;
        //case GFX_EV_BUTTONDOWN:
        //    break;
        //case GFX_EV_BUTTONUP:
        //    break;
        case GFX_EV_MOUSEWHEEL:
            break;
        case GFX_EV_KEYDOWN:
            event->type = GUM_EV_KEY_PRESS;
            break;
        case GFX_EV_KEYUP:
            event->type = GUM_EV_KEY_RELEASE;
            break;
         case GFX_EV_TIMER:
            event->type = GUM_EV_TICK;
            break;
        case GFX_EV_RESIZE:
            break;
        case GFX_EV_PAINT:
            event->type = GUM_EV_EXPOSE;
            break;
        default:
            break;
        }
    } while (event->type == -1);
 
    return 0;
}


void gum_start_paint(GUM_window *win)
{
    if (win->gfx->pixels == NULL)
        gfx_map(win->gfx);
    //cairo_push_group(win->ctx);
    //cairo_set_source_rgb(win->ctx, 1, 1, 1);
    //cairo_paint(win->ctx);
    //cairo_reset_clip(win->ctx);
    // cairo_translate(win->ctx, x, y);
    win->clip.left = 0;
    win->clip.top = 0;
}

void gum_end_paint(GUM_window *win)
{
    //cairo_pop_group_to_source(win->ctx);
    //cairo_paint(win->ctx);
    //cairo_surface_flush(win->srf);
    win->redraw = false;
    gfx_flip(win->gfx);
}

void gum_push_clip(GUM_window *win, GUM_box *box)
{
    //cairo_save(win->ctx);
    //cairo_new_path(win->ctx);
    //cairo_rectangle(win->ctx, box->cx, box->cy, box->cw, box->ch);
    //cairo_clip(win->ctx);
    //cairo_translate(win->ctx, box->cx - box->sx, box->cy - box->sy);
    win->clip.left += box->cx - box->sx;
    win->clip.top += box->cy - box->sy;
}

void gum_pop_clip(GUM_window *win, GUM_box *box, GUM_box *prev)
{
    win->clip.left -= box->cx - box->sx;
    win->clip.top -= box->cy - box->sy;
    //cairo_translate(win->ctx, box->sx - box->cx, box->sy - box->cy);
    //cairo_restore(win->ctx);
}

void gum_resize_win(GUM_window *win, int width, int height)
{
    gfx_unmap(win->gfx);
    // gfx_resize(win->gfx, width, height);
    //cairo_xlib_surface_set_size(win->srf, width, height);
}


void *gum_load_image(const char *name)
{
    gfx_t* img = gfx_load_image(name);
    //cairo_surface_t *img = cairo_image_surface_create_from_png(name);
    //if (cairo_surface_status(img) != 0)
    //    return NULL;
    return img;
}

void gum_do_visual(GUM_cell *cell, GUM_window *win, GUM_sideruler *inval)
{
    win->redraw = true;
    gfx_invalid(win->gfx);
    // TODO -- Push Exposure !!
    //XEvent e;
    //memset(&e, 0, sizeof(e));
    //XExposeEvent *ee = (XExposeEvent *)&e;
    //ee->type = Expose;
    //ee->x = 0;
    //ee->y = 0;
    //ee->width = cairo_xlib_surface_get_width(__lastWin->srf);
    //ee->height = cairo_xlib_surface_get_height(__lastWin->srf);

    //Display *dsp = cairo_xlib_surface_get_display(__lastWin->srf);
    //Window w = (Window)cairo_xlib_surface_get_drawable(__lastWin->srf);
    //XSendEvent(dsp, w, False, ExposureMask, &e);
}

void gum_push_event(GUM_window *win, int type, size_t param0, size_t param1, void *data)
{
    // TODO -- Write on GFX
    //XEvent e;
    //memset(&e, 0, sizeof(e));
    //XClientMessageEvent *em = (XClientMessageEvent *)&e;
    //em->type = ClientMessage;
    //em->format = 32;
    //em->data.l[0] = type;
    //em->data.l[1] = param0;

    //// ee->width = cairo_xlib_surface_get_width(__lastWin->srf);
    //// ee->height = cairo_xlib_surface_get_height(__lastWin->srf);

    //Display *dsp = cairo_xlib_surface_get_display(__lastWin->srf);
    //Window w = (Window)cairo_xlib_surface_get_drawable(__lastWin->srf);
    //XSendEvent(dsp, w, False, ExposureMask, &e);
}

