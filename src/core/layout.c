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
#include <kora/css.h>
#include <stdlib.h>


/* Condensed algorithm: absolute position */
static void gum_layout_absolute_part(struct GUM_absolruler *pos, int minimum,
                                     int container, short dpi, float dsp, int *pPos, int *pSz)
{
    short min = MAX(minimum, CSS_GET_UNIT(pos->min, pos->munit, dpi, dsp, container));
    short size = CSS_GET_UNIT(pos->size, pos->sunit, dpi, dsp, container);
    short before = CSS_GET_UNIT(pos->before, pos->bunit, dpi, dsp, container);
    short after = CSS_GET_UNIT(pos->after, pos->aunit, dpi, dsp, container);
    short center = CSS_GET_UNIT(pos->center, pos->cunit, dpi, dsp, container);

    if (pos->bunit && pos->aunit) {
        *pPos = before;
        *pSz = MAX(min, container - before - after);
    } else if (pos->bunit) {
        *pPos = before;
        if (pos->cunit)
            *pSz = MAX(min, size);
        else
            *pSz = MAX(min, size);
    } else if (pos->aunit) {
        *pPos = container - MAX(min, size) - after;
        if (pos->cunit)
            *pSz = MAX(min, size);
        else
            *pSz = MAX(min, size);
    } else if (pos->cunit) {
        *pPos = (container - MAX(min, size)) / 2 + center;
        *pSz = MAX(min, size);
    } else {
        *pPos = 0;
        *pSz = MAX(min, size);
    }
}

/* Condensed algorithm: absolute position */
static int gum_layout_absolute_min(struct GUM_absolruler *pos, int minimum,
                                     short dpi, float dsp)
{
    int rPos, rSz;
    short min = MAX(minimum, CSS_GET_UNIT(pos->min, pos->munit, dpi, dsp, 0));
    short size = CSS_GET_UNIT_R(pos->size, pos->sunit, dpi, dsp, min);
    short before = CSS_GET_UNIT_R(pos->before, pos->bunit, dpi, dsp, min);
    short after = CSS_GET_UNIT_R(pos->after, pos->aunit, dpi, dsp, min);
    short center = CSS_GET_UNIT_R(pos->center, pos->cunit, dpi, dsp, min);

    if (pos->bunit && pos->aunit) {
        return before + after + MAX(min, - before - after);
    } else if (pos->bunit) {
        rPos = before;
        if (pos->cunit)
            rSz = MAX(min, size);
        else
            rSz = MAX(min, size);
    } else if (pos->aunit) {
        rPos = min - MAX(min, size) - after;
        if (pos->cunit)
            rSz = MAX(min, size);
        else
            rSz = MAX(min, size);
    } else if (pos->cunit) {
        rPos = (min - MAX(min, size)) / 2 + center;
        rSz = MAX(min, size);
        return rSz + abs(rPos);
    } else {
        return MAX(min, size);
    }
    return rSz;
}


static void gum_layout_absolute_minsize(GUM_cell *cell, GUM_cell *child, GUM_layout *layout)
{
    int cw = gum_layout_absolute_min(&child->rulerx, child->box.minw, layout->dpi_x, layout->dsp_x);
    int ch = gum_layout_absolute_min(&child->rulery, child->box.minh, layout->dpi_y, layout->dsp_y);

    short left = CSS_GET_UNIT(child->rulerx.before, child->rulerx.bunit, layout->dpi_x, layout->dsp_x, 0);
    short right = CSS_GET_UNIT(child->rulerx.after, child->rulerx.aunit, layout->dpi_x, layout->dsp_x, 0);
    short width = MAX(cw, left + right + child->box.minw);
    short top = CSS_GET_UNIT(child->rulery.before, child->rulery.bunit, layout->dpi_y, layout->dsp_y, 0);
    short bottom = CSS_GET_UNIT(child->rulery.after, child->rulery.aunit, layout->dpi_y, layout->dsp_y, 0);
    short height = MAX(ch, top + bottom + child->box.minh);
    if (width > cell->box.mincw)
        cell->box.mincw = width;
    if (height > cell->box.minch)
        cell->box.minch = height;
}

static void gum_layout_absolute_resize(GUM_cell *cell, GUM_layout *layout)
{
    GUM_cell *rel;
    cell->box.x = 0;
    cell->box.y = 0;
    cell->box.w = MAX(layout->width, cell->box.minw);
    cell->box.h = MAX(layout->height, cell->box.minh);

    if (cell->rell != NULL) {
        rel = gum_get_by_id(cell->parent, cell->rell);
        cell->rulerx.before = rel != NULL ? rel->box.x : 0;
    }
    if (cell->relr != NULL) {
        rel = gum_get_by_id(cell->parent, cell->relr);
        cell->rulerx.after = layout->width - (rel != NULL ? rel->box.x + rel->box.w : 0);
    }
    if (cell->relt != NULL) {
        rel = gum_get_by_id(cell->parent, cell->relt);
        cell->rulery.before = rel != NULL ? rel->box.y : 0;
    }
    if (cell->relb != NULL) {
        rel = gum_get_by_id(cell->parent, cell->relb);
        cell->rulery.after = layout->height - (rel != NULL ? rel->box.y + rel->box.h : 0);
    }

    gum_layout_absolute_part(&cell->rulerx, cell->box.minw, layout->width, layout->dpi_x, layout->dsp_x, &cell->box.x, &cell->box.w);
    gum_layout_absolute_part(&cell->rulery, cell->box.minh, layout->height, layout->dpi_y, layout->dsp_y, &cell->box.y, &cell->box.h);
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

static void gum_layout_group_resize(GUM_cell *cell, GUM_layout *layout)
{
    short min_width = CSS_GET_UNIT(cell->rulerx.min, cell->rulerx.munit, layout->dpi_x, layout->dsp_x, layout->width);
    short min_height = CSS_GET_UNIT(cell->rulery.min, cell->rulery.munit, layout->dpi_y, layout->dsp_y, layout->height);
    short sz_width = CSS_GET_UNIT(cell->rulerx.size, cell->rulerx.sunit, layout->dpi_x, layout->dsp_x, layout->width);
    short sz_height = CSS_GET_UNIT(cell->rulery.size, cell->rulery.sunit, layout->dpi_y, layout->dsp_y, layout->height);

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
    layout->gap_x = CSS_GET_UNIT(cell->gap_x, cell->gxunit, layout->dpi_x, layout->dsp_x, 0);
    layout->gap_y = CSS_GET_UNIT(cell->gap_y, cell->gyunit, layout->dpi_y, layout->dsp_y, 0);
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


static void gum_layout_fixgrid_minsize(GUM_cell *cell, GUM_cell *child, GUM_layout *layout)
{
    int cw = child->box.minw;
    int ch = child->box.minh;
    cw = cw * layout->cursor2 + layout->gap_x * (layout->cursor2 - 1);
    ch = ch * layout->cursor3 + layout->gap_y * (layout->cursor3 - 1);
    if (cw > cell->box.mincw)
        cell->box.mincw = cw;
    if (ch > cell->box.minch)
        cell->box.minch = ch;
    // TODO -- spancols spanrows
}

static void gum_layout_fixgrid_resize(GUM_cell *cell, GUM_layout *layout)
{
    int cw = (layout->width - layout->gap_x * (layout->cursor2 - 1)) /layout->cursor2;
    int ch = (layout->height - layout->gap_y * (layout->cursor3 - 1)) /layout->cursor3;

    cell->box.w = MAX(cell->box.minw, cw);
    cell->box.h = MAX(cell->box.minh, ch);

    int x = layout->cursor % layout->cursor2;
    int y = layout->cursor / layout->cursor2;

    cell->box.x = x * (cw + layout->gap_x);
    cell->box.y = y * (ch + layout->gap_y);

    layout->cursor++;
}


void gum_layout_fixgrid(GUM_cell *cell, GUM_layout *layout)
{
    layout->width = cell->box.cw;
    layout->height = cell->box.ch;
    layout->flags = 0;
    layout->cursor = 0;
    layout->cursor2 = 2;
    layout->cursor3 = 2;

    layout->gap_x = CSS_GET_UNIT(cell->gap_x, cell->gxunit, layout->dpi_x, layout->dsp_x, 0);
    layout->gap_y = CSS_GET_UNIT(cell->gap_y, cell->gyunit, layout->dpi_y, layout->dsp_y, 0);
    layout->resize = gum_layout_fixgrid_resize;
    layout->minsize = gum_layout_fixgrid_minsize;
}



/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

static void gum_layout_grid_minsize(GUM_cell *cell, GUM_cell *child, GUM_layout *layout)
{
    cell->box.mincw = MAX(cell->box.mincw, child->box.minw);
    cell->box.minch = MAX(cell->box.minch, child->box.minh);
}

static void gum_layout_grid_resize(GUM_cell *cell, GUM_layout *layout)
{
    short min_width = CSS_GET_UNIT(cell->rulerx.min, cell->rulerx.munit, layout->dpi_x, layout->dsp_x, layout->width);
    short min_height = CSS_GET_UNIT(cell->rulery.min, cell->rulery.munit, layout->dpi_y, layout->dsp_y, layout->height);
    short sz_width = CSS_GET_UNIT(cell->rulerx.size, cell->rulerx.sunit, layout->dpi_x, layout->dsp_x, layout->width);
    short sz_height = CSS_GET_UNIT(cell->rulery.size, cell->rulery.sunit, layout->dpi_y, layout->dsp_y, layout->height);

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
    layout->gap_x = CSS_GET_UNIT(cell->gap_x, cell->gxunit, layout->dpi_x, layout->dsp_x, 0);
    layout->gap_y = CSS_GET_UNIT(cell->gap_y, cell->gyunit, layout->dpi_y, layout->dsp_y, 0);
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


bool gum_do_measure(GUM_cell *cell, GUM_gctx *ctx)
{
    int min_w = cell->box.minw;
    int min_h = cell->box.minh;
    bool layout_changed = false;

    /* Get padding size */
    int pad_left = CSS_GET_UNIT(cell->padding.left, cell->padding.lunit, ctx->dpi_x, ctx->dsp_x, cell->box.w);
    int pad_right = CSS_GET_UNIT(cell->padding.right, cell->padding.runit, ctx->dpi_x, ctx->dsp_x, cell->box.w);
    int pad_top = CSS_GET_UNIT(cell->padding.top, cell->padding.tunit, ctx->dpi_y, ctx->dsp_y, cell->box.h);
    int pad_bottom = CSS_GET_UNIT(cell->padding.bottom, cell->padding.bunit, ctx->dpi_y, ctx->dsp_y, cell->box.h);

    /* Compute children minimum size */
    if (cell->first) {
        GUM_cell *child;
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
        short min_width = CSS_GET_UNIT(cell->rulerx.min, cell->rulerx.munit, ctx->dpi_x, ctx->dsp_x, 0);
        short min_height = CSS_GET_UNIT(cell->rulery.min, cell->rulery.munit, ctx->dpi_y, ctx->dsp_y, 0);
        short sz_width = CSS_GET_UNIT(cell->rulerx.size, cell->rulerx.sunit, ctx->dpi_x, ctx->dsp_x, 0);
        short sz_height = CSS_GET_UNIT(cell->rulery.size, cell->rulery.sunit, ctx->dpi_y, ctx->dsp_y, 0);
        cell->box.minw = MAX(cell->box.minw, MAX(min_width, sz_width));
        cell->box.minh = MAX(cell->box.minh, MAX(min_height, sz_height));

        /* Compute content string size */
        int w = 0, h = 0;
        if (cell->text != NULL)
            gum_text_size(cell->text, &w, &h, cell->skin);
        cell->box.minw = MAX(cell->box.minw, w + pad_left + pad_right);
        cell->box.minh = MAX(cell->box.minh, h + pad_top + pad_bottom);
    }


    /* Check if re-layout is required */
    if ((cell->state & (GUM_CELL_OVERFLOW_X | GUM_CELL_OVERFLOW_Y)) == 0 && layout_changed)
        gum_invalid_layout(cell);

    cell->state &= ~GUM_CELL_MEASURE;
    return cell->box.minw != min_w || cell->box.minh != min_h;
}


void gum_do_layout(GUM_cell *cell, GUM_gctx *ctx)
{
    GUM_cell *child;
    GUM_layout layout;
    layout.dpi_x = ctx->dpi_x;
    layout.dsp_x = ctx->dsp_x;
    layout.dpi_y = ctx->dpi_y;
    layout.dsp_y = ctx->dsp_y;

    /* Remove padding */
    int pad_left = CSS_GET_UNIT(cell->padding.left, cell->padding.lunit, ctx->dpi_x, ctx->dsp_x, cell->box.w);
    int pad_right = CSS_GET_UNIT(cell->padding.right, cell->padding.runit, ctx->dpi_x, ctx->dsp_x, cell->box.w);
    int pad_top = CSS_GET_UNIT(cell->padding.top, cell->padding.tunit, ctx->dpi_y, ctx->dsp_y, cell->box.h);
    int pad_bottom = CSS_GET_UNIT(cell->padding.bottom, cell->padding.bunit, ctx->dpi_y, ctx->dsp_y, cell->box.h);

    int cx = cell->box.x + pad_left;
    int cy = cell->box.y + pad_top;
    int cw = cell->box.w - pad_left - pad_right;
    int ch = cell->box.h - pad_top - pad_bottom;

    /* If client size change, invalid visual */
    if (cell->box.cx != cx || cell->box.cy != cy || cell->box.cw != cw || cell->box.ch != ch) {
        cell->box.cx = cx;
        cell->box.cy = cy;
        cell->box.cw = cw;
        cell->box.ch = ch;
        gum_invalid_visual(cell);
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

        /* Comupte actual client size */
        if (child->box.x + child->box.w > cell->box.ch_w)
            cell->box.ch_w = child->box.x + child->box.w;
        if (child->box.y + child->box.h > cell->box.ch_h)
            cell->box.ch_h = child->box.y + child->box.h;
    }
}


void gum_resize(GUM_cell *cell, int width, int height, char xunit, char yunit)
{
    cell->rulerx.sunit = xunit;
    cell->rulery.sunit = yunit;
    cell->rulerx.size = width;
    cell->rulery.size = height;
    if (cell->parent) // TODO, min too
        gum_invalid_layout(cell->parent);
    else {
        GUM_gctx *ctx = gum_graphic_context(cell);
        int sizex = CSS_GET_UNIT(cell->rulerx.size, cell->rulerx.sunit, ctx->dpi_x, ctx->dsp_x, ctx->width);
        int sizey = CSS_GET_UNIT(cell->rulery.size, cell->rulery.sunit, ctx->dpi_y, ctx->dsp_y, ctx->height);
        cell->box.x = 0;
        cell->box.y = 0;
        cell->box.w = MAX(sizex, cell->box.minw);
        cell->box.h = MAX(sizey, cell->box.minh);
        gum_invalid_layout(cell);
    }
}

void gum_resize_px(GUM_cell *cell, int width, int height)
{
    gum_resize(cell, width, height, CSS_SIZE_PX, CSS_SIZE_PX);
}
