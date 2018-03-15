// #include <kora/gum/display.h>
#include <kora/gum/rendering.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


GUM_cell *gum_cell_hit(GUM_cell *cell, int x, int y)
{
    if (x < cell->box.x || y < cell->box.y ||
            x >= cell->box.x + cell->box.w || y >= cell->box.y + cell->box.h)
        return NULL;

    GUM_cell *child, *hit;
    for (child = cell->last; child; child = child->previous) {
        hit = gum_cell_hit(child, x - cell->box.cx, y - cell->box.cy);
        if (hit)
            return hit;
    }

    return cell->state & GUM_CELL_SOLID ? cell : NULL;
}


void gum_paint(void *ctx, GUM_cell *cell)
{
    // void *ctx = gum_context(win);
    int x = 0, y = 0;
    for (;;) {
        // fprintf(stderr, "Paint %s [%d, %d, %d, %d]\n",
        //     cell->id, cell->box.x, cell->box.y, cell->box.w, cell->box.h);
        gum_draw_cell(ctx, cell, x, y);
        if (cell->first) {
            x += cell->box.cx - cell->box.sx;
            y += cell->box.cy - cell->box.sy;
            // TODO - prune if cell is outside drawing clip
            cell = cell->first;
            continue;
        }

        while (!cell->next) {
            cell = cell->parent;
            if (cell == NULL) {
                // gum_complete(win, ctx);
                return;
            }
            x -= cell->box.cx - cell->box.sx;
            y -= cell->box.cy - cell->box.sy;
            if (cell->box.sy != 0) { // TODO
                gum_draw_scrolls(ctx, cell, x, y);
            }
        }
        if (cell) {
            cell = cell->next;
        }
    }
}

GUM_skin *gum_skin(GUM_cell *cell)
{
    if (cell->state & GUM_CELL_DOWN && cell->skin_down)
        return cell->skin_down;
    else if (cell->state & GUM_CELL_OVER && cell->skin_over)
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
        cell->parent->last = cell->next;
    cell->previous = NULL;
    cell->next = NULL;
    cell->parent = NULL;
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

