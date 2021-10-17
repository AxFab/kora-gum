/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2021  <Fabien Bavent>
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
#include <gum/xml.h>
#include "../win.h"
#include "../hmap.h"
#include "../mcrs.h"
#include <keycodes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <threads.h>


typedef struct GUM_async {
    void *arg;
    GUM_async_worker worker;
    GUM_async_worker callback;
    gum_window_t* win;
} GUM_async;



LIBAPI gum_window_t* gum_new_window(gum_cell_t *root)
{
    gum_window_t* win = (gum_window_t*)calloc(1, sizeof(gum_window_t));
    win->ctx.dpi_x = 96;
    win->ctx.dpi_y = 96;
    win->ctx.dsp_x = 0.75;
    win->ctx.dsp_y = 0.75;
    win->ctx.width = 680;
    win->ctx.height = 425;
    hmp_init(&win->actions, 16);

    win->root = root;
    win->layout = root;
    root->win = win;
    win->measure = true;
    return win;
}

LIBAPI void gum_close_window(gum_window_t* win)
{
    // TODO - Free every binded actions
    hmp_destroy(&win->actions);
    free(win);
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

// Trigger an UI event on a cell
static void gum_emit_event(gum_window_t* win, gum_cell_t*cell, int event)
{
    char key[32];
    int lg = snprintf(key, 32, "%p]%4x", cell, event);
    GUM_handler_record *action = (void *)hmp_get(&win->actions, key, lg);
    if (action != NULL)
        action->handler(win, cell, event, action->data);

    lg = snprintf(key, 32, "%p]%4x", NULL, event);
    action = (void *)hmp_get(&win->actions, key, lg);
    if (action != NULL)
        action->handler(win, cell, event, action->data);
}

// Change the status of a cell
static void gum_cell_chstatus(gum_window_t* win, gum_cell_t *cell, int flags, int set, int event)
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
        gum_invalid_visual(win, cell);
    else {
        gum_cell_t*child;
        for (child = cell->first; child; child = child->next) {
            // TODO -- Is there a better way to ensure childs are changing too?
            if (child->state & GUM_CELL_SUBSTYLE)
                gum_invalid_visual(win, child);
        }
    }

    gum_emit_event(win, cell, event);
}

// Register an handler to a cell event (erase the previous one)
void gum_event_bind(gum_window_t* win, gum_cell_t*cell, int event, GUM_event_handler handler, void *data)
{
    char key[32];
    int lg = snprintf(key, 32, "%p]%4x", cell, event);
    GUM_handler_record *record = malloc(sizeof(GUM_handler_record));
    record->handler = handler;
    record->data = data;
    hmp_put(&win->actions, key, lg, record);
}

// Give focus to a new cell
void gum_set_focus(gum_window_t* win, gum_cell_t* cell)
{
    gum_cell_chstatus(win, win->focus, GUM_CELL_FOCUS, 0, GUM_EV_FOCUS_OUT);
    gum_cell_chstatus(win, cell, GUM_CELL_FOCUS, 1, GUM_EV_FOCUS);
    win->focus = cell;
    if (win->edit != NULL) {
        win->edit->text_pen = -1;
        win->edit = NULL;
        // TODO redraw for cursor remove
    }
    if (cell && cell->state & GUM_CELL_EDITABLE) {
        printf("Enter edit test mode: %s\n", cell->text);
        win->edit = cell;
        win->edit->text_pen = 0;
        if (cell->text == NULL)
            cell->text = strdup("");
        // TODO set cursor in place -- Handle UTF-8
    }
}

// Remove all opened context menu
LIBAPI void gum_remove_context(gum_window_t* win)
{
    int i;
    for (i = 0; i < win->menu_sp; ++i)
        gum_invalid_visual(win, win->menus[i]);
    win->menu_sp = 0;
}

LIBAPI void gum_show_context(gum_window_t* win, gum_cell_t* menu)
{
    int width = win->ctx.width;
    int height = win->ctx.height; 
    // TODO -- Might use another or larger window !?
    gum_resize_px(menu, 0, 0);
    menu->rulerx.before.len = win->mouse_x;
    menu->rulery.before.len = win->mouse_y;
    if (menu->rulerx.before.len + menu->box.w > width && menu->rulerx.before.len >= menu->box.w)
        menu->rulerx.before.len -= menu->box.w;
    if (menu->rulery.before.len + menu->box.h > height) {
        if (menu->rulery.before.len >= menu->box.h)
            menu->rulery.before.len -= menu->box.h;
        else
            menu->rulery.before.len = height - menu->box.h;
    }

    if (win->menu_sp > GUM_MAX_MCTX) {
        gum_remove_context(win);
        return;
    }

    win->menus[win->menu_sp] = menu;
    win->menu_sp++;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

// Mouse motion event, call with absolute coords to windows
void gum_event_motion(gum_window_t* win, int x, int y)
{
    win->mouse_x = x;
    win->mouse_y = y;
    // TODO Cursor ?
    gum_cell_t*target = gum_cell_hit(win->root, x, y);
    if (win->over != target) {
        win->click_cnt = 0;
        win->spec_btn = 0;
        gum_cell_chstatus(win, win->over, GUM_CELL_OVER, 0, GUM_EV_OUT);
        gum_cell_chstatus(win, target, GUM_CELL_OVER, 1, GUM_EV_OVER);
        // if (evm->over)
        // fprintf(stderr, "Out %s\n", evm->over->id);
        // if (target)
        // fprintf(stderr, "Over %s\n", target->id);
        win->over = target;
        /* If we live a down cell, it's like a release */
        if (target == win->down) {
            gum_cell_chstatus(win, win->down, GUM_CELL_DOWN, 0, GUM_EV_UP);
            win->down = NULL;
        }
    }
    if (win->grab != NULL) {
        // drop request !?
        gum_invalid_visual(win, win->grab);
        win->grab->box.x -= win->grab->box.dx;
        win->grab->box.y -= win->grab->box.dy;
        win->grab->box.dx = win->mouse_x - win->grab_x;
        win->grab->box.dy = win->mouse_y - win->grab_y;

        /* BEGIN: drag limit */
        if (win->grab->box.dx < 0)
            win->grab->box.dx = 0;
        else if (win->grab->box.dx > win->grab->parent->box.cw - win->grab->box.w)
            win->grab->box.dx = win->grab->parent->box.cw - win->grab->box.w;
        win->grab->box.dy = 0;
        /* END */

        win->grab->box.x += win->grab->box.dx;
        win->grab->box.y += win->grab->box.dy;
        gum_invalid_visual(win, win->grab);
    }
}

// Mouse primary button press event
void gum_event_left_press(gum_window_t* win)
{
    gum_cell_t*target = gum_cell_hit(win->root, win->mouse_x, win->mouse_y);
    /* Change focus */
    if (win->focus != target)
        gum_set_focus(win, target);
    /* Cell is down */
    gum_cell_chstatus(win, target, GUM_CELL_DOWN, 1, GUM_EV_DOWN);
    win->down = target;
    if (target && target->state & GUM_CELL_DRAGABLE) {
        win->grab = target;
        win->grab_x = win->mouse_x - win->grab->box.dx;
        win->grab_y = win->mouse_y - win->grab->box.dy;
    }
}

// Mouse primary button release event
void gum_event_left_release(gum_window_t* win)
{
    gum_cell_t*target = gum_cell_hit(win->root, win->mouse_x, win->mouse_y);
    if (win->menu_sp > 0)
        gum_remove_context(win);
    /* if cell grabbed, drop-it */
    if (win->grab)
        win->grab = NULL;
    /* Translate into click */
    if (target && win->down == target) {
        long long now = gum_system_time();
        if (win->click != target || win->click_cnt >= 3 || now - win->last_click > CLICK_TIMEOUT) {
            win->click = target;
            win->click_cnt = 0;
        }

        // printf("Click %d, %p\n", evm->click_cnt+1, target);
        gum_emit_event(win, target, win->click_cnt == 0 ? GUM_EV_CLICK :
                       (win->click_cnt == 1 ? GUM_EV_DOUBLECLICK : GUM_EV_TRIPLECLICK));
        win->click_cnt++;
    }
    /* Invalid down */
    if (win->down) {
        gum_cell_chstatus(win, win->down, GUM_CELL_DOWN, 0, GUM_EV_UP);
        win->down = NULL;
    }

    win->last_click = gum_system_time();
}

// Mouse button press event
void gum_event_button_press(gum_window_t* win, int btn)
{
    win->click_cnt = 0;
    win->spec_btn = btn;
}

// Mouse button release event
void gum_event_button_release(gum_window_t* win, int btn)
{
    win->click_cnt = 0;
    if (win->menu_sp > 0)
        gum_remove_context(win);
    if (win->spec_btn == btn) {
        if (btn == 3) // Right button
            gum_emit_event(win, win->over, GUM_EV_RIGHTCLICK);
        else if (btn == 8) // Previous button
            gum_emit_event(win, win->focus, GUM_EV_PREVIOUS);
        else if (btn == 9) // Next button
            gum_emit_event(win, win->focus, GUM_EV_NEXT);
        else if (btn == 4) // Wheel up
            gum_emit_event(win, win->over, GUM_EV_WHEEL_UP);
        else if (btn == 5) // Wheel down
            gum_emit_event(win, win->over, GUM_EV_WHEEL_DOWN);
        else if (btn == 2) // Wheel button
            gum_emit_event(win, win->over, GUM_EV_WHEEL_CLICK);
    }
}

// Mouse wheel event
void gum_event_wheel(gum_window_t* win, int move)
{
    gum_cell_t* container = gum_cell_hit_ex(win->root, win->mouse_x, win->mouse_y, GUM_CELL_OVERFLOW_X | GUM_CELL_OVERFLOW_Y);
    if (container != NULL) {
        if (container->state & GUM_CELL_OVERFLOW_Y) {
            int st = container->box.ch_h - container->box.ch;
            container->box.sy = MAX(0, MIN(st, container->box.sy + move));
            gum_invalid_visual(win, container);
        }
        else if (container->state & GUM_CELL_OVERFLOW_X) {
            int st = container->box.ch_w - container->box.cw;
            container->box.sx = MAX(0, MIN(st, container->box.sx + move));
            gum_invalid_visual(win, container);
        }
    }
}

void gum_event_key_press(gum_window_t* win, int unicode, int key)
{
    if (win->edit == NULL)
        return;
    if (unicode <= 0)
        return;

    int len = strlen(win->edit->text);
    int cursor = win->edit->text_pen;
    if (unicode == KEY_BACKSPACE) {
        if (win->edit->text_pen == 0)
            return;

        cursor--;
        while ((win->edit->text[cursor] & 0xc0) == 0x80)
            cursor--;

        memmove(&win->edit->text[cursor], &win->edit->text[win->edit->text_pen], len - win->edit->text_pen + 1);
        win->edit->text_pen = cursor;
    } else {
        char *buf = malloc(len + 8);
        memcpy(buf, win->edit->text, win->edit->text_pen);
        cursor += uctomb(&buf[win->edit->text_pen], unicode);
        memcpy(&buf[cursor], &win->edit->text[win->edit->text_pen], len - win->edit->text_pen + 1);
        win->edit->text_pen = cursor;
        free(win->edit->text);
        win->edit->text = strdup(buf);
        free(buf);
    }

    gum_invalid_measure(win, win->edit);
    gum_invalid_visual(win, win->edit);
}

void gum_event_key_release(gum_window_t* win, int unicode, int key)
{

}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

static void gum_async_job(GUM_async* async)
{
    async->worker(async->win, async->arg);
    gum_push_event(async->win, GUM_EV_ASYNC, (size_t)async);
}

void gum_event_async(gum_window_t* win, void* data)
{
    GUM_async* async = data;
    async->callback(async->win, async->arg);
    free(async);
}

LIBAPI void gum_async_worker(gum_window_t* win, GUM_async_worker worker, GUM_async_worker callback, void* arg)
{
    GUM_async* async = calloc(sizeof(GUM_async), 1);
    async->worker = worker;
    async->callback = callback;
    async->arg = arg;
    async->win = win;

    thrd_t thrd;
    thrd_create(&thrd, gum_async_job, async);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void gum_dereference_cell(gum_window_t *win, gum_cell_t *cell)
{
    if (win->over == cell)
        win->over = NULL;
    if (win->down == cell)
        win->down = NULL;
    if (win->focus == cell)
        win->focus = NULL;
    if (win->click == cell)
        win->click = NULL;
    if (win->edit == cell)
        win->edit = NULL;

    if (win->layout == cell) //  TODO if any parent is layout
        win->layout = cell->parent;
    if (cell->parent)
        gum_invalid_measure(win, cell->parent);
}





/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */


void gum_invalid_measure(gum_window_t* win, gum_cell_t* cell)
{
    cell->state |= GUM_CELL_MEASURE;
    if (win != NULL)
        win->measure = true;
}

void gum_invalid_layout(gum_window_t* win, gum_cell_t* cell)
{
    if (win != NULL)
        win->layout = gum_baseof(cell, win->layout);
}

void gum_invalid_visual(gum_window_t* win, gum_cell_t* cell)
{
    gum_cell_t* ancestors;
    int x = cell->box.x;
    int y = cell->box.y;

    for (ancestors = cell->parent; ancestors; ancestors = ancestors->parent) {
        x += ancestors->box.cx;
        y += ancestors->box.cy;
    }

    if (win != NULL) {
        // TODO -- For each region on invalid,
        // look for MIN px sum (union / split)
        // if ratio > 1.8 - Add a new rectangle
        // else merge
        if (win->inval.right == 0) {
            win->inval.left = x;
            win->inval.right = x + cell->box.w;
        }
        else {
            win->inval.left = MIN(x, win->inval.left);
            win->inval.right = MAX(x + cell->box.w, win->inval.right);
        }
        if (win->inval.bottom == 0) {
            win->inval.top = y;
            win->inval.bottom = y + cell->box.h;
        }
        else {
            win->inval.top = MIN(y, win->inval.top);
            win->inval.bottom = MAX(y + cell->box.h, win->inval.bottom);
        }
    }
}

void gum_invalid_all(gum_window_t* win, gum_cell_t* cell)
{
    gum_cell_t* child;
    for (child = cell->first; child; child = child->next)
        gum_invalid_all(win, child);
    cell->state |= GUM_CELL_MEASURE;
    win->measure = true;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void gum_update_mesure(gum_window_t* win)
{
    if (!win->measure)
        return;
    win->measure = false;
    gum_do_measure(win->root, &win->ctx);
    for (int i = 0; i < win->menu_sp; ++i) {
        gum_do_measure(win->menus[i], &win->ctx);
        // TODO -- Each menu have an anchor point
        win->menus[i]->box.w = win->menus[i]->box.minw;
        win->menus[i]->box.h = win->menus[i]->box.minh;
        gum_do_layout(win->menus[i], &win->ctx);
        gum_invalid_visual(win, win->menus[i]);
    }
}

void gum_update_layout(gum_window_t* win)
{
    if (!win->layout)
        return;

    gum_cell_t* cell_layout = win->layout;
    win->layout = NULL;
    if (cell_layout == win->root) {
        cell_layout->box.x = 0;
        cell_layout->box.y = 0;
        cell_layout->box.w = win->ctx.width;
        cell_layout->box.h = win->ctx.height;
    }
    gum_do_layout(cell_layout, &win->ctx);
}

bool gum_update_visual(gum_window_t* win)
{
    if (win->inval.left != win->inval.right || win->inval.top != win->inval.bottom) {
        // fprintf(stderr, "Paint <%d, %d, %d, %d>\n", win->inval.left, win->inval.top, win->inval.right - win->inval.left, win->inval.bottom - win->inval.top);
        gum_start_paint(win);
        gum_paint(win, win->root);
       for (int i = 0; i < win->menu_sp; ++i)
           gum_paint(win, win->menus[i]);
        gum_end_paint(win);
        memset(&win->inval, 0, sizeof(win->inval));
        return true;
    }
    return false;
}

LIBAPI bool gum_update(gum_window_t* win)
{
    gum_update_mesure(win);
    gum_update_layout(win);
    return gum_update_visual(win);
}
