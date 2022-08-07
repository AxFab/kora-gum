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
#ifndef _GUM_CORE_H
#define _GUM_CORE_H 1

#include <stddef.h>
#include <stdbool.h>

#ifndef LIBAPI
# if defined(WIN32) || defined(_WIN32)
#  define LIBAPI __declspec(dllexport)
# else
#  define LIBAPI
# endif
#endif

typedef struct gum_window gum_window_t;
typedef struct gum_cell gum_cell_t;

typedef struct GUM_skins GUM_skins;
typedef struct GUM_skin GUM_skin;
typedef struct GUM_skins gum_skins_t;
typedef struct GUM_skin gum_skin_t;

typedef struct GUM_box GUM_box;
typedef struct GUM_layout GUM_layout;
typedef struct GUM_gctx GUM_gctx;


/* Driver */
//LIBAPI GUM_window *gum_create_surface(int width, int height);
//LIBAPI void gum_destroy_surface(GUM_window *win);
//
//void gum_draw_cell(GUM_window *win, GUM_cell *cell, bool top);
//void gum_draw_scrolls(GUM_window *win, GUM_cell *cell);
//
//int gum_event_poll(GUM_window *win, GUM_event *event, int timeout);
//void gum_invalid_surface(GUM_window *win, int x, int y, int w, int h);

void gum_text_size(gum_cell_t *cell, float*w, float*h, GUM_skin *skin, GUM_gctx* ctx);

void *gum_image(const char *name);
void* gum_load_image(const char* source);

void* gum_load_fontface(const char* source);
void* gum_face(const char* name);
// LIBAPI unsigned gum_mix(unsigned src, unsigned dest, float mx);

//void gum_start_paint(GUM_window *win);
//void gum_end_paint(GUM_window *win);
//void gum_push_clip(GUM_window *win, GUM_box *box);
//void gum_pop_clip(GUM_window *win, GUM_box *box, GUM_box *prev);
//void gum_resize_win(GUM_window *win, int width, int height);
//void gum_fill_context(GUM_window *win, GUM_gctx *ctx);


struct GUM_gctx {
    int dpi_x;
    int dpi_y;
    int width;
    int height;
    float dsp_x;
    float dsp_y;
};



#endif  /* _GUM_CORE_H */
