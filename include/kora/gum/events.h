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
#ifndef _KORA_GUM_EVENTS_H
#define _KORA_GUM_EVENTS_H 1

#include <kora/gum/core.h>

#define CLICK_TIMEOUT  (200000LL) /* 200 ms, in us */

LIBAPI GUM_event_manager *gum_event_manager(GUM_cell *root, GUM_window *win);
LIBAPI void gum_event_loop(GUM_event_manager *evm);

void gum_push_event(GUM_window *win, int type, size_t param0, size_t param1);

typedef void(*GUM_EventHandler)(GUM_event_manager *evm, GUM_cell *cell, int event);

LIBAPI void gum_set_focus(GUM_event_manager *evm, GUM_cell *cell);
LIBAPI void gum_refresh(GUM_event_manager *evm);
LIBAPI void gum_event_bind(GUM_event_manager *evm, GUM_cell *cell, int event, GUM_EventHandler handler);
LIBAPI void gum_async_worker(GUM_event_manager *evm, void *(*worker)(GUM_event_manager *, void *), void (*callback)(GUM_event_manager *, void *), void *arg);

LIBAPI void gum_show_context(GUM_event_manager *evm, GUM_cell *menu);
void gum_do_visual(GUM_cell *cell, GUM_window *win, GUM_sideruler *inval) ;

LIBAPI void gum_close_mananger(GUM_event_manager *evm);
LIBAPI void gum_dereference_cell(GUM_event_manager *evm, GUM_cell *cell);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

struct GUM_event {
    int type;
    int param0;
    int param1;
};

enum {
    GUM_EV_DESTROY,
    GUM_EV_EXPOSE,
    GUM_EV_KEY_PRESS,
    GUM_EV_KEY_RELEASE,
    GUM_EV_MOTION,
    GUM_EV_BTN_PRESS,
    GUM_EV_BTN_RELEASE,

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

    GUM_EV_RESIZE,

    GUM_EV_TICK,
    GUM_EV_ASYNC,
};


#endif  /* _KORA_GUM_EVENTS_H */
