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
#include <kora/gum/events.h>
#include <kora/gum/cells.h>
#include <kora/hmap.h>
#include <kora/keys.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define GUM_MAX_MCTX  8

struct GUM_event_manager {
    int mouse_x, mouse_y;
    GUM_cell *root;
    GUM_cell *menus[GUM_MAX_MCTX];
    int menu_sp;
    GUM_window *win;

    HMP_map actions;

    GUM_cell *over;
    GUM_cell *down;
    GUM_cell *focus;
    GUM_cell *click;
    GUM_cell *edit;

    int click_cnt;
    int spec_btn;

    long long last_click;

    bool measure;
    GUM_cell *layout;
    GUM_gctx ctx;
    GUM_sideruler inval;
};

typedef struct GUM_async {
    void *res;
    void *arg;
    void *(*worker)(GUM_event_manager *, void*);
    void (*callback)(GUM_event_manager *, void*);
    GUM_event_manager *evm;
} GUM_async;

GUM_gctx *gum_graphic_context(GUM_cell *cell)
{
    return &gum_fetch_manager(cell)->ctx;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void gum_invalid_properties(GUM_cell *cell)
{
}

void gum_invalid_measure(GUM_cell *cell)
{
    GUM_event_manager *evm = gum_fetch_manager(cell);
    cell->state |= GUM_CELL_MEASURE;
    if (evm != NULL)
        evm->measure = true;
}
void gum_invalid_layout(GUM_cell *cell)
{
    GUM_event_manager *evm = gum_fetch_manager(cell);
    if (evm != NULL)
        evm->layout = gum_baseof(cell, evm->layout);
}

void gum_invalid_visual(GUM_cell *cell)
{
    GUM_cell *ancestors;
    int x = cell->box.x;
    int y = cell->box.y;

    for (ancestors = cell->parent; ancestors; ancestors = ancestors->parent) {
        x += ancestors->box.cx;
        y += ancestors->box.cy;
    }

    GUM_event_manager *evm = gum_fetch_manager(cell);
    if (evm != NULL) {
        // TODO -- For each region on invalid,
        // look for MIN px sum (union / split)
        // if ratio > 1.8 - Add a new rectangle
        // else merge
        if (evm->inval.right == 0) {
            evm->inval.left = x;
            evm->inval.right = x + cell->box.w;
        } else {
            evm->inval.left = MIN(x, evm->inval.left);
            evm->inval.right = MAX(x + cell->box.w, evm->inval.right);
        }
        if (evm->inval.bottom == 0) {
            evm->inval.top = y;
            evm->inval.bottom = y + cell->box.h;
        } else {
            evm->inval.top = MIN(y, evm->inval.top);
            evm->inval.bottom = MAX(y + cell->box.h, evm->inval.bottom);
        }
    }
}

void gum_invalid_all_(GUM_cell *cell)
{
    GUM_cell *child;
    for (child = cell->first; child; child = child->next)
        gum_invalid_all_(child);
    cell->state |= GUM_CELL_MEASURE;
}

void gum_invalid_all(GUM_cell *cell)
{
    GUM_event_manager *evm = gum_fetch_manager(cell);
    gum_invalid_all_(cell);
    if (evm != NULL)
        evm->measure = true;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

static void gum_emit_event(GUM_event_manager *evm, GUM_cell *cell, int event)
{
    char key[32];
    int lg = snprintf(key, 32, "%p]%4x", cell, event);
    GUM_EventHandler action = (GUM_EventHandler)hmp_get(&evm->actions, key, lg);
    if (action != NULL)
        action(evm, cell, event);

    lg = snprintf(key, 32, "%p]%4x", NULL, event);
    action = (GUM_EventHandler)hmp_get(&evm->actions, key, lg);
    if (action != NULL)
        action(evm, cell, event);
}

static void gum_cell_chstatus(GUM_event_manager *evm, GUM_cell *cell, int flags, int set, int event)
{
    if (cell == NULL)
        return;
    // fprintf(stderr, "chstatus %s (%x)\n", cell->id, flags);
    GUM_skin *skin = gum_skin(cell);
    if (set)
        cell->state |= flags;
    else
        cell->state &= ~flags;

    if (skin != gum_skin(cell))
        gum_invalid_visual(cell);
    else {
        GUM_cell *child;
        for (child = cell->first; child; child = child->next) {
            // TODO -- Is there a better way to ensure childs are changing too?
            if (child->state & GUM_CELL_SUBSTYLE)
                gum_invalid_visual(child);
        }
    }

    gum_emit_event(evm, cell, event);
}


void gum_event_bind(GUM_event_manager *evm, GUM_cell *cell, int event, GUM_EventHandler handler)
{
    char key[32];
    int lg = snprintf(key, 32, "%p]%4x", cell, event);
    hmp_put(&evm->actions, key, lg, handler);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void gum_set_focus(GUM_event_manager *evm, GUM_cell *cell)
{
    gum_cell_chstatus(evm, evm->focus, GUM_CELL_FOCUS, 0, GUM_EV_FOCUS_OUT);
    gum_cell_chstatus(evm, cell, GUM_CELL_FOCUS, 1, GUM_EV_FOCUS);
    evm->focus = cell;
    if (evm->edit != NULL) {
        evm->edit->text_pen = -1;
        evm->edit = NULL;
        // TODO redraw for cursor remove
    }
    if (cell && cell->state & GUM_CELL_EDITABLE) {
        printf("Enter edit test mode: %s\n", cell->text);
        evm->edit = cell;
        evm->edit->text_pen = 0;
        if (cell->text == NULL)
            cell->text = strdup("");
        // TODO set cursor in place -- Handle UTF-8
    }
}

static void gum_remove_context(GUM_event_manager *evm)
{
    int i;
    for (i = 0; i < evm->menu_sp; ++i)
        gum_invalid_visual(evm->menus[i]);
    evm->menu_sp = 0;
}

static void gum_event_motion(GUM_event_manager *evm, int x, int y)
{
    evm->mouse_x = x;
    evm->mouse_y = y;
    // TODO Cursor ?
    GUM_cell *target = gum_cell_hit(evm->root, x, y);
    if (evm->over != target) {
        evm->click_cnt = 0;
        evm->spec_btn = 0;
        gum_cell_chstatus(evm, evm->over, GUM_CELL_OVER, 0, GUM_EV_OUT);
        gum_cell_chstatus(evm, target, GUM_CELL_OVER, 1, GUM_EV_OVER);
        // if (evm->over)
            // fprintf(stderr, "Out %s\n", evm->over->id);
        // if (target)
            // fprintf(stderr, "Over %s\n", target->id);
        evm->over = target;
        /* If we live a down cell, it's like a release */
        if (target == evm->down) {
            gum_cell_chstatus(evm, evm->down, GUM_CELL_DOWN, 0, GUM_EV_UP);
            evm->down = NULL;
        }
    }
}

static void gum_event_left_press(GUM_event_manager *evm)
{
    GUM_cell *target = gum_cell_hit(evm->root, evm->mouse_x, evm->mouse_y);
    /* Change focus */
    if (evm->focus != target)
        gum_set_focus(evm, target);
    /* Cell is down */
    gum_cell_chstatus(evm, target, GUM_CELL_DOWN, 1, GUM_EV_DOWN);
    evm->down = target;
}

static void gum_event_left_release(GUM_event_manager *evm)
{
    GUM_cell *target = gum_cell_hit(evm->root, evm->mouse_x, evm->mouse_y);
    if (evm->menu_sp > 0)
        gum_remove_context(evm);
    /* Translate into click */
    if (target && evm->down == target) {
        long long now = gum_system_time();
        if (evm->click != target || evm->click_cnt >= 3 || now - evm->last_click > CLICK_TIMEOUT) {
            evm->click = target;
            evm->click_cnt = 0;
        }

        // printf("Click %d, %p\n", evm->click_cnt+1, target);
        gum_emit_event(evm, target, evm->click_cnt == 0 ? GUM_EV_CLICK :
                       (evm->click_cnt == 1 ? GUM_EV_DOUBLECLICK : GUM_EV_TRIPLECLICK));
        evm->click_cnt++;
    }
    /* Invalid down */
    if (evm->down) {
        gum_cell_chstatus(evm, evm->down, GUM_CELL_DOWN, 0, GUM_EV_UP);
        evm->down = NULL;
    }

    evm->last_click = gum_system_time();
}

static void gum_event_button_press(GUM_event_manager *evm, int btn)
{
    evm->click_cnt = 0;
    evm->spec_btn = btn;
}

static void gum_event_button_release(GUM_event_manager *evm, int btn)
{
    evm->click_cnt = 0;
    if (evm->menu_sp > 0)
        gum_remove_context(evm);
    if (evm->spec_btn == btn) {
        if (btn == 3) // Right button
            gum_emit_event(evm, evm->over, GUM_EV_RIGHTCLICK);
        else if (btn == 8) // Previous button
            gum_emit_event(evm, evm->focus, GUM_EV_PREVIOUS);
        else if (btn == 9) // Next button
            gum_emit_event(evm, evm->focus, GUM_EV_NEXT);
        else if (btn == 4) // Wheel up
            gum_emit_event(evm, evm->over, GUM_EV_WHEEL_UP);
        else if (btn == 5) // Wheel down
            gum_emit_event(evm, evm->over, GUM_EV_WHEEL_DOWN);
        else if (btn == 2) // Wheel button
            gum_emit_event(evm, evm->over, GUM_EV_WHEEL_CLICK);
    }
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

static void gum_event_wheel_up(GUM_event_manager *evm)
{
    GUM_cell *container = gum_cell_hit_ex(evm->root, evm->mouse_x, evm->mouse_y, GUM_CELL_OVERFLOW_X | GUM_CELL_OVERFLOW_Y);
    if (container != NULL) {
        // fprintf(stderr, "wheel_up %s\n", evm->over->id);
        if (container->state & GUM_CELL_OVERFLOW_Y) {
            container->box.sy = MAX(0, container->box.sy - 20);
            gum_invalid_visual(container);
        } else if (container->state & GUM_CELL_OVERFLOW_X) {
            container->box.sx = MAX(0, container->box.sx - 20);
            gum_invalid_visual(container);
        }
    }
}

static void gum_event_wheel_down(GUM_event_manager *evm)
{
    GUM_cell *container = gum_cell_hit_ex(evm->root, evm->mouse_x, evm->mouse_y, GUM_CELL_OVERFLOW_X | GUM_CELL_OVERFLOW_Y);
    if (container != NULL) {
        // fprintf(stderr, "wheel_down %s\n", evm->over->id);
        if (container->state & GUM_CELL_OVERFLOW_Y) {
            int st = container->box.ch_h - container->box.ch;
            // fprintf(stderr, "Down : %d - %d - %d\n", evm->over->box.sy, evm->over->box.minch, evm->over->box.ch);
            container->box.sy = MIN(st, container->box.sy + 20);
            gum_invalid_visual(container);
        } else if (container->state & GUM_CELL_OVERFLOW_X) {
            int st = container->box.ch_w - container->box.cw;
            container->box.sx = MIN(st, container->box.sx + 20);
            gum_invalid_visual(container);
        }
    }
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

static void gum_event_key_press(GUM_event_manager *evm, int unicode, int key)
{
    char buf[128] = { 0 };
    if (evm->edit == NULL)
        return;

    int lg = strlen(evm->edit->text);
    if (unicode > 0) {
        memcpy(buf, evm->edit->text, evm->edit->text_pen);
        if (unicode < 128) {
            buf[evm->edit->text_pen] = unicode;
            evm->edit->text_pen++;
        }
        memcpy(&buf[evm->edit->text_pen], &evm->edit->text[evm->edit->text_pen], lg - evm->edit->text_pen + 1);
    } else if (unicode == K_BACKSPACE) {
        if (evm->edit->text_pen == 0)
            return;

        memcpy(buf, evm->edit->text, evm->edit->text_pen - 1);
        memcpy(&buf[evm->edit->text_pen - 1], &evm->edit->text[evm->edit->text_pen], lg - evm->edit->text_pen);
        evm->edit->text_pen--;
    } else
        return;

    free(evm->edit->text);
    evm->edit->text = strdup(buf);
    gum_invalid_visual(evm->edit);
}

static void gum_event_key_release(GUM_event_manager *evm, int unicode, int key)
{

}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void gum_refresh(GUM_event_manager *evm)
{
    gum_resize_px(evm->root, evm->ctx.width, evm->ctx.height);
    gum_invalid_all(evm->root);
}

GUM_event_manager *gum_event_manager(GUM_cell *root, GUM_window *win)
{
    GUM_event_manager *evm = (GUM_event_manager *)calloc(1, sizeof(GUM_event_manager));
    gum_fill_context(win, &evm->ctx);
    evm->root = root;
    evm->win = win;
    evm->measure = true;

    root->manager = evm;
    hmp_init(&evm->actions, 16);

    gum_refresh(evm);
    return evm;
}

void gum_close_manager(GUM_event_manager *evm)
{
    // TODO - Free every binded actions
    hmp_destroy(&evm->actions, 0);
    free(evm);
}

void gum_handle_event(GUM_event_manager *evm, GUM_event *event)
{
    int i;
    GUM_async *async;
    // fprintf(stderr, "Event %d enter\n", event->type);
    switch (event->type) {
    case GUM_EV_RESIZE:
        // fprintf(stderr, "W %d - H %d\n", event->param0, event->param1);
        gum_resize_win(evm->win, event->param0, event->param1);
        evm->ctx.width = event->param0;
        evm->ctx.height = event->param1;
        gum_refresh(evm);
        break;

    case GUM_EV_MOTION:
        gum_event_motion(evm, event->param0, event->param1);
        break;

    case GUM_EV_BTN_PRESS:
        if (event->param0 == 1)
            gum_event_left_press(evm);
        else
            gum_event_button_press(evm, event->param0);
        break;

    case GUM_EV_WHEEL_UP:
        gum_event_wheel_up(evm);
        break;

    case GUM_EV_WHEEL_DOWN:
        gum_event_wheel_down(evm);
        break;

    case GUM_EV_BTN_RELEASE:
        if (event->param0 == 1)
            gum_event_left_release(evm);
        else
            gum_event_button_release(evm, event->param0);
        break;

    case GUM_EV_KEY_PRESS:
        gum_event_key_press(evm, event->param0, event->param1);
        break;
    case GUM_EV_KEY_RELEASE:
        gum_event_key_release(evm, event->param0, event->param1);
        break;
    case GUM_EV_EXPOSE:
        gum_start_paint(evm->win);
        gum_paint(evm->win, evm->root);
        for (i = 0; i < evm->menu_sp; ++i)
            gum_paint(evm->win, evm->menus[i]);
        gum_end_paint(evm->win);
        break;
    case GUM_EV_TICK:
        gum_emit_event(evm, NULL, GUM_EV_TICK);
        // TODO properties
        if (evm->measure) {
            evm->measure = false;
            printf("do measure\n");
            gum_do_measure(evm->root, &evm->ctx);
            for (i = 0; i < evm->menu_sp; ++i) {
                gum_do_measure(evm->menus[i], &evm->ctx);
                // TODO -- Each menu have an anchor point
                evm->menus[i]->box.w = evm->menus[i]->box.minw;
                evm->menus[i]->box.h = evm->menus[i]->box.minh;
                gum_do_layout(evm->menus[i], &evm->ctx);
                gum_invalid_visual(evm->menus[i]);
            }
        }
        if (evm->layout) {
            printf("do layout\n");
            GUM_cell *cell_layout = evm->layout;
            evm->layout = NULL;
            gum_do_layout(cell_layout, &evm->ctx);
        }
        if (evm->inval.left != evm->inval.right || evm->inval.top != evm->inval.bottom) {
            gum_do_visual(evm->root, evm->win, &evm->inval);
            // fprintf(stderr, "Visual\n", event->type);
            memset(&evm->inval, 0, sizeof(evm->inval));
        }
        break;
    case GUM_EV_ASYNC:
        async = (GUM_async *)(size_t)event->param0;
        async->callback(evm, async->res);
        break;
    }
    // fprintf(stderr, "Event %d leave\n", event->type);
}

void gum_event_loop(GUM_event_manager *evm)
{
    GUM_event event;
    for (;;) {
        if (gum_event_poll(evm->win, &event, -1) != 0)
            continue;

        if (event.type == GUM_EV_DESTROY)
            break;

        gum_handle_event(evm, &event);
    }
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void gum_show_context(GUM_event_manager *evm, GUM_cell *menu)
{
    gum_resize_px(menu, 0, 0);
    menu->rulerx.before = evm->mouse_x;
    menu->rulery.before = evm->mouse_y;
    if (menu->rulerx.before + menu->box.w > evm->ctx.width && menu->rulerx.before >= menu->box.w)
        menu->rulerx.before -= menu->box.w;
    if (menu->rulery.before + menu->box.h > evm->ctx.height) {
        if (menu->rulery.before >= menu->box.h)
            menu->rulery.before -= menu->box.h;
        else
            menu->rulery.before = evm->ctx.height - menu->box.h;
    }

    if (evm->menu_sp > GUM_MAX_MCTX) {
        gum_remove_context(evm);
        return;
    }

    evm->menus[evm->menu_sp] = menu;
    evm->menu_sp++;
    gum_refresh(evm);
}

static void gum_async_job(GUM_async *async)
{
    async->res = async->worker(async->evm, async->arg);
    gum_push_event(async->evm->win, GUM_EV_ASYNC, (size_t)async, 0);
}

void gum_async_worker(GUM_event_manager *evm, void *(*worker)(GUM_event_manager *, void *), void (*callback)(GUM_event_manager *, void *), void *arg)
{
    GUM_async *async = calloc(sizeof(GUM_async), 1);
    async->worker = worker;
    async->callback = callback;
    async->arg = arg;
    async->evm = evm;
    // thrd_create(gum_async_job, async);
    gum_async_job(async);
}

void gum_dereference_cell(GUM_event_manager *evm, GUM_cell *cell)
{
    if (evm->over == cell)
        evm->over = NULL;
    if (evm->down == cell)
        evm->down = NULL;
    if (evm->focus == cell)
        evm->focus = NULL;
    if (evm->click == cell)
        evm->click = NULL;
    if (evm->edit == cell)
        evm->edit = NULL;

    if (evm->layout == cell)
        evm->layout = cell->parent;
    if (cell->parent)
        gum_invalid_measure(cell->parent);
}
