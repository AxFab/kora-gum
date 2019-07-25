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
#include <gum/widgets.h>
#include <gum/events.h>
#include <kora/css.h>
#include <stdlib.h>

GUM_skins *global_skins;

void gum_initialize()
{
    // Load models
    global_skins = gum_skins_loadcss(NULL, "./resx/app.css");
}


GUM_container *gum_container_create(GUM_container *parent, const char *layout, int pad)
{
    GUM_container *container = calloc(1, sizeof(GUM_container));
    gum_cell_pushback(parent->group, &container->box);
    container->box.layout = gum_fetch_layout(layout);
    if (pad == 0)
        container->group = &container->box;
    else {
        container->group = calloc(1, sizeof(GUM_cell));
        gum_cell_pushback(&container->box, container->group);

        container->box.padding.left = pad;
        container->box.padding.right = pad;
        container->box.padding.top = pad;
        container->box.padding.bottom = pad;

        container->box.padding.lunit = CSS_SIZE_PX;
        container->box.padding.runit = CSS_SIZE_PX;
        container->box.padding.tunit = CSS_SIZE_PX;
        container->box.padding.bunit = CSS_SIZE_PX;

        container->group->gap_x = pad / 2;
        container->group->gap_y = pad / 2;
        container->group->gxunit = CSS_SIZE_PX;
        container->group->gyunit = CSS_SIZE_PX;
    }
    container->group->layout = gum_fetch_layout(layout);
    return container;
}


GUM_container *gum_container_window(int flags)
{
    GUM_window *win = gum_create_surface(600, 480);
    if (win == NULL)
        return NULL;
    GUM_container *container = calloc(1, sizeof(GUM_container));

    container->box.manager = gum_event_manager(&container->box, win);
    container->box.skin = gum_style_find(global_skins, "panel");
    container->group = &container->box;

    container->box.layout = gum_fetch_layout("Wrap");
    // TODO - flags - button, border, padding..'
    return container;
}


