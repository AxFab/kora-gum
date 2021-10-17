/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2021  <Fabien Bavent>
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
#ifndef _gum_cell_tS_H
#define _gum_cell_tS_H  1

#include <gum/core.h>
#include <gum/css.h>

struct GUM_absolruler {
    css_size_t before, after, center, size, min;
};

struct GUM_box {
    float x, y, w, h; // Current area
    float minw, minh; // Min size
    float cx, cy, cw, ch; // Client area
    float mincw, minch; // Min client area
    float sx, sy; // Scrolling
    float dx, dy; // Dragged
    float ch_w, ch_h; // Children real size
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
    css_size_t brsize;
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
    double font_size;
    // void *font;
};

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

enum {
    GUM_CELL_OVER = (1 << 0), // Mouse is over this cell
    GUM_CELL_DOWN = (1 << 1), // Mouse is press on this cell
    GUM_CELL_FOCUS = (1 << 2), // This cell have focus

    GUM_CELL_SOLID = (1 << 3), // This cell is usable for mouse event
    GUM_CELL_EDITABLE = (1 << 4), // The text of this cell can be edit

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

typedef void(*GUM_layout_algo)(gum_cell_t *, GUM_layout *);

struct gum_cell {
    struct GUM_absolruler rulerx;
    struct GUM_absolruler rulery;
    css_box_t padding;
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

    //char *rell;
    //char *relr;
    //char *relt;
    //char *relb;

    // Cached drawing data
    void *image;
    void *path;
    void *gradient;
    GUM_skin *cachedSkin;
    void *font;

    // Cell hierarchy into the rendering tree
    gum_cell_t *parent;
    gum_cell_t *previous;
    gum_cell_t *next;
    gum_cell_t *first;
    gum_cell_t *last;

    int state;
    int text_pen;

    GUM_layout_algo layout;

    GUM_anim anim;
    gum_window_t* win;
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
    void (*minsize)(gum_cell_t *cell, gum_cell_t *child, GUM_layout *layout);
    // Set the size of a cell using layout status
    void (*resize)(gum_cell_t *cell, GUM_layout *layout);
};

void gum_layout_absolute(gum_cell_t *cell, GUM_layout *layout);
void gum_layout_fill(gum_cell_t *cell, GUM_layout *layout);
void gum_layout_vgroup_extend(gum_cell_t *cell, GUM_layout *layout);
void gum_layout_hgroup_extend(gum_cell_t *cell, GUM_layout *layout);
void gum_layout_vgroup_left(gum_cell_t *cell, GUM_layout *layout);
void gum_layout_hgroup_top(gum_cell_t *cell, GUM_layout *layout);
void gum_layout_vgroup_center(gum_cell_t *cell, GUM_layout *layout);
void gum_layout_hgroup_middle(gum_cell_t *cell, GUM_layout *layout);
void gum_layout_vgroup_right(gum_cell_t *cell, GUM_layout *layout);
void gum_layout_hgroup_bottom(gum_cell_t *cell, GUM_layout *layout);
void gum_layout_column_grid(gum_cell_t *cell, GUM_layout *layout);
void gum_layout_row_grid(gum_cell_t *cell, GUM_layout *layout);
void gum_layout_fixgrid(gum_cell_t *cell, GUM_layout *layout);

LIBAPI GUM_layout_algo gum_fetch_layout(const char *);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

LIBAPI void gum_cell_attribute(gum_cell_t* cell, const char* key, const char* value);
LIBAPI void gum_cell_style(gum_cell_t* cell, GUM_skins* skins, const char* name);
LIBAPI gum_cell_t* gum_create_cell(const char* name, gum_cell_t* parent);

LIBAPI gum_cell_t *gum_cell_loadxml(const char *filename, GUM_skins *skins);

LIBAPI gum_skin_t* gum_skin_open(gum_skin_t* skin);
LIBAPI GUM_skins *gum_skins_loadcss(GUM_skins *skins, const char *filename);
GUM_skin *gum_skin_property_setter(GUM_skin *skin, const char *property, const char *value);
LIBAPI GUM_skin *gum_style_find(GUM_skins *skins, const char *name);


gum_cell_t *gum_cell_hit(gum_cell_t *cell, float x, float y);
gum_cell_t *gum_cell_hit_ex(gum_cell_t *cell, float x, float y, int mask);
void gum_paint(gum_window_t *win, gum_cell_t *cell);
void gum_resize(gum_cell_t *cell, css_size_t width, css_size_t height);
void gum_resize_px(gum_cell_t *cell, int width, int height);
void gum_do_layout(gum_cell_t *cell, GUM_gctx *ctx);
bool gum_do_measure(gum_cell_t *cell, GUM_gctx *ctx);

gum_cell_t *gum_baseof(gum_cell_t *cell1, gum_cell_t *cell2);

GUM_skin *gum_skin(gum_cell_t *cell);
LIBAPI gum_cell_t *gum_get_by_id(gum_cell_t *cell, const char *id);
LIBAPI void gum_cell_detach(gum_cell_t *cell);
LIBAPI void gum_cell_destroy_children(gum_window_t *win, gum_cell_t *cell);

LIBAPI void gum_cell_pushback(gum_cell_t *cell, gum_cell_t *child);
LIBAPI gum_cell_t *gum_cell_copy(gum_cell_t *cell);
LIBAPI void gum_dereference_cell(gum_window_t* win, gum_cell_t *cell);

LIBAPI void gum_reset_style(GUM_skins *skins, gum_cell_t *cell, const char *skinname);

LIBAPI void gum_invalid_measure(gum_window_t* win, gum_cell_t *cell);
LIBAPI void gum_invalid_layout(gum_window_t* win, gum_cell_t *cell);
LIBAPI void gum_invalid_visual(gum_window_t* win, gum_cell_t *cell);
// void gum_invalid_all(gum_cell_t *cell);
void gum_skin_close(GUM_skin *skin);

LIBAPI void gum_destroy_cells(gum_window_t* win, gum_cell_t *cell);
LIBAPI void gum_destroy_skins(GUM_skins *skins);

LIBAPI void gum_cell_set_text(gum_cell_t *cell, const char *text);

gum_window_t* gum_fetch_window(gum_cell_t* cell);

void gum_push_clip(gum_window_t* win, GUM_box* box);
void gum_pop_clip(gum_window_t* win, GUM_box* box, GUM_box* prev);
void gum_draw_cell(gum_window_t* win, gum_cell_t* cell);


#endif  /* _gum_cell_tS_H */
