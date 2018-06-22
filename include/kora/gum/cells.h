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
#ifndef _KORA_GUM_RENDERING_H
#define _KORA_GUM_RENDERING_H 1

#include <kora/gum/core.h>

#ifdef WIN32
# define main app_main
#endif

struct GUM_absolruler
{
    int before, after, center, size, min;
    char bunit, aunit, cunit, sunit, munit;
};

struct GUM_sideruler {
    int left, right, top, bottom;
    char lunit, runit, tunit, bunit;
};

struct GUM_box
{
    int x, y, w, h;
    int minw, minh;
    int cx, cy, cw, ch;
    int mincw, minch;
    int sx, sy;
    int ch_w, ch_h;
};

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

struct GUM_skin
{
    unsigned int bgcolor;
    unsigned int txcolor;
    unsigned int brcolor;
    unsigned int shcolor;
    unsigned int grcolor;
    int width, wunit;
    int height, hunit;
    char read_only;
    char align, valign;
    int grad_angle;
    // Background (image), Color (opacity) or Gradient
    // Border (color, width, style, radius) + image
    // Font properties (family, size, style-weight-variant, stretch, size-adjust)
    // Text properties (outline, direction, decoration, align, shadow, line-height)
    // Visual formating (display, position, float, clear, z-index, overflow, cursor, shadow)
    int r_top_left;
    int r_top_right;
    int r_bottom_right;
    int r_bottom_left;

    char u_top_left;
    char u_top_right;
    char u_bottom_right;
    char u_bottom_left;
};

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

enum {
    GUM_CELL_OVER = (1 << 0),
    GUM_CELL_DOWN = (1 << 1),
    GUM_CELL_FOCUS = (1 << 2),

    GUM_CELL_SOLID = (1 << 3),
    GUM_CELL_EDITABLE = (1 << 4),

    GUM_CELL_OVERFLOW_X = (1 << 5),
    GUM_CELL_OVERFLOW_Y = (1 << 6),

    GUM_CELL_SUBSTYLE = (1 << 7),
};

struct GUM_cell
{
    struct GUM_absolruler rulerx;
    struct GUM_absolruler rulery;
    struct GUM_sideruler padding;
    int gap_x, gap_y;
    char gxunit, gyunit;
    // struct GUM_sideruler margin;

    char *id; // Identifier of the cell
    GUM_box box; // Boxing compiled attribute (size, client, min...)
    GUM_skin *skin;
    GUM_skin *skin_over;
    GUM_skin *skin_down;
    char *text; // Text value
    char *img_src;

    // Cached drawing data
    void *image;
    void *path;
    void *gradient;
    GUM_skin *cachedSkin;

    // Cell hierarchy into the rendering tree
    GUM_cell *parent;
    GUM_cell *previous;
    GUM_cell *next;
    GUM_cell *first;
    GUM_cell *last;

    int state;
    int text_pen;

    void (*layout)(GUM_cell *cell, GUM_layout *layout);
};

struct GUM_layout
{
    // int padleft, padright, padtop, padbottom;
    // int pad_width, pad_height;
    int width; int height;
    int dpi;
    float dsp;
    int flags;
    int cursor, cursor2, cursor3;
    int gap_x, gap_y;

    // Update layout and/or min size, by adding a child to the container
    void (*minsize)(GUM_cell *cell, GUM_cell *child, GUM_layout *layout);
    // Set the size of a cell using layout status
    void (*resize)(GUM_cell *cell, GUM_layout *layout);
};

void gum_layout_absolute(GUM_cell *cell, GUM_layout *layout);
void gum_layout_wrap(GUM_cell *cell, GUM_layout *layout);
void gum_layout_vgroup_extend(GUM_cell *cell, GUM_layout *layout);
void gum_layout_hgroup_extend(GUM_cell *cell, GUM_layout *layout);
void gum_layout_vgroup_left(GUM_cell *cell, GUM_layout *layout);
void gum_layout_hgroup_top(GUM_cell *cell, GUM_layout *layout);
void gum_layout_vgroup_center(GUM_cell *cell, GUM_layout *layout);
void gum_layout_hgroup_middle(GUM_cell *cell, GUM_layout *layout);
void gum_layout_vgroup_right(GUM_cell *cell, GUM_layout *layout);
void gum_layout_hgroup_bottom(GUM_cell *cell, GUM_layout *layout);
void gum_layout_column_grid(GUM_cell *cell, GUM_layout *layout);
void gum_layout_row_grid(GUM_cell *cell, GUM_layout *layout);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */


GUM_cell *gum_cell_loadxml(const char *filename, GUM_skins *skins);

GUM_skins *gum_skins_loadcss(GUM_skins *skins, const char *filename);
GUM_skin* gum_skin_property_setter(GUM_skin *skin, const char* property, const char* value);
GUM_skin *gum_style_find(GUM_skins *skins, const char* name);



GUM_cell *gum_cell_hit(GUM_cell *cell, int x, int y);
GUM_cell *gum_cell_hit_ex(GUM_cell *cell, int x, int y, int mask);
void gum_paint(GUM_window *win, GUM_cell *cell);
void gum_resize(GUM_cell *cell, int width, int height, int dpi, float dsp);


GUM_skin *gum_skin(GUM_cell *cell);
void gum_invalid_cell(GUM_cell *cell, GUM_window *win);
GUM_cell *gum_get_by_id(GUM_cell *cell, const char *id);
void gum_cell_dettach(GUM_cell *cell);
void gum_cell_destroy_children(GUM_cell *cell);

void gum_cell_pushback(GUM_cell *cell, GUM_cell *child);
GUM_cell *gum_cell_copy(GUM_cell *cell);


#endif  /* _KORA_GUM_RENDERING_H */
