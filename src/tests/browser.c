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
#include <kora/gum/cells.h>
#include <kora/gum/events.h>
#include <kora/xml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>

char current_path[8192];
GUM_cell *view;
GUM_cell *icon;
GUM_cell *ico_txt;
GUM_cell *ico_img;
GUM_event_manager * evm;
GUM_cell *root;

void on_refresh(GUM_event_manager *evm, GUM_cell *cell, int event);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */


void on_icon_click(GUM_event_manager *evm, GUM_cell *cell, int event)
{
    strcat(current_path, "/");
    strcat(current_path, cell->last->text);
    printf("Click - Go to : %s\n", current_path);
    on_refresh(evm, cell, event);
}

void on_previous(GUM_event_manager *evm, GUM_cell *cell, int event)
{
}

void on_next(GUM_event_manager *evm, GUM_cell *cell, int event)
{
}

void on_parent(GUM_event_manager *evm, GUM_cell *cell, int event)
{
    char *last = strrchr(current_path, '/');
    if (last != NULL)
        *last = '\0';
    printf("Parent - Go to : %s\n", current_path);
    on_refresh(evm, cell, event);
}

void on_refresh(GUM_event_manager *evm, GUM_cell *cell, int event)
{
    void *dir = opendir(current_path[0] == '\0' ? "/" : current_path);
    if (dir == NULL) {
        printf("Can't open directory '%s'\n", current_path);
        return;
    }
    gum_cell_destroy_children(view);
    for (;;) {
        struct dirent *en = readdir(dir);
        if (en == NULL)
            break;
        if (strcmp(en->d_name, ".") == 0 || strcmp(en->d_name, "..") == 0)
            continue;
        if (en->d_name[0] == '.')
            continue;
        if (ico_txt->text)
            free(ico_txt->text);
        ico_txt->text = strdup(en->d_name);

        if (ico_img->img_src)
            free(ico_img->img_src);
        if (en->d_type == 4)
            ico_img->img_src = strdup("./icons/DIR.png");
        else {
            ico_img->img_src = strdup("./icons/TXT.png");
        }
        // fprintf(stderr, "Dirent %s\n", en->d_name);
        GUM_cell *cpy = gum_cell_copy(icon);
        gum_cell_pushback(view, cpy);
        gum_event_bind(evm, cpy, GUM_EV_CLICK, on_icon_click);
    }
    closedir(dir);
    gum_refresh(evm);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

/* Graphical User-interface Module */
int S_main ()
{
    int width = 680;
    int height = width * 10 / 16; // 425

    // Load models
    GUM_skins *skins = gum_skins_loadcss(NULL, "./resx/browser/app2.css");
    root = gum_cell_loadxml("./resx/browser/app2.xml", skins);
    if (root == NULL) {
        printf("Unable to create render model.\n");
        return EXIT_FAILURE;
    }

    // Open Window
    GUM_window *win = gum_create_surface(width, height);
    if (win == NULL) {
        printf("Unable to initialize window.\n");
        return EXIT_FAILURE;
    }

    // Remouve context menu
    GUM_cell *ctx_view = gum_get_by_id(root, "ctx-menu-view");
    GUM_cell *ctx_file = gum_get_by_id(root, "ctx-menu-file");
    gum_cell_dettach(ctx_view);
    gum_cell_dettach(ctx_file);

    // Prepare widget using template
    view = gum_get_by_id(root, "view");
    icon = gum_get_by_id(root, "icon");
    ico_txt = gum_get_by_id(icon, "icon-text");
    ico_img = gum_get_by_id(icon, "icon-img");
    gum_cell_dettach(icon);

    evm = gum_event_manager(root, win);
    gum_event_bind(evm, NULL, GUM_EV_PREVIOUS, on_parent);
    gum_event_bind(evm, NULL, GUM_EV_NEXT, on_next);
    gum_event_bind(evm, gum_get_by_id(root, "btn-prev"), GUM_EV_CLICK, on_previous);
    gum_event_bind(evm, gum_get_by_id(root, "btn-next"), GUM_EV_CLICK, on_next);
    gum_event_bind(evm, gum_get_by_id(root, "btn-top"), GUM_EV_CLICK, on_parent);
    gum_event_bind(evm, gum_get_by_id(root, "btn-refr"), GUM_EV_CLICK, on_refresh);


    strcpy(current_path, "/home/fabien");
    on_refresh(evm, NULL, 0);

    fprintf(stderr, "View: %dx%d  - zone %dx%d\n",
        view->box.w, view->box.h,
        view->box.mincw, view->box.minch);
    // view->box.sy = 20;

    gum_event_loop(evm);
    gum_destroy_surface(win);
    // TODO -- Free cells and skins
    return EXIT_SUCCESS;
}


