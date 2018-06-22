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
#include <kora/css.h>

/* Condensed algorithm: aboslute position */
static void gum_layout_absolute_part(struct GUM_absolruler *pos, int minimum,
                                     int container, short dpi, float dsp, int *pPos, int *pSz)
{
    short min = MAX(minimum, CSS_GET_UNIT(pos->min, pos->munit, dpi, dsp, container));
    short size = CSS_GET_UNIT(pos->size, pos->sunit, dpi, dsp, container);
    short before = CSS_GET_UNIT(pos->before, pos->bunit, dpi, dsp, container);
    short after = CSS_GET_UNIT(pos->after, pos->aunit, dpi, dsp, container);

    if (pos->bunit && pos->aunit) {
        *pPos = before;
        *pSz = MAX(min, container - before - after);
    } else if (pos->bunit) {
        *pPos = before;
        *pSz = MAX(min, size);
    } else if (pos->aunit) {
        *pPos = container - MAX(min, size) - after;
        *pSz = MAX(min, size);
    } else {
        *pPos = 0;
        *pSz = MAX(min, size);
    }
}

static void gum_layout_absolute_minsize(GUM_cell *cell, GUM_cell *child, GUM_layout *layout)
{
    short left = CSS_GET_UNIT(child->rulerx.before, child->rulerx.bunit, layout->dpi, layout->dsp, 0);
    short right = CSS_GET_UNIT(child->rulerx.after, child->rulerx.aunit, layout->dpi, layout->dsp, 0);
    short width = left + right + child->box.minw;
    short top = CSS_GET_UNIT(child->rulery.before, child->rulery.bunit, layout->dpi, layout->dsp, 0);
    short bottom = CSS_GET_UNIT(child->rulery.after, child->rulery.aunit, layout->dpi, layout->dsp, 0);
    short height = top + bottom + child->box.minh;
    if (width > cell->box.mincw)
        cell->box.mincw = width;
    if (height > cell->box.minch)
        cell->box.minch = height;
}

static void gum_layout_absolute_resize(GUM_cell *cell, GUM_layout *layout)
{
    cell->box.x = 0;
    cell->box.y = 0;
    cell->box.w = MAX(layout->width, cell->box.minw);
    cell->box.h = MAX(layout->height, cell->box.minh);
    gum_layout_absolute_part(&cell->rulerx, cell->box.minw, layout->width, layout->dpi, layout->dsp, &cell->box.x, &cell->box.w);
    gum_layout_absolute_part(&cell->rulery, cell->box.minh, layout->height, layout->dpi, layout->dsp, &cell->box.y, &cell->box.h);
}

static void gum_layout_absolute_wrap(GUM_cell *cell, GUM_layout *layout)
{
    cell->box.x = 0;
    cell->box.y = 0;
    cell->box.w = MAX(layout->width, cell->box.minw);
    cell->box.h = MAX(layout->height, cell->box.minh);
}

void gum_layout_absolute(GUM_cell *cell, GUM_layout *layout)
{
    layout->width = cell->box.cw;
    layout->height = cell->box.ch;
    layout->resize = gum_layout_absolute_resize;
    layout->minsize = gum_layout_absolute_minsize;
}

void gum_layout_wrap(GUM_cell *cell, GUM_layout *layout)
{
    layout->width = cell->box.cw;
    layout->height = cell->box.ch;
    layout->resize = gum_layout_absolute_wrap;
    layout->minsize = gum_layout_absolute_minsize;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

#define GUM_GRP_VERTICAL 1
#define GUM_GRP_TOP   2
#define GUM_GRP_MIDDLE 4
#define GUM_GRP_BOTTOM 8

static void gum_layout_group_minsize(GUM_cell *cell, GUM_cell *child, GUM_layout *layout)
{
    if (layout->flags & GUM_GRP_VERTICAL) {
        if (child->box.minw > cell->box.mincw)
            cell->box.mincw = child->box.minw;
        layout->cursor += child->box.minh + layout->gap_y;
        cell->box.minch = MAX(cell->box.minch, layout->cursor - layout->gap_y);
    } else {
        if (child->box.minh > cell->box.minch)
            cell->box.minch = child->box.minh;
        layout->cursor += child->box.minw + layout->gap_x;
        cell->box.mincw = MAX(cell->box.mincw, layout->cursor - layout->gap_x);
    }
}

void gum_layout_group_resize(GUM_cell *cell, GUM_layout *layout)
{
    short min_width = CSS_GET_UNIT(cell->rulerx.min, cell->rulerx.munit, layout->dpi, layout->dsp, layout->width);
    short min_height = CSS_GET_UNIT(cell->rulery.min, cell->rulery.munit, layout->dpi, layout->dsp, layout->height);
    short sz_width = CSS_GET_UNIT(cell->rulerx.size, cell->rulerx.sunit, layout->dpi, layout->dsp, layout->width);
    short sz_height = CSS_GET_UNIT(cell->rulery.size, cell->rulery.sunit, layout->dpi, layout->dsp, layout->height);

    cell->box.w = MAX(cell->box.minw, MAX(min_width, sz_width));
    cell->box.h = MAX(cell->box.minh, MAX(min_height, sz_height));

    if (layout->flags & GUM_GRP_VERTICAL) {
        cell->box.y = layout->cursor;
        if (layout->flags & GUM_GRP_TOP)
            cell->box.x = 0;
        else if (layout->flags & GUM_GRP_BOTTOM)
            cell->box.x = layout->width - cell->box.w;
        else if (layout->flags & GUM_GRP_MIDDLE)
            cell->box.x = (layout->width - cell->box.w) / 2;
        else {
            cell->box.x = 0;
            cell->box.w = MAX(cell->box.w, layout->width);
        }
        layout->cursor += cell->box.h + layout->gap_y;
    } else {
        cell->box.x = layout->cursor;
        if (layout->flags & GUM_GRP_TOP)
            cell->box.y = 0;
        else if (layout->flags & GUM_GRP_BOTTOM)
            cell->box.y = layout->height - cell->box.h;
        else if (layout->flags & GUM_GRP_MIDDLE)
            cell->box.y = (layout->height - cell->box.h) / 2;
        else {
            cell->box.y = 0;
            cell->box.h = MAX(cell->box.h, layout->height);
        }
        layout->cursor += cell->box.w + layout->gap_x;
    }
}

static void gum_layout_group(GUM_cell *cell, GUM_layout *layout, int flags)
{
    layout->width = cell->box.cw;
    layout->height = cell->box.ch;
    layout->flags = flags;
    layout->cursor = 0;
    layout->gap_x = CSS_GET_UNIT(cell->gap_x, cell->gxunit, layout->dpi, layout->dsp, 0);
    layout->gap_y = CSS_GET_UNIT(cell->gap_y, cell->gyunit, layout->dpi, layout->dsp, 0);
    layout->resize = gum_layout_group_resize;
    layout->minsize = gum_layout_group_minsize;
}

void gum_layout_vgroup_extend(GUM_cell *cell, GUM_layout *layout)
{
    gum_layout_group(cell, layout, GUM_GRP_VERTICAL);
}

void gum_layout_hgroup_extend(GUM_cell *cell, GUM_layout *layout)
{
    gum_layout_group(cell, layout, 0);
}

void gum_layout_vgroup_left(GUM_cell *cell, GUM_layout *layout)
{
    gum_layout_group(cell, layout, GUM_GRP_VERTICAL | GUM_GRP_TOP);
}

void gum_layout_hgroup_top(GUM_cell *cell, GUM_layout *layout)
{
    gum_layout_group(cell, layout, 0 | GUM_GRP_TOP);
}

void gum_layout_vgroup_center(GUM_cell *cell, GUM_layout *layout)
{
    gum_layout_group(cell, layout, GUM_GRP_VERTICAL | GUM_GRP_MIDDLE);
}

void gum_layout_hgroup_middle(GUM_cell *cell, GUM_layout *layout)
{
    gum_layout_group(cell, layout, 0 | GUM_GRP_MIDDLE);
}

void gum_layout_vgroup_right(GUM_cell *cell, GUM_layout *layout)
{
    gum_layout_group(cell, layout, GUM_GRP_VERTICAL | GUM_GRP_BOTTOM);
}

void gum_layout_hgroup_bottom(GUM_cell *cell, GUM_layout *layout)
{
    gum_layout_group(cell, layout, 0 | GUM_GRP_BOTTOM);
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

static void gum_layout_grid_minsize(GUM_cell *cell, GUM_cell *child, GUM_layout *layout)
{
    cell->box.mincw = MAX(cell->box.mincw, child->box.minw);
    cell->box.minch = MAX(cell->box.minch, child->box.minh);
}

static void gum_layout_grid_resize(GUM_cell *cell, GUM_layout *layout)
{
    short min_width = CSS_GET_UNIT(cell->rulerx.min, cell->rulerx.munit, layout->dpi, layout->dsp, layout->width);
    short min_height = CSS_GET_UNIT(cell->rulery.min, cell->rulery.munit, layout->dpi, layout->dsp, layout->height);
    short sz_width = CSS_GET_UNIT(cell->rulerx.size, cell->rulerx.sunit, layout->dpi, layout->dsp, layout->width);
    short sz_height = CSS_GET_UNIT(cell->rulery.size, cell->rulery.sunit, layout->dpi, layout->dsp, layout->height);

    cell->box.w = MAX(cell->box.minw, MAX(min_width, sz_width));
    cell->box.h = MAX(cell->box.minh, MAX(min_height, sz_height));

    if (layout->flags & GUM_GRP_VERTICAL) {
        if (cell->box.h + layout->cursor > layout->height) {
            layout->cursor = 0;
            layout->cursor2 += layout->cursor3 + layout->gap_x;
            layout->cursor3 = 0;
        }
        if (cell->box.w > layout->cursor3)
            layout->cursor3 = cell->box.w;
        cell->box.y = layout->cursor;
        cell->box.x = layout->cursor2;
        layout->cursor += cell->box.h + layout->gap_y;

    } else {
        cell->box.x = layout->cursor;
        cell->box.y = layout->cursor2;
        layout->cursor += cell->box.w + layout->gap_x;
    }
}

static void gum_layout_grid(GUM_cell *cell, GUM_layout *layout, int flags)
{
    layout->width = cell->box.cw;
    layout->height = cell->box.ch;
    layout->flags = flags;
    layout->cursor = 0;
    layout->cursor2 = 0;
    layout->cursor3 = 0;
    layout->gap_x = CSS_GET_UNIT(cell->gap_x, cell->gxunit, layout->dpi, layout->dsp, 0);
    layout->gap_y = CSS_GET_UNIT(cell->gap_y, cell->gyunit, layout->dpi, layout->dsp, 0);
    layout->resize = gum_layout_grid_resize;
    layout->minsize = gum_layout_grid_minsize;
}

void gum_layout_column_grid(GUM_cell *cell, GUM_layout *layout)
{
    gum_layout_grid(cell, layout, GUM_GRP_VERTICAL);
}

void gum_layout_row_grid(GUM_cell *cell, GUM_layout *layout)
{
    gum_layout_grid(cell, layout, 0);
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

static void gum_cell_minsize(GUM_cell *cell, GUM_layout *layout)
{
    short min_width = CSS_GET_UNIT(cell->rulerx.min, cell->rulerx.munit, layout->dpi, layout->dsp, 0);
    short min_height = CSS_GET_UNIT(cell->rulery.min, cell->rulery.munit, layout->dpi, layout->dsp, 0);
    short sz_width = CSS_GET_UNIT(cell->rulerx.size, cell->rulerx.sunit, layout->dpi, layout->dsp, 0);
    short sz_height = CSS_GET_UNIT(cell->rulery.size, cell->rulery.sunit, layout->dpi, layout->dsp, 0);
    cell->box.minw = MAX(min_width, sz_width);
    cell->box.minh = MAX(min_height, sz_height);
    cell->box.mincw = cell->box.minch = 0;

    int pad_left = CSS_GET_UNIT(cell->padding.left, cell->padding.lunit, layout->dpi, layout->dsp, cell->box.w);
    int pad_right = CSS_GET_UNIT(cell->padding.right, cell->padding.runit, layout->dpi, layout->dsp, cell->box.w);
    int pad_top = CSS_GET_UNIT(cell->padding.top, cell->padding.tunit, layout->dpi, layout->dsp, cell->box.h);
    int pad_bottom = CSS_GET_UNIT(cell->padding.bottom, cell->padding.bunit, layout->dpi, layout->dsp, cell->box.h);

    // TODO Add String size
    int w = 0, h = 0;
    if (cell->text != NULL)
        gum_text_size(cell->text, &w, &h);
    cell->box.minw = MAX(cell->box.minw, w + pad_left + pad_right);
    cell->box.minh = MAX(cell->box.minh, h + pad_top + pad_bottom);

    if (cell->first) {
        GUM_cell *child;
        GUM_layout sub_layout;
        sub_layout.dpi = layout->dpi;
        sub_layout.dsp = layout->dsp;
        if (cell->layout)
            cell->layout(cell, &sub_layout);
        else
            gum_layout_absolute(cell, &sub_layout);

        for (child = cell->first; child; child = child->next) {
            gum_cell_minsize(child, &sub_layout);
            sub_layout.minsize(cell, child, &sub_layout);
        }
        if (!(cell->state & GUM_CELL_OVERFLOW_X))
            cell->box.minw = MAX(cell->box.minw, cell->box.mincw + pad_left + pad_right);
        if (!(cell->state & GUM_CELL_OVERFLOW_Y))
            cell->box.minh = MAX(cell->box.minh, cell->box.minch + pad_top + pad_bottom);
    }
}

static void gum_cell_resize(GUM_cell *cell, GUM_layout *layout)
{
    // Invalid cached data
    cell->cachedSkin = NULL;
    layout->resize(cell, layout);

    int pad_left = CSS_GET_UNIT(cell->padding.left, cell->padding.lunit, layout->dpi, layout->dsp, cell->box.w);
    int pad_right = CSS_GET_UNIT(cell->padding.right, cell->padding.runit, layout->dpi, layout->dsp, cell->box.w);
    cell->box.cx = cell->box.x + pad_left;
    cell->box.cw = cell->box.w - pad_left - pad_right;

    int pad_top = CSS_GET_UNIT(cell->padding.top, cell->padding.tunit, layout->dpi, layout->dsp, cell->box.h);
    int pad_bottom = CSS_GET_UNIT(cell->padding.bottom, cell->padding.bunit, layout->dpi, layout->dsp, cell->box.h);
    cell->box.cy = cell->box.y + pad_top;
    cell->box.ch = cell->box.h - pad_top - pad_bottom;

    // printf("CELL %s %dx%d <%d, %d, %d, %d>  -- %dx%d <%d, %d, %d, %d> \n", cell->id,
    //         cell->box.minw, cell->box.minh,
    //         cell->box.x, cell->box.y, cell->box.w, cell->box.h,
    //         cell->box.mincw, cell->box.minch,
    //         cell->box.cx, cell->box.cy, cell->box.cw, cell->box.ch);
    // Children CELLs
    GUM_cell *child;
    GUM_layout sub_layout;
    sub_layout.dpi = layout->dpi;
    sub_layout.dsp = layout->dsp;

    if (cell->layout)
        cell->layout(cell, &sub_layout);
    else
        gum_layout_absolute(cell, &sub_layout);

    cell->box.ch_w = 0;
    cell->box.ch_h = 0;
    for (child = cell->first; child; child = child->next) {
        gum_cell_resize(child, &sub_layout);
        if (child->box.x + child->box.w > cell->box.ch_w)
            cell->box.ch_w = child->box.x + child->box.w;
        if (child->box.y + child->box.h > cell->box.ch_h)
            cell->box.ch_h = child->box.y + child->box.h;
    }
}

void gum_resize(GUM_cell *cell, int width, int height, int dpi, float dsp)
{
    GUM_layout layout;
    layout.width = width;
    layout.height = height;
    layout.dpi = dpi;
    layout.dsp = dsp;
    layout.resize = gum_layout_absolute_wrap;
    layout.minsize = gum_layout_absolute_minsize;
    gum_cell_minsize(cell, &layout);
    gum_cell_resize(cell, &layout);
}

