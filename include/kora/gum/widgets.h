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
#ifndef _KORA_GUM_WIDGETS_H
#define _KORA_GUM_WIDGETS_H 1

#include <kora/gum/core.h>
#include <kora/gum/cells.h>

typedef union GUM_variant {
    double d;
    long long l;
    char *;
} GUM_variant;

typedef struct GUM_widget {
} GUM_widget;

typedef GUM_container {
} GUM_container;

LIBAPI void gum_initialize();

LIBAPI GUM_widget *gum_widget_factory(GUM_container *parent, const char *type, const char *text /*, menu, action*/);
LIBAPI void gum_widget_set_text(GUM_widget *widget, const char *text);


#endif  /* _KORA_GUM_WIDGETS_H */

