/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2018  <Fabien Bavent>
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
#ifndef _GUM_EVENTS_H
#define _GUM_EVENTS_H 1

#include <gum/core.h>
#include <kora/hmap.h>

#define CLICK_TIMEOUT  (200000LL) /* 200 ms, in us */


LIBAPI gum_window_t* gum_new_window(gum_cell_t* root);
LIBAPI void gum_close_window(gum_window_t* win);


void gum_push_event(gum_window_t* win, int type, size_t param);


typedef void(*GUM_event_handler)(gum_window_t* win, gum_cell_t *cell, int event, void *data);
typedef void(*GUM_async_worker)(gum_window_t* win, void* arg);

LIBAPI void gum_set_focus(gum_window_t* win, gum_cell_t *cell);
LIBAPI void gum_event_bind(gum_window_t* win, gum_cell_t* cell, int event, GUM_event_handler handler, void *data);
LIBAPI void gum_async_worker(gum_window_t* win, GUM_async_worker worker, GUM_async_worker callback, void *arg);
LIBAPI void gum_remove_context(gum_window_t* win);
LIBAPI void gum_show_context(gum_window_t* win, gum_cell_t* menu);

// LIBAPI void gum_refresh(GUM_event_manager* evm);
// void gum_do_visual(GUM_cell *cell, GUM_window *win, GUM_rect *inval);


void gum_start_paint(gum_window_t* win);
void gum_end_paint(gum_window_t* win);

LIBAPI void gum_dereference_cell(gum_window_t *win, gum_cell_t *cell);

void gum_update_mesure(gum_window_t* win);
void gum_update_layout(gum_window_t* win);
bool gum_update_visual(gum_window_t* win);
LIBAPI bool gum_update(gum_window_t* win);


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */


// Mouse motion event, call with absolute coords to windows
void gum_event_motion(gum_window_t* win, int x, int y);
// Mouse primary button press event
void gum_event_left_press(gum_window_t* win);
// Mouse primary button release event
void gum_event_left_release(gum_window_t* win);
// Mouse button press event
void gum_event_button_press(gum_window_t* win, int btn);
// Mouse button release event
void gum_event_button_release(gum_window_t* win, int btn);
// Mouse wheel event
void gum_event_wheel(gum_window_t* win, int move);
void gum_event_key_press(gum_window_t* win, int unicode, int key);
void gum_event_key_release(gum_window_t* win, int unicode, int key);


void gum_event_async(gum_window_t* win, void* async);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */


#define GUM_MAX_MCTX  8

typedef struct {
    int left, right, top, bottom;
} GUM_rect;

struct gum_window {
    //void* pixels;
    //int width;
    //int height;
    //int pitch;

    //int dpi_x;
    //int dpi_y;
    //float dsp_x;
    //float dsp_y;

    gum_cell_t* root;
    gum_cell_t* menus[GUM_MAX_MCTX];
    int menu_sp;

    hmap_t actions;

    gum_cell_t* over;
    gum_cell_t* down;
    gum_cell_t* focus;
    gum_cell_t* click;
    gum_cell_t* edit;
    gum_cell_t* grab;

    int mouse_x, mouse_y;
    int grab_x, grab_y;
    int click_cnt;
    int spec_btn;
    long long last_click;

    bool measure;
    gum_cell_t* layout;

    GUM_rect inval;

    GUM_gctx ctx;
    void* data;
};


typedef struct GUM_handler_record GUM_handler_record;

struct GUM_handler_record {
    GUM_event_handler handler;
    void *data;
};

struct GUM_event {
    int type;
    size_t param0;
    size_t param1;
};

enum {
    // From 0 to 127, those are GFX_EV_*.
    GUM_EV_DESTROY = 0, //
    GUM_EV_MOTION,
    GUM_EV_BTN_PRESS,
    GUM_EV_BTN_RELEASE,
    GUM_EV_KEY_PRESS,
    GUM_EV_KEY_RELEASE,
    GUM_EV_KEY_ENTER,
    GUM_EV_MOUSEWHEEL,
    GUM_EV_TIMER,
    GUM_EV_RESIZE,
    // GUM_EV_PAINT,
    GUM_EV_EXPOSE,
    GUM_EV_DELAY = 127,

    GUM_EV_OUT,
    GUM_EV_OVER,
    GUM_EV_DOWN,
    GUM_EV_UP,
    GUM_EV_FOCUS,
    GUM_EV_FOCUS_OUT,

    GUM_EV_CLICK,
    GUM_EV_DOUBLECLICK,
    GUM_EV_TRIPLECLICK,
    GUM_EV_RIGHTCLICK,

    GUM_EV_PREVIOUS,
    GUM_EV_NEXT,
    GUM_EV_WHEEL_UP,
    GUM_EV_WHEEL_DOWN,
    GUM_EV_WHEEL_CLICK,

    // GUM_EV_RESIZE,

    GUM_EV_TICK,
    GUM_EV_ASYNC,
};


#endif  /* _GUM_EVENTS_H */
