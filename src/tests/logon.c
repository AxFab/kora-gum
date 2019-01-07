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
#include <kora/gum/core.h>
#include <kora/gum/cells.h>
#include <kora/gum/events.h>
#include <kora/xml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


GUM_event_manager *evm;
GUM_cell *root;
GUM_skins *skins;

GUM_cell *user;
GUM_cell *users;
GUM_cell *selected_user;

void on_select(GUM_event_manager *evm, GUM_cell *cell, int event)
{
    GUM_cell *usr = users->first ;
    while (usr) {
        usr->last->state |= GUM_CELL_HIDDEN;
        usr = usr->next;
    }

    if (cell != selected_user) {
        selected_user = cell;
        printf("Select user '%s' \n", cell->first->next->text);
        cell->last->state &= ~GUM_CELL_HIDDEN;
    } else {
        printf("Unselect user\n");
        selected_user = NULL;
    }
    // give focus
    gum_refresh(evm);
}

void load_users()
{
    gum_cell_destroy_children(users);
    FILE *info = fopen("./resx/logon/password.txt", "r");
    if (info == NULL)
        return;
    char buf[512];
    while (fgets(buf, 512, info)) {
        GUM_cell *usr = gum_cell_copy(user) ;
        usr->first->next->text = strdup(strtok(buf, ";\n"));
        usr->first->img_src = strdup(strtok(NULL, ";\n"));
        gum_event_bind(evm, usr, GUM_EV_CLICK, on_select);
        gum_cell_pushback(users, usr) ;
    }
    gum_refresh(evm);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

/* Graphical User-interface Module */
int main(int argc, char **argv, char **env)
{
    int width = 680;
    int height = width * 10 / 16; // 425

    // Load models
    skins = gum_skins_loadcss(NULL, "./resx/logon/app.css");
    root = gum_cell_loadxml("./resx/logon/app.xml", skins);
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

    users = gum_get_by_id(root, "users") ;
    user = gum_get_by_id(root, "user") ;
    gum_cell_detach(user) ;

    evm = gum_event_manager(root, win);
    // gum_event_bind(evm, NULL, GUM_EV_PREVIOUS, on_parent);
    load_users() ;

    gum_event_loop(evm);
    gum_destroy_surface(win);
    // TODO -- Free cells and skins
    return EXIT_SUCCESS;
}


