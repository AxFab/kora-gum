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
#include <kora/gum/cells.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cairo/cairo.h>

GUM_cell *gum_cell_hit_ex(GUM_cell *cell, int x, int y, int mask)
{
    if (x < cell->box.x || y < cell->box.y ||
            x >= cell->box.x + cell->box.w || y >= cell->box.y + cell->box.h)
        return NULL;

    GUM_cell *child, *hit;
    for (child = cell->last; child; child = child->previous) {
        hit = gum_cell_hit_ex(child,
            x - cell->box.cx + cell->box.sx,
            y - cell->box.cy + cell->box.sy, mask);
        if (hit)
            return hit;
    }

    return cell->state & mask ? cell : NULL;
}

GUM_cell *gum_cell_hit(GUM_cell *cell, int x, int y)
{
    return gum_cell_hit_ex(cell, x, y, GUM_CELL_SOLID);
}


void gum_paint(void *ctx, GUM_cell *cell)
{
    // void *ctx = gum_context(win);
    cairo_reset_clip(ctx);
    // int x = 0, y = 0;
    for (;;) {
        // fprintf(stderr, "Paint %s [%d, %d, %d, %d]\n",
        //     cell->id, cell->box.x, cell->box.y, cell->box.w, cell->box.h);
        gum_draw_cell(ctx, cell);
        if (cell->first) {
            // TODO - prune if cell is outside drawing clip
            cairo_save(ctx);
            cairo_new_path(ctx);
            cairo_rectangle(ctx, cell->box.cx, cell->box.cy, cell->box.cw, cell->box.ch);
            cairo_clip(ctx);
            // x += cell->box.cx - cell->box.sx;
            // y += cell->box.cy - cell->box.sy;
            cairo_translate(ctx, cell->box.cx - cell->box.sx, cell->box.cy - cell->box.sy);
            cell = cell->first;
            continue;
        }

        while (!cell->next) {
            cell = cell->parent;
            if (cell == NULL) {
                // gum_complete(win, ctx);
                return;
            }

            // x -= cell->box.cx - cell->box.sx;
            // y -= cell->box.cy - cell->box.sy;
            cairo_translate(ctx, cell->box.sx - cell->box.cx, cell->box.sy - cell->box.cy);
            cairo_restore(ctx);
            if (cell->state & (GUM_CELL_OVERFLOW_X | GUM_CELL_OVERFLOW_Y)) { // TODO
                gum_draw_scrolls(ctx, cell);
            }
        }
        if (cell) {
            cell = cell->next;
        }
    }
}

GUM_skin *gum_skin(GUM_cell *cell)
{
    GUM_cell *ref = cell;
    while (ref->state & GUM_CELL_SUBSTYLE)
        ref = ref->parent;

    if (ref->state & GUM_CELL_DOWN && cell->skin_down)
        return cell->skin_down;
    else if (ref->state & GUM_CELL_OVER && cell->skin_over)
        return cell->skin_over;

    return cell->skin;
}

void gum_invalid_cell(GUM_cell *cell, void *win)
{
    // TODO -- Same as get by Id, but start by Last instead of Frist
    int x = cell->box.x;
    int y = cell->box.y;
    GUM_cell *ancestors;
    for (ancestors = cell->parent; ancestors; ancestors = ancestors->parent) {
        x += ancestors->box.cx;
        y += ancestors->box.cy;
    }

    gum_invalid_surface(win, x, y, cell->box.w, cell->box.h);
}


GUM_cell *gum_get_by_id(GUM_cell *cell, const char *id)
{
    while (cell) {
        if (cell->id && !strcmp(id, cell->id))
            return cell;
        if (cell->first) {
            cell = cell->first;
            continue;
        }

        while (!cell->next) {
            cell = cell->parent;
        }
        if (cell) {
            cell = cell->next;
        }
    }
    return NULL;
}

void gum_cell_dettach(GUM_cell *cell)
{
    if (cell->previous)
        cell->previous->next = cell->next;
    else
        cell->parent->first = cell->next;

    if (cell->next)
        cell->next->previous = cell->previous;
    else
        cell->parent->last = cell->previous;
    cell->previous = NULL;
    cell->next = NULL;
    cell->parent = NULL;
}

void gum_cell_destroy(GUM_cell *cell)
{

}

void gum_cell_destroy_children(GUM_cell *cell)
{
    while (cell->first) {
        GUM_cell *child = cell->first;
        cell->first = child->next;
        gum_cell_destroy(child);
    }
    cell->last = NULL;
}

void gum_cell_pushback(GUM_cell *cell, GUM_cell *child)
{
    child->parent = cell;
    child->previous = cell->last;
    child->next = NULL;
    // lock
    if (cell->last != NULL)
        cell->last->next = child;
    else
        cell->first = child;
    cell->last = child;
}


static GUM_cell *gum_cell_copy_one(GUM_cell *cell)
{
    GUM_cell *cpy = (GUM_cell*)calloc(sizeof(GUM_cell), 1);
    memcpy(cpy, cell, sizeof(GUM_cell));
    cpy->id = cell->id ? strdup(cell->id) : NULL;
    cpy->text = cell->text ? strdup(cell->text) : NULL;
    cpy->img_src = cell->img_src ? strdup(cell->img_src) : NULL;
    // TODO -- COPY IMAGE !?
    cpy->parent = NULL;
    cpy->previous = NULL;
    cpy->next = NULL;
    cpy->first = NULL;
    cpy->last = NULL;
    // TODO -- COPY SKIN IN CASE OF NO-READONLY
    return cpy;
}

GUM_cell *gum_cell_copy(GUM_cell *cell)
{
    GUM_cell *root;
    GUM_cell *cursor = NULL;
    while (cell) {
        GUM_cell *cpy = gum_cell_copy_one(cell);
        if (cursor) {
            gum_cell_pushback(cursor, cpy);
        }
        else
            root = cpy;
        cursor = cpy;

        if (cell->first) {
            cell = cell->first;
            continue;
        }

        cursor = cursor->parent;
        while (cell && !cell->next) {
            cell = cell->parent;
            if (cursor == NULL && cell == NULL)
                return root;
            cursor = cursor->parent;
        }
        if (cell) {
            cell = cell->next;
        }
    }
    return root;
}

