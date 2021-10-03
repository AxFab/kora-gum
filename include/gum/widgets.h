/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2019  <Fabien Bavent>
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
#ifndef _GUM_WIDGETS_H
#define _GUM_WIDGETS_H 1

#include <gum/core.h>
#include <gum/cells.h>

typedef union GUM_variant {
    double d;
    long long l;
    char *s;
} GUM_variant;

typedef struct GUM_widget {
    gum_cell_t box;
    gum_cell_t ico;
    gum_cell_t txt;
    gum_cell_t bup;
    gum_cell_t bdw;
    gum_cell_t trk;
    gum_cell_t tra;

    GUM_skins *skins;
    GUM_variant value;

    int mode;
    char *text;
} GUM_widget;

typedef struct GUM_container {
    gum_cell_t box;
    gum_cell_t*group;
} GUM_container;

LIBAPI void gum_initialize();

LIBAPI GUM_widget *gum_create_widget(GUM_container *parent, const char *type, const char *text /*, menu, action*/);
LIBAPI void gum_widget_set_text(GUM_widget *widget, const char *text);

LIBAPI GUM_container *gum_container_create(GUM_container *parent, const char *layout, int padding);
LIBAPI GUM_container *gum_container_window(int flags);

#endif  /* _GUM_WIDGETS_H */

