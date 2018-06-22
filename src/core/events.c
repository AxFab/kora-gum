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
#include <kora/gum/events.h>
#include <kora/gum/cells.h>
#include <kora/hmap.h>
#include <kora/keys.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct GUM_event_manager {
    int mouse_x, mouse_y;
    int width, height;
    GUM_cell *root;
    GUM_window *win;

    HMP_map actions;

    GUM_cell *over;
    GUM_cell *down;
    GUM_cell *focus;
    GUM_cell *click;
    GUM_cell *edit;

    int click_cnt;
    int spec_btn;
};


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
        gum_invalid_cell(cell, evm->win);

    gum_emit_event(evm, cell, event);
}


void gum_event_bind(GUM_event_manager *evm, GUM_cell *cell, int event, GUM_EventHandler handler)
{
    char key[32];
    int lg = snprintf(key, 32, "%p]%4x", cell, event);
    hmp_put(&evm->actions, key, lg, handler);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

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
        //     fprintf(stderr, "Out %s\n", evm->over->id);
        // if (target)
        //     fprintf(stderr, "Over %s\n", target->id);
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
    if (evm->focus != target) {
        gum_cell_chstatus(evm, evm->focus, GUM_CELL_FOCUS, 0, GUM_EV_FOCUS_OUT);
        gum_cell_chstatus(evm, target, GUM_CELL_FOCUS, 1, GUM_EV_FOCUS);
        evm->focus = target;
        if (evm->edit != NULL) {
            evm->edit->text_pen = -1;
            evm->edit = NULL;
            // TODO Redraw for cursor remove !?
        }
    }
    /* Cell is down */
    gum_cell_chstatus(evm, target, GUM_CELL_DOWN, 1, GUM_EV_DOWN);
    evm->down = target;
    if (target && target->state & GUM_CELL_EDITABLE) {
        printf("Enter edit test mode :%s\n", target->text);
        evm->edit = target;
        evm->edit->text_pen = 0;
        if (target->text == NULL)
            target->text = strdup("");
        // TODO Set cursor in place ! -- Handle UTF-8.
    }
}

static void gum_event_left_release(GUM_event_manager *evm)
{
    GUM_cell *target = gum_cell_hit(evm->root, evm->mouse_x, evm->mouse_y);
    /* Translate into click */
    if (target && evm->down == target) {
        if (evm->click != target || evm->click_cnt >= 3 || 0/* TODO - TIME OFF */) {
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

}

static void gum_event_button_press(GUM_event_manager *evm, int btn)
{
    evm->click_cnt = 0;
    evm->spec_btn = btn;
}

static void gum_event_button_release(GUM_event_manager *evm, int btn)
{
    evm->click_cnt = 0;
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
            gum_invalid_cell(container, evm->win);
        } else if (container->state & GUM_CELL_OVERFLOW_X) {
            container->box.sx = MAX(0, container->box.sx - 20);
            gum_invalid_cell(container, evm->win);
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
            gum_invalid_cell(container, evm->win);
        } else if (container->state & GUM_CELL_OVERFLOW_X) {
            int st = container->box.ch_w - container->box.cw;
            container->box.sx = MIN(st, container->box.sx + 20);
            gum_invalid_cell(container, evm->win);
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
    gum_invalid_cell(evm->edit, evm->win);
}

static void gum_event_key_release(GUM_event_manager *evm, int unicode, int key)
{

}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void gum_refresh(GUM_event_manager *evm)
{
    gum_resize(evm->root, evm->width, evm->height, 96, 0.75);
    gum_invalid_cell(evm->root, evm->win);
}

GUM_event_manager *gum_event_manager(GUM_cell *root, GUM_window *win)
{
    GUM_event_manager *evm = (GUM_event_manager *)calloc(1, sizeof(GUM_event_manager));
    evm->root = root;
    evm->win = win;
    hmp_init(&evm->actions, 16);

    evm->width = 680;
    evm->height = 425;
    gum_resize(root, 680, 425, 96, 0.75);
    // gum_resize(root, win->width, win->height, win->xdpi, win->xdsp);
    // TODO -- Full update and paint (or invalidate)
    return evm;
}

void gum_handle_event(GUM_event_manager *evm, GUM_event *event)
{
    // fprintf(stderr, "Event %d enter\n", event->type);
    switch (event->type) {
    case GUM_EV_EXPOSE:
        gum_paint(evm->win, evm->root);
        break;

    case GUM_EV_RESIZE:
        // fprintf(stderr, "W %d - H %d\n", event->param0, event->param1);
        gum_resize_win(evm->win, event->param0, event->param1);
        evm->width = event->param0;
        evm->height = event->param1;
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
