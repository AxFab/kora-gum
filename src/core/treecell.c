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
#include <gum/cells.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

gum_cell_t *gum_cell_hit_ex(gum_cell_t *cell, float x, float y, int mask)
{
    if (cell->state & GUM_CELL_HIDDEN)
        return NULL;
    if (x < cell->box.x || y < cell->box.y ||
        x >= cell->box.x + cell->box.w || y >= cell->box.y + cell->box.h)
        return NULL;

    gum_cell_t *child, *hit;
    for (child = cell->last; child; child = child->previous) {
        hit = gum_cell_hit_ex(child,
                              x - cell->box.cx + cell->box.sx,
                              y - cell->box.cy + cell->box.sy, mask);
        if (hit)
            return hit;
    }

    return cell->state & mask ? cell : NULL;
}

gum_cell_t *gum_cell_hit(gum_cell_t *cell, float x, float y)
{
    return gum_cell_hit_ex(cell, x, y, GUM_CELL_SOLID);
}

void gum_paint(gum_window_t *win, gum_cell_t *root)
{
    gum_cell_t *cell = root;
    char pad[50];
    for (;;) {
        memset(pad, ' ', cell->depth * 2);
        pad[cell->depth * 2] = '\0';
        // fprintf(stderr, "%sPaint <%s.%s> [%3.1f, %3.1f, %3.1f, %3.1f]\n", pad, cell->id, cell->name, cell->box.x, cell->box.y, cell->box.w, cell->box.h);
        if (!(cell->state & GUM_CELL_HIDDEN))
            gum_draw_cell(win, cell);
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
            //if (cell->state & (gum_cell_t_OVERFLOW_X | gum_cell_t_OVERFLOW_Y) && !(cell->state & gum_cell_t_HIDDEN)) {
            //    gum_draw_scrolls(win, cell);
            //}
        }
        if (cell)
            cell = cell->next;
    }
}

GUM_skin *gum_skin(gum_cell_t *cell)
{
    gum_cell_t *ref = cell;
    while (ref->state & GUM_CELL_SUBSTYLE)
        ref = ref->parent;

    if (ref->state & GUM_CELL_DOWN && cell->skin_down)
        return cell->skin_down;
    else if (ref->state & GUM_CELL_OVER && cell->skin_over)
        return cell->skin_over;

    return cell->skin;
}


gum_cell_t *gum_get_by_id(gum_cell_t *cell, const char *id)
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

void gum_cell_detach(gum_cell_t *cell)
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

static void gum_cell_destroy(gum_window_t* win, gum_cell_t *cell)
{
    gum_cell_destroy_children(win, cell);
    if (win)
        gum_dereference_cell(win, cell);
    if (cell->skin)
        gum_skin_close(cell->skin);
    if (cell->skin_down)
        gum_skin_close(cell->skin_down);
    if (cell->skin_over)
        gum_skin_close(cell->skin_over);
    // Skins
    // Cache skin / image / font
    // Animation
    if (cell->id)
        free(cell->id);
    if (cell->name)
        free(cell->name);
    if (cell->text)
        free(cell->text);
    if (cell->img_src)
        free(cell->img_src);
    free(cell);
}

void gum_cell_destroy_children(gum_window_t* win, gum_cell_t *cell)
{
    gum_invalid_visual(win, cell);
    while (cell->first) {
        gum_cell_t *child = cell->first;
        cell->first = child->next;
        gum_cell_destroy(win, child);
    }
    cell->last = NULL;
}

void gum_destroy_cells(gum_window_t* win, gum_cell_t *cell)
{
    gum_cell_destroy_children(win, cell);
    gum_cell_destroy(win, cell);
}

void gum_cell_pushback(gum_cell_t *cell, gum_cell_t *child)
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
    gum_window_t* win = gum_fetch_window(cell);
    gum_invalid_measure(win, cell);
    gum_invalid_layout(win, cell);
}

static gum_cell_t *gum_cell_copy_one(gum_cell_t *cell)
{
    gum_cell_t *cpy = (gum_cell_t *)calloc(sizeof(gum_cell_t), 1);
    memcpy(cpy, cell, sizeof(gum_cell_t));
    cpy->id = cell->id ? strdup(cell->id) : NULL;
    cpy->name = cell->name ? strdup(cell->name) : NULL;
    cpy->text = cell->text ? strdup(cell->text) : NULL;
    cpy->img_src = cell->img_src ? strdup(cell->img_src) : NULL;
    cell->state |= GUM_CELL_MEASURE;
    // TODO -- COPY IMAGE !?
    cpy->parent = NULL;
    cpy->previous = NULL;
    cpy->next = NULL;
    cpy->first = NULL;
    cpy->last = NULL;
    // TODO -- COPY SKIN IN CASE OF NO-READONLY
    return cpy;
}

gum_cell_t *gum_cell_copy(gum_cell_t *cell)
{
    gum_cell_t *root = NULL;
    gum_cell_t *cursor = NULL;
    while (cell) {
        gum_cell_t *cpy = gum_cell_copy_one(cell);
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


gum_cell_t *gum_baseof(gum_cell_t *cell1, gum_cell_t *cell2)
{
    if (cell2 == NULL)
        return cell1;
    while (cell1) {
        gum_cell_t *cell = cell2;
        while (cell) {
            if (cell == cell1)
                return cell;
            cell = cell->parent;
        }
        cell1 = cell1->parent;
    }
    return cell2;
}


gum_window_t * gum_fetch_window(gum_cell_t *cell)
{
    while (cell->parent)
        cell = cell->parent;
    return cell->win;
}

void gum_cell_set_text(gum_cell_t *cell, const char *text)
{
    if (cell->text)
        free(cell->text);
    cell->text = text ? strdup(text) : NULL;
    gum_window_t* win = gum_fetch_window(cell);
    if (win) {
        gum_invalid_measure(win, cell);
        gum_invalid_visual(win, cell);
    }
}

#ifndef NDEBUG

void gum_debug_show_tree(gum_cell_t *cell, int depth)
{
    char *indent = malloc(depth * 2 + 1);
    memset(indent, ' ', depth * 2);
    indent[depth * 2] = '\0';
    printf("%s> Cell [%s: %s] %p\n", indent, cell->name, cell->id, cell);
    gum_cell_t *child = cell->first;
    if (child == NULL) {
        if (cell->last != NULL)
            printf("%s  | Error children linked are corrupted\n", indent);
    } else {

        for (child = cell->first; child; child = child->next) {
            if (child->parent != cell)
                printf("%s  | Error child is not linked to parent\n", indent);
            if (child->previous == NULL && child != cell->first)
                printf("%s  | Error on a children\n", indent);
            if (child->next == NULL && child != cell->last)
                printf("%s  | Error on a children\n", indent);
            if (child == cell->first && child->previous != NULL)
                printf("%s  | Error on first children\n", indent);
            if (child == cell->last && child->next != NULL)
                printf("%s  | Error on last children\n", indent);
            if (child->previous != NULL && child->previous->next != child)
                printf("%s  | Link Error on a children\n", indent);
            if (child->next != NULL && child->next->previous != child)
                printf("%s  | Link Error on a children\n", indent);
        }
        // Check all
    }

    for (child = cell->first; child; child = child->next)
        gum_debug_show_tree(child, depth + 1);
    free(indent);
}

#endif
