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
#include <gum/events.h>
#include <gum/css.h>
#include <stdlib.h>
#include <kora/mcrs.h>

/* Condensed algorithm: absolute position */
static void gum_layout_absolute_part(struct GUM_absolruler *pos, float minimum, float container, short dpi, float dsp, float*pPos, float*pSz)
{
    float min = MAX(minimum, CSS_GET_SIZE(pos->min, dpi, dsp, container));
    float size = CSS_GET_SIZE(pos->size, dpi, dsp, container);
    float before = CSS_GET_SIZE(pos->before, dpi, dsp, container);
    float after = CSS_GET_SIZE(pos->after, dpi, dsp, container);
    float center = CSS_GET_SIZE(pos->center, dpi, dsp, container);

    if (pos->before.unit && pos->after.unit) {
        *pPos = before;
        *pSz = MAX(min, container - before - after);
    } else if (pos->before.unit) {
        *pPos = before;
        if (pos->center.unit)
            *pSz = MAX(min, size);
        else
            *pSz = MAX(min, size);
    } else if (pos->after.unit) {
        *pPos = container - MAX(min, size) - after;
        if (pos->center.unit)
            *pSz = MAX(min, size);
        else
            *pSz = MAX(min, size);
    } else if (pos->center.unit) {
        *pPos = (container - MAX(min, size)) / 2 + center;
        *pSz = MAX(min, size);
    } else {
        *pPos = 0;
        *pSz = MAX(min, size);
    }
}

/* Condensed algorithm: absolute position */
static int gum_layout_absolute_min(struct GUM_absolruler *pos, float minimum, short dpi, float dsp)
{
    float rPos, rSz;
    float min = MAX(minimum, CSS_GET_SIZE(pos->min, dpi, dsp, 0));
    float size = CSS_GET_SIZE_R(pos->size, dpi, dsp, min);
    float before = CSS_GET_SIZE_R(pos->before, dpi, dsp, min);
    float after = CSS_GET_SIZE_R(pos->after, dpi, dsp, min);
    float center = CSS_GET_SIZE_R(pos->center, dpi, dsp, min);

    if (pos->before.unit && pos->after.unit)
        return before + after + MAX(min, - before - after);
    else if (pos->before.unit) {
        rPos = before;
        if (pos->center.unit)
            rSz = MAX(min, size);
        else
            rSz = MAX(min, size);
    } else if (pos->after.unit) {
        rPos = min - MAX(min, size) - after;
        if (pos->center.unit)
            rSz = MAX(min, size);
        else
            rSz = MAX(min, size);
    } else if (pos->center.unit) {
        rPos = (min - MAX(min, size)) / 2 + center;
        rSz = MAX(min, size);
        return rSz + abs(rPos);
    } else
        return MAX(min, size);
    return rSz;
}


static void gum_layout_absolute_minsize(gum_cell_t *cell, gum_cell_t *child, GUM_layout *layout)
{
    float cw = gum_layout_absolute_min(&child->rulerx, child->box.minw, layout->dpi_x, layout->dsp_x);
    float ch = gum_layout_absolute_min(&child->rulery, child->box.minh, layout->dpi_y, layout->dsp_y);

    float left = CSS_GET_SIZE(child->rulerx.before, layout->dpi_x, layout->dsp_x, 0);
    float right = CSS_GET_SIZE(child->rulerx.after, layout->dpi_x, layout->dsp_x, 0);
    float width = MAX(cw, left + right + child->box.minw);
    float top = CSS_GET_SIZE(child->rulery.before, layout->dpi_y, layout->dsp_y, 0);
    float bottom = CSS_GET_SIZE(child->rulery.after, layout->dpi_y, layout->dsp_y, 0);
    float height = MAX(ch, top + bottom + child->box.minh);
    if (width > cell->box.mincw)
        cell->box.mincw = width;
    if (height > cell->box.minch)
        cell->box.minch = height;
}

static void gum_layout_absolute_resize(gum_cell_t *cell, GUM_layout *layout)
{
    gum_cell_t *rel;
    cell->box.x = 0;
    cell->box.y = 0;
    cell->box.w = MAX(layout->width, cell->box.minw);
    cell->box.h = MAX(layout->height, cell->box.minh);

    //if (cell->rell != NULL) {
    //    rel = gum_get_by_id(cell->parent, cell->rell);
    //    cell->rulerx.before.len = rel != NULL ? rel->box.x : 0;
    //}
    //if (cell->relr != NULL) {
    //    rel = gum_get_by_id(cell->parent, cell->relr);
    //    cell->rulerx.after.len = layout->width - (rel != NULL ? rel->box.x + rel->box.w : 0);
    //}
    //if (cell->relt != NULL) {
    //    rel = gum_get_by_id(cell->parent, cell->relt);
    //    cell->rulery.before.len = rel != NULL ? rel->box.y : 0;
    //}
    //if (cell->relb != NULL) {
    //    rel = gum_get_by_id(cell->parent, cell->relb);
    //    cell->rulery.after.len = layout->height - (rel != NULL ? rel->box.y + rel->box.h : 0);
    //}

    gum_layout_absolute_part(&cell->rulerx, cell->box.minw, layout->width, layout->dpi_x, layout->dsp_x, &cell->box.x, &cell->box.w);
    gum_layout_absolute_part(&cell->rulery, cell->box.minh, layout->height, layout->dpi_y, layout->dsp_y, &cell->box.y, &cell->box.h);
}

static void gum_layout_absolute_fill(gum_cell_t *cell, GUM_layout *layout)
{
    cell->box.x = 0;
    cell->box.y = 0;
    cell->box.w = MAX(layout->width, cell->box.minw);
    cell->box.h = MAX(layout->height, cell->box.minh);
}

void gum_layout_absolute(gum_cell_t *cell, GUM_layout *layout)
{
    layout->width = cell->box.cw;
    layout->height = cell->box.ch;
    layout->resize = gum_layout_absolute_resize;
    layout->minsize = gum_layout_absolute_minsize;
}

void gum_layout_fill(gum_cell_t *cell, GUM_layout *layout)
{
    layout->width = cell->box.cw;
    layout->height = cell->box.ch;
    layout->resize = gum_layout_absolute_fill;
    layout->minsize = gum_layout_absolute_minsize;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

#define GUM_GRP_VERTICAL 1
#define GUM_GRP_TOP   2
#define GUM_GRP_MIDDLE 4
#define GUM_GRP_BOTTOM 8

static void gum_layout_group_minsize(gum_cell_t *cell, gum_cell_t *child, GUM_layout *layout)
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

static void gum_layout_group_resize(gum_cell_t *cell, GUM_layout *layout)
{
    float min_width = CSS_GET_SIZE(cell->rulerx.min, layout->dpi_x, layout->dsp_x, layout->width);
    float min_height = CSS_GET_SIZE(cell->rulery.min, layout->dpi_y, layout->dsp_y, layout->height);
    float sz_width = CSS_GET_SIZE(cell->rulerx.size, layout->dpi_x, layout->dsp_x, layout->width);
    float sz_height = CSS_GET_SIZE(cell->rulery.size, layout->dpi_y, layout->dsp_y, layout->height);

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

static void gum_layout_group(gum_cell_t *cell, GUM_layout *layout, int flags)
{
    layout->width = cell->box.cw;
    layout->height = cell->box.ch;
    layout->flags = flags;
    layout->cursor = 0;
    layout->gap_x = CSS_GET_SIZE(cell->gap_x, layout->dpi_x, layout->dsp_x, 0);
    layout->gap_y = CSS_GET_SIZE(cell->gap_y, layout->dpi_y, layout->dsp_y, 0);
    layout->resize = gum_layout_group_resize;
    layout->minsize = gum_layout_group_minsize;
}

void gum_layout_vgroup_extend(gum_cell_t *cell, GUM_layout *layout)
{
    gum_layout_group(cell, layout, GUM_GRP_VERTICAL);
}

void gum_layout_hgroup_extend(gum_cell_t *cell, GUM_layout *layout)
{
    gum_layout_group(cell, layout, 0);
}

void gum_layout_vgroup_left(gum_cell_t *cell, GUM_layout *layout)
{
    gum_layout_group(cell, layout, GUM_GRP_VERTICAL | GUM_GRP_TOP);
}

void gum_layout_hgroup_top(gum_cell_t *cell, GUM_layout *layout)
{
    gum_layout_group(cell, layout, 0 | GUM_GRP_TOP);
}

void gum_layout_vgroup_center(gum_cell_t *cell, GUM_layout *layout)
{
    gum_layout_group(cell, layout, GUM_GRP_VERTICAL | GUM_GRP_MIDDLE);
}

void gum_layout_hgroup_middle(gum_cell_t *cell, GUM_layout *layout)
{
    gum_layout_group(cell, layout, 0 | GUM_GRP_MIDDLE);
}

void gum_layout_vgroup_right(gum_cell_t *cell, GUM_layout *layout)
{
    gum_layout_group(cell, layout, GUM_GRP_VERTICAL | GUM_GRP_BOTTOM);
}

void gum_layout_hgroup_bottom(gum_cell_t *cell, GUM_layout *layout)
{
    gum_layout_group(cell, layout, 0 | GUM_GRP_BOTTOM);
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */


static void gum_layout_fixgrid_minsize(gum_cell_t *cell, gum_cell_t *child, GUM_layout *layout)
{
    float cw = child->box.minw;
    float ch = child->box.minh;
    cw = cw * layout->cursor2 + layout->gap_x * (layout->cursor2 - 1);
    ch = ch * layout->cursor3 + layout->gap_y * (layout->cursor3 - 1);
    if (cw > cell->box.mincw)
        cell->box.mincw = cw;
    if (ch > cell->box.minch)
        cell->box.minch = ch;
    // TODO -- spancols spanrows
}

static void gum_layout_fixgrid_resize(gum_cell_t *cell, GUM_layout *layout)
{
    float cw = (layout->width - layout->gap_x * (layout->cursor2 - 1)) / layout->cursor2;
    float ch = (layout->height - layout->gap_y * (layout->cursor3 - 1)) / layout->cursor3;

    cell->box.w = MAX(cell->box.minw, cw);
    cell->box.h = MAX(cell->box.minh, ch);

    float x = layout->cursor % layout->cursor2;
    float y = layout->cursor / layout->cursor2;

    cell->box.x = x * (cw + layout->gap_x);
    cell->box.y = y * (ch + layout->gap_y);

    layout->cursor++;
}


void gum_layout_fixgrid(gum_cell_t *cell, GUM_layout *layout)
{
    layout->width = cell->box.cw;
    layout->height = cell->box.ch;
    layout->flags = 0;
    layout->cursor = 0;
    layout->cursor2 = 2;
    layout->cursor3 = 2;

    layout->gap_x = CSS_GET_SIZE(cell->gap_x, layout->dpi_x, layout->dsp_x, 0);
    layout->gap_y = CSS_GET_SIZE(cell->gap_y, layout->dpi_y, layout->dsp_y, 0);
    layout->resize = gum_layout_fixgrid_resize;
    layout->minsize = gum_layout_fixgrid_minsize;
}



/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

static void gum_layout_grid_minsize(gum_cell_t *cell, gum_cell_t *child, GUM_layout *layout)
{
    cell->box.mincw = MAX(cell->box.mincw, child->box.minw);
    cell->box.minch = MAX(cell->box.minch, child->box.minh);
}

static void gum_layout_grid_resize(gum_cell_t *cell, GUM_layout *layout)
{
    float min_width = CSS_GET_SIZE(cell->rulerx.min, layout->dpi_x, layout->dsp_x, layout->width);
    float min_height = CSS_GET_SIZE(cell->rulery.min, layout->dpi_y, layout->dsp_y, layout->height);
    float sz_width = CSS_GET_SIZE(cell->rulerx.size, layout->dpi_x, layout->dsp_x, layout->width);
    float sz_height = CSS_GET_SIZE(cell->rulery.size, layout->dpi_y, layout->dsp_y, layout->height);

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

static void gum_layout_grid(gum_cell_t *cell, GUM_layout *layout, int flags)
{
    layout->width = cell->box.cw;
    layout->height = cell->box.ch;
    layout->flags = flags;
    layout->cursor = 0;
    layout->cursor2 = 0;
    layout->cursor3 = 0;
    layout->gap_x = CSS_GET_SIZE(cell->gap_x, layout->dpi_x, layout->dsp_x, 0);
    layout->gap_y = CSS_GET_SIZE(cell->gap_y, layout->dpi_y, layout->dsp_y, 0);
    layout->resize = gum_layout_grid_resize;
    layout->minsize = gum_layout_grid_minsize;
}

void gum_layout_column_grid(gum_cell_t *cell, GUM_layout *layout)
{
    gum_layout_grid(cell, layout, GUM_GRP_VERTICAL);
}

void gum_layout_row_grid(gum_cell_t *cell, GUM_layout *layout)
{
    gum_layout_grid(cell, layout, 0);
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */


bool gum_do_measure(gum_cell_t *cell, GUM_gctx *ctx)
{
    float min_w = cell->box.minw;
    float min_h = cell->box.minh;
    bool layout_changed = false;

    /* Get padding size */
    float pad_left = CSS_GET_SIZE(cell->padding.left, ctx->dpi_x, ctx->dsp_x, cell->box.w);
    float pad_right = CSS_GET_SIZE(cell->padding.right, ctx->dpi_x, ctx->dsp_x, cell->box.w);
    float pad_top = CSS_GET_SIZE(cell->padding.top, ctx->dpi_y, ctx->dsp_y, cell->box.h);
    float pad_bottom = CSS_GET_SIZE(cell->padding.bottom, ctx->dpi_y, ctx->dsp_y, cell->box.h);

    /* Compute children minimum size */
    if (cell->first) {
        gum_cell_t *child;
        /* Initialize layout */
        GUM_layout layout;
        layout.dpi_x = ctx->dpi_x;
        layout.dsp_x = ctx->dsp_x;
        layout.dpi_y = ctx->dpi_y;
        layout.dsp_y = ctx->dsp_y;
        (cell->layout ? cell->layout : gum_layout_absolute)(cell, &layout);

        /* Check children */
        cell->box.mincw = cell->box.minch = 0;
        for (child = cell->first; child; child = child->next) {
            if (child->state & GUM_CELL_HIDDEN)
                continue;
            layout_changed |= gum_do_measure(child, ctx);
            layout.minsize(cell, child, &layout);
        }

        if (!(cell->state & GUM_CELL_OVERFLOW_X))
            cell->box.minw = MAX(cell->box.minw, cell->box.mincw + pad_left + pad_right);
        if (!(cell->state & GUM_CELL_OVERFLOW_Y))
            cell->box.minh = MAX(cell->box.minh, cell->box.minch + pad_top + pad_bottom);
    }


    if (cell->state & GUM_CELL_MEASURE) {
        /* Compute minimum requested size */
        float min_width = CSS_GET_SIZE(cell->rulerx.min, ctx->dpi_x, ctx->dsp_x, 0);
        float min_height = CSS_GET_SIZE(cell->rulery.min, ctx->dpi_y, ctx->dsp_y, 0);
        float sz_width = CSS_GET_SIZE(cell->rulerx.size, ctx->dpi_x, ctx->dsp_x, 0);
        float sz_height = CSS_GET_SIZE(cell->rulery.size, ctx->dpi_y, ctx->dsp_y, 0);
        cell->box.minw = MAX(cell->box.minw, MAX(min_width, sz_width));
        cell->box.minh = MAX(cell->box.minh, MAX(min_height, sz_height));

        /* Compute content string size */
        float w = 0, h = 0;
        if (cell->text != NULL)
            gum_text_size(cell, &w, &h, cell->skin);
        cell->box.minw = MAX(cell->box.minw, w + pad_left + pad_right);
        cell->box.minh = MAX(cell->box.minh, h + pad_top + pad_bottom);
    }


    /* Check if re-layout is required */
    if ((cell->state & (GUM_CELL_OVERFLOW_X | GUM_CELL_OVERFLOW_Y)) == 0 && layout_changed)
        gum_invalid_layout(gum_fetch_window(cell), cell);

    cell->state &= ~GUM_CELL_MEASURE;
    return cell->box.minw != min_w || cell->box.minh != min_h;
}


void gum_do_layout(gum_cell_t *cell, GUM_gctx *ctx)
{
    gum_cell_t *child;
    GUM_layout layout;
    layout.dpi_x = ctx->dpi_x;
    layout.dsp_x = ctx->dsp_x;
    layout.dpi_y = ctx->dpi_y;
    layout.dsp_y = ctx->dsp_y;

    /* Remove padding */
    float pad_left = CSS_GET_SIZE(cell->padding.left, ctx->dpi_x, ctx->dsp_x, cell->box.w);
    float pad_right = CSS_GET_SIZE(cell->padding.right, ctx->dpi_x, ctx->dsp_x, cell->box.w);
    float pad_top = CSS_GET_SIZE(cell->padding.top, ctx->dpi_y, ctx->dsp_y, cell->box.h);
    float pad_bottom = CSS_GET_SIZE(cell->padding.bottom, ctx->dpi_y, ctx->dsp_y, cell->box.h);

    float cx = cell->box.x + pad_left;
    float cy = cell->box.y + pad_top;
    float cw = cell->box.w - pad_left - pad_right;
    float ch = cell->box.h - pad_top - pad_bottom;

    /* If client size change, invalid visual */
    if (cell->box.cx != cx || cell->box.cy != cy || cell->box.cw != cw || cell->box.ch != ch) {
        cell->box.cx = cx;
        cell->box.cy = cy;
        cell->box.cw = cw;
        cell->box.ch = ch;
        gum_invalid_visual(gum_fetch_window(cell), cell);
    }

    (cell->layout ? cell->layout : gum_layout_absolute)(cell, &layout);

    cell->box.ch_w = 0;
    cell->box.ch_h = 0;
    for (child = cell->first; child; child = child->next) {
        if (child->state & GUM_CELL_HIDDEN)
            continue;

        /* Invalid cache */
        child->cachedSkin = NULL;
        /* Re compute position and size */
        layout.resize(child, &layout);
        /* Do layout on children */
        gum_do_layout(child, ctx);

        /* Compute actual client size */
        if (child->box.x + child->box.w > cell->box.ch_w)
            cell->box.ch_w = child->box.x + child->box.w;
        if (child->box.y + child->box.h > cell->box.ch_h)
            cell->box.ch_h = child->box.y + child->box.h;
    }
}


void gum_resize(gum_cell_t *cell, css_size_t width, css_size_t height)
{
    gum_window_t* win = gum_fetch_window(cell);
    cell->rulerx.size = width;
    cell->rulery.size = height;
    if (cell->parent) // TODO, min too
        gum_invalid_layout(win, cell->parent);
    else {
        GUM_gctx* ctx = &win->ctx;
        float sizex = CSS_GET_SIZE(cell->rulerx.size, ctx->dpi_x, ctx->dsp_x, ctx->width);
        float sizey = CSS_GET_SIZE(cell->rulery.size, ctx->dpi_y, ctx->dsp_y, ctx->height);
        cell->box.x = 0;
        cell->box.y = 0;
        cell->box.w = MAX(sizex, cell->box.minw);
        cell->box.h = MAX(sizey, cell->box.minh);
        gum_invalid_layout(win, cell);
    }
}

void gum_resize_px(gum_cell_t *cell, int width, int height)
{
    css_size_t w = { CSS_SIZE_PX, width  };
    css_size_t h = { CSS_SIZE_PX, height };
    gum_resize(cell, w, h);
}

