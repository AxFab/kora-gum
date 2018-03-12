#include <kora/gum/events.h>
#include <kora/gum/display.h>
#include <kora/gum/rendering.h>
#include <kora/hmap.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct GUM_event_manager
{
  int mouse_x, mouse_y;
  GUM_cell *root;
  GUM_surface *win;

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

void gum_emit_event(GUM_event_manager *evm, GUM_cell *cell, int event)
{
  char key[32];
  snprintf(key, 32, "%p]%4x", cell, event);
  // GIL_EventHandler action = (GIL_EventHandler)hmp_get(&evm->actions, key);
  // if (action != NULL) {
  //   action(evm, cell, event);
  // } else if (event == GIL_CE_PREVIOUS || event == GIL_CE_NEXT) {
  //   snprintf(key, 32, "%p]%4x", NULL, event);
  //   action = (GIL_EventHandler)hmp_get(&evm->actions, key);
  //   if (action != NULL) {
  //     action(evm, cell, event);
  //   }
  // }
}


void gum_cell_chstatus(GUM_event_manager *evm, GUM_cell *cell, int flags, int set, int event)
{
    if (cell == NULL)
        return;
    fprintf(stderr, "chstatus %s (%x)\n", cell->id, flags);
    GUM_skin *skin = gum_skin(cell);
    if (set)
        cell->state |= flags;
    else
        cell->state &= ~flags;

    if (skin != gum_skin(cell))
        gum_invalid_cell(cell, evm->win);

    gum_emit_event(evm, cell, event);
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
        evm->over = target;
        /* If we live a down cell, it's like a release */
        if (evm->down) {
            fprintf(stderr, "Over %s\n", evm->down->id);
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

void gum_event_key_press(GUM_event_manager *evm, int unicode, int key)
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

        memcpy(buf, evm->edit->text, evm->edit->text_pen-1);
        memcpy(&buf[evm->edit->text_pen-1], &evm->edit->text[evm->edit->text_pen], lg - evm->edit->text_pen);
        evm->edit->text_pen--;
    } else {
        return;
    }

    free(evm->edit->text);
    evm->edit->text = strdup(buf);
    gum_invalid_cell(evm->edit, evm->win);
}

void gum_event_key_release(GUM_event_manager *evm, int unicode, int key)
{

}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */


GUM_event_manager *gum_event_manager(GUM_cell *root, GUM_surface *win)
{
    GUM_event_manager *evm = (GUM_event_manager*)calloc(1, sizeof(GUM_event_manager));
    evm->root = root;
    evm->win = win;
    hmp_init(&evm->actions, 16);

    win->cell = root; // TODO -- Remove all links

    gum_resize(root, win->width, win->height, win->xdpi, win->xdsp);
    // TODO -- Full update and paint (or invalidate)
    return evm;
}

void gum_handle_event(GUM_event_manager *evm, GUM_event *event)
{
    fprintf(stderr, "Event %d enter\n", event->type);
    switch (event->type) {
    case GUM_EV_EXPOSE:
        gum_paint(evm->win, evm->root);
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
    fprintf(stderr, "Event %d leave\n", event->type);
}

void gum_event_loop(GUM_event_manager *evm)
{
    GUM_event event;
    for (;;) {
        if (gum_event_poll(evm->win->info, &event, -1) != 0)
            continue;

        if (event.type == GUM_EV_DESTROY)
            break;

        gum_handle_event(evm, &event);
    }
}
