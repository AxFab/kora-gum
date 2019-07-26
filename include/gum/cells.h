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
#ifndef _GUM_CELLS_H
#define _GUM_CELLS_H  1

#include <gum/core.h>
#include <kora/css.h>

struct GUM_absolruler {
    css_size_t before, after, center, size, min;
};

struct GUM_sideruler {
    css_size_t left, right, top, bottom;
};

struct GUM_rect {
    int left, right, top, bottom;
};

struct GUM_box {
    int x, y, w, h; // Current area
    int minw, minh; // Min size
    int cx, cy, cw, ch; // Client area
    int mincw, minch; // Min client area
    int sx, sy; // Scrolling
    int dx, dy; // Dragged
    int ch_w, ch_h; // Children real size
};

struct GUM_gctx {
    int dpi_x;
    int dpi_y;
    int width;
    int height;
    float dsp_x;
    float dsp_y;
};

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

struct GUM_skin {
    unsigned int bgcolor;
    unsigned int txcolor;
    unsigned int brcolor;
    unsigned int shcolor;
    unsigned int grcolor;
    css_size_t width;
    css_size_t height;
    char read_only;
    char align, valign;
    int grad_angle;
    // Background (image), Color (opacity) or Gradient
    // Border (color, width, style, radius) + image
    // Font properties (family, size, style-weight-variant, stretch, size-adjust)
    // Text properties (outline, direction, decoration, align, shadow, line-height)
    // Visual formating (display, position, float, clear, z-index, overflow, cursor, shadow)
    css_size_t r_top_left;
    css_size_t r_top_right;
    css_size_t r_bottom_right;
    css_size_t r_bottom_left;

    char *font_family;
    int font_size;
    void *font;
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

    GUM_CELL_HIDDEN = (1 << 8),
    GUM_CELL_BUFFERED = (1 << 9),

    GUM_CELL_DRAGABLE = (1 << 10),

    GUM_CELL_MEASURE = (1 << 16),
};

long long gum_system_time();

typedef struct GUM_anim GUM_anim;
struct GUM_anim {
    int delay, duration;
    long long last;
    float elapsed;
    // method pre/post!
    int ow, oh;
    int ew, eh;
};

typedef void(*GUM_layout_algo)(GUM_cell *, GUM_layout *);

struct GUM_cell {
    struct GUM_absolruler rulerx;
    struct GUM_absolruler rulery;
    GUM_sideruler padding;
    css_size_t gap_x, gap_y;
    // GUM_sideruler margin;

    char *id; // Identifier of the cell
    char *name;
    int depth;
    GUM_box box; // Boxing compiled attribute (size, client, min...)
    GUM_skin *skin;
    GUM_skin *skin_over;
    GUM_skin *skin_down;
    char *text; // Text value
    char *img_src;

    char *rell;
    char *relr;
    char *relt;
    char *relb;

    // Cached drawing data
    void *image;
    void *path;
    void *gradient;
    GUM_skin *cachedSkin;
    GUM_window *surface;

    // Cell hierarchy into the rendering tree
    GUM_cell *parent;
    GUM_cell *previous;
    GUM_cell *next;
    GUM_cell *first;
    GUM_cell *last;

    int state;
    int text_pen;

    GUM_layout_algo layout;

    GUM_anim anim;
    GUM_event_manager *manager;
};

struct GUM_layout {
    // int padleft, padright, padtop, padbottom;
    // int pad_width, pad_height;
    int width;
    int height;
    int dpi_x, dpi_y;
    float dsp_x, dsp_y;
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
void gum_layout_fixgrid(GUM_cell *cell, GUM_layout *layout);

LIBAPI GUM_layout_algo gum_fetch_layout(const char *);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */


LIBAPI GUM_cell *gum_cell_loadxml(const char *filename, GUM_skins *skins);

LIBAPI GUM_skins *gum_skins_loadcss(GUM_skins *skins, const char *filename);
GUM_skin *gum_skin_property_setter(GUM_skin *skin, const char *property, const char *value);
LIBAPI GUM_skin *gum_style_find(GUM_skins *skins, const char *name);


GUM_cell *gum_cell_hit(GUM_cell *cell, int x, int y);
GUM_cell *gum_cell_hit_ex(GUM_cell *cell, int x, int y, int mask);
void gum_paint(GUM_window *win, GUM_cell *cell);
void gum_resize(GUM_cell *cell, css_size_t width, css_size_t height);
void gum_resize_px(GUM_cell *cell, int width, int height);
void gum_do_layout(GUM_cell *cell, GUM_gctx *ctx);
bool gum_do_measure(GUM_cell *cell, GUM_gctx *ctx);

GUM_cell *gum_baseof(GUM_cell *cell1, GUM_cell *cell2);
LIBAPI GUM_event_manager *gum_fetch_manager(GUM_cell *cell);

GUM_skin *gum_skin(GUM_cell *cell);
LIBAPI GUM_cell *gum_get_by_id(GUM_cell *cell, const char *id);
LIBAPI void gum_cell_detach(GUM_cell *cell);
LIBAPI void gum_cell_destroy_children(GUM_event_manager *evm, GUM_cell *cell);

LIBAPI void gum_cell_pushback(GUM_cell *cell, GUM_cell *child);
LIBAPI GUM_cell *gum_cell_copy(GUM_cell *cell);
LIBAPI void gum_dereference_cell(GUM_event_manager *evm, GUM_cell *cell);

LIBAPI void gum_reset_style(GUM_skins *skins, GUM_cell *cell, const char *skinname);

LIBAPI void gum_invalid_properties(GUM_cell *cell);
LIBAPI void gum_invalid_measure(GUM_cell *cell);
LIBAPI void gum_invalid_layout(GUM_cell *cell);
LIBAPI void gum_invalid_visual(GUM_cell *cell);
GUM_gctx *gum_graphic_context(GUM_cell *cell) ;
void gum_invalid_all(GUM_cell *cell);
void gum_skin_close(GUM_skin *skin);

LIBAPI void gum_destroy_cells(GUM_event_manager *evm, GUM_cell *cell);
LIBAPI void gum_destroy_skins(GUM_skins *skins);

LIBAPI void gum_cell_set_text(GUM_cell *cell, const char *text);

#endif  /* _GUM_CELLS_H */
