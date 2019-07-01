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
#include <kora/gum/cells.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

GUM_cell *gum_cell_hit_ex(GUM_cell *cell, int x, int y, int mask)
{
    if (cell->state & GUM_CELL_HIDDEN)
        return NULL;
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

void gum_paint(GUM_window *win, GUM_cell *root)
{
    GUM_cell *cell = root;
    char pad[50];
    for (;;) {
        memset(pad, ' ', cell->depth * 2);
        pad[cell->depth * 2] = '\0';
        // fprintf(stderr, "%sPaint <%s.%s> [%d, %d, %d, %d]\n", pad,
        //         cell->id, cell->name, cell->box.x, cell->box.y, cell->box.w, cell->box.h);
        if (!(cell->state & GUM_CELL_HIDDEN))
            gum_draw_cell(win, cell, cell == root);
        if ((cell == root || !(cell->state & GUM_CELL_BUFFERED)) && 1) {
            // TODO - prune if cell is outside drawing clip
            if (cell->first && !(cell->state & GUM_CELL_HIDDEN)) {
                gum_push_clip(win, &cell->box);
                cell = cell->first;
                continue;
            }
        }

        while (!cell->next) {

            if (cell == root || cell->parent == NULL)
                return;

            cell = cell->parent;
            gum_pop_clip(win, &cell->box, cell->parent ? &cell->parent->box : NULL);
            if (cell->state & (GUM_CELL_OVERFLOW_X | GUM_CELL_OVERFLOW_Y) && !(cell->state & GUM_CELL_HIDDEN))  // TODO
                gum_draw_scrolls(win, cell);
        }
        if (cell)
            cell = cell->next;
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


GUM_cell *gum_get_by_id(GUM_cell *cell, const char *id)
{
    while (cell) {
        if (cell->id && !strcmp(id, cell->id))
            return cell;
        if (cell->first) {
            cell = cell->first;
            continue;
        }

        while (cell && !cell->next)
            cell = cell->parent;
        if (cell)
            cell = cell->next;
    }
    return NULL;
}

void gum_cell_detach(GUM_cell *cell)
{
    if (cell == NULL || cell->parent == NULL)
        return;
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

void gum_cell_destroy(GUM_event_manager *evm, GUM_cell *cell)
{
    gum_cell_destroy_children(evm, cell);
    if (evm)
        gum_dereference_cell(evm, cell);
    if (cell->skin)
        gum_skin_close(cell->skin);
    if (cell->skin_down)
        gum_skin_close(cell->skin_down);
    if (cell->skin_over)
        gum_skin_close(cell->skin_over);
    // Skins
    // Cache skin / image / font
    // Animation
    if (cell->id) free(cell->id);
    if (cell->name) free(cell->name);
    if (cell->text) free(cell->text);
    if (cell->img_src) free(cell->img_src);
    free(cell);
}

void gum_cell_destroy_children(GUM_event_manager *evm, GUM_cell *cell)
{
    while (cell->first) {
        GUM_cell *child = cell->first;
        cell->first = child->next;
        gum_cell_destroy(evm, child);
    }
    cell->last = NULL;
}

void gum_destroy_cells(GUM_event_manager *evm, GUM_cell *cell)
{
    gum_cell_destroy_children(evm, cell);
    gum_cell_destroy(evm, cell);
}

void gum_cell_pushback(GUM_cell *cell, GUM_cell *child)
{
    if (child->parent != NULL)
        gum_cell_detach(child);
    child->parent = cell;
    child->previous = cell->last;
    child->next = NULL;
    // lock
    if (cell->last != NULL)
        cell->last->next = child;
    else
        cell->first = child;
    cell->last = child;
    gum_invalid_all(cell);
}

static GUM_cell *gum_cell_copy_one(GUM_cell *cell)
{
    GUM_cell *cpy = (GUM_cell *)calloc(sizeof(GUM_cell), 1);
    memcpy(cpy, cell, sizeof(GUM_cell));
    cpy->id = cell->id ? strdup(cell->id) : NULL;
    cpy->name = cell->name ? strdup(cell->name) : NULL;
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
    GUM_cell *root = NULL;
    GUM_cell *cursor = NULL;
    while (cell) {
        GUM_cell *cpy = gum_cell_copy_one(cell);
        if (cursor)
            gum_cell_pushback(cursor, cpy);
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
        if (cell)
            cell = cell->next;
    }
    return root;
}


GUM_cell *gum_baseof(GUM_cell *cell1, GUM_cell *cell2)
{
    if (cell2 == NULL)
        return cell1;
    while (cell1) {
        GUM_cell *cell = cell2;
        while (cell) {
            if (cell == cell1)
                return cell;
            cell = cell->parent;
        }
        cell1 = cell1->parent;
    }
    return cell2;
}


GUM_event_manager *gum_fetch_manager(GUM_cell *cell)
{
    while (cell->parent)
        cell = cell->parent;
    return cell->manager;
}

void gum_cell_set_text(GUM_cell *cell, const char *text)
{
    if (cell->text)
        free(cell->text);
    cell->text = text ? strdup(text) : NULL;
    gum_invalid_measure(cell);
    gum_invalid_visual(cell);
}

