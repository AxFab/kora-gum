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
#ifndef _KORA_GUM_CORE_H
#define _KORA_GUM_CORE_H 1

typedef struct GUM_cell GUM_cell;
typedef struct GUM_skin GUM_skin;
typedef struct GUM_display GUM_display;
typedef struct GUM_surface GUM_surface;
typedef struct GUM_layout GUM_layout;
typedef struct GUM_skins GUM_skins;
typedef struct GUM_event_manager GUM_event_manager;
typedef struct GUM_event GUM_event;

/* Driver */
void *gum_create_surface(int width, int height);
void* gum_surface_info(GUM_surface *img);
void gum_destroy_surface(void *win);

void gum_draw_cell(void *win, GUM_cell *cell);
void gum_draw_scrolls(void *win, GUM_cell *cell);

int gum_event_poll(void *win, GUM_event *event, int timeout);
int gum_check_events(void *win, int block);
void gum_invalid_surface(void *win, int x, int y, int w, int h);

void *gum_context(void *win);
void gum_complete(void *win, void *ctx);

void gum_text_size(const char *text, int *w, int *h);
void *gum_load_image(const char *name);


#define ALIGN_UP(v,a) (((v)+((a)-1)) & ~((a)-1))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))


/* Commmon */
const char *gum_error();
void gum_log(const char *log);


#endif  /* _KORA_GUM_CORE_H */
