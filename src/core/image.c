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
#include <kora/hmap.h>
#include <cairo/cairo.h>
#include <string.h>

HMP_map img_map;
int is_map_init = 0;

void *gum_load_image(const char *name)
{
    if (!is_map_init)
        hmp_init(&img_map, 16);

    cairo_surface_t *img;
    int lg = strlen(name);
    img = hmp_get(&img_map, name, lg);
    if (img != NULL)
        return img;

    img = cairo_image_surface_create_from_png(name);
    if (cairo_surface_status(img) != 0)
        return NULL;

    hmp_put(&img_map, name, lg, img);
    return img;
}

