#ifndef _KORA_GUM_RENDERING_H
#define _KORA_GUM_RENDERING_H 1

#include <kora/gum/core.h>

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
};

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

struct GUM_skin
{
    unsigned int bgcolor;
    unsigned int hgcolor;
    unsigned int txcolor;
    unsigned int brcolor;
    unsigned int incolor;
    unsigned int shcolor;
    unsigned int grcolor;
    int width, wunit;
    int height, hunit;
    char read_only;
    char align, valign;
    // Background (image), Color (opacity) or Gradient
    // Border (color, width, style, radius) + image
    // Font properties (family, size, style-weight-variant, stretch, size-adjust)
    // Text properties (outline, direction, decoration, align, shadow, line-height)
    // Visual formating (display, position, float, clear, z-index, overflow, cursor, shadow)
};

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

enum {
    GUM_CELL_OVER = (1 << 0),
    GUM_CELL_DOWN = (1 << 1),
    GUM_CELL_FOCUS = (1 << 2),

    GUM_CELL_SOLID = (1 << 3),
    GUM_CELL_EDITABLE = (1 << 4),
};

struct GUM_cell
{
    struct GUM_absolruler rulerx;
    struct GUM_absolruler rulery;
    struct GUM_sideruler padding;
    int gap; char gunit;
    // struct GUM_sideruler margin;

    char *id; // Identifier of the cell
    struct GUM_box box; // Boxing compiled attribute (size, client, min...)
    GUM_skin *skin;
    GUM_skin *skin_over;
    GUM_skin *skin_down;
    char *text; // Text value

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
    int pad_width, pad_height;
    int width; int height;
    int dpi;
    float dsp;
    int flags, cursor, gap;

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


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */


GUM_cell *gum_cell_loadxml(const char *filename, GUM_skins *skins);

GUM_skins *gum_skins_loadcss(GUM_skins *skins, const char *filename);
GUM_skin* gum_skin_property_setter(GUM_skin *skin, const char* property, const char* value);
GUM_skin *gum_style_find(GUM_skins *skins, const char* name);



GUM_cell *gum_cell_hit(GUM_cell *cell, int x, int y);
void gum_paint(GUM_surface *win, GUM_cell *cell);
void gum_resize(GUM_cell *cell, int width, int height, int dpi, float dsp);


GUM_skin *gum_skin(GUM_cell *cell);
void gum_invalid_cell(GUM_cell *cell, GUM_surface *win);
GUM_cell *gum_get_by_id(GUM_cell *cell, const char *id);
void gum_cell_dettach(GUM_cell *cell);
void gum_cell_pushback(GUM_cell *cell, GUM_cell *child);
GUM_cell *gum_cell_copy(GUM_cell *cell);


#endif  /* _KORA_GUM_RENDERING_H */
