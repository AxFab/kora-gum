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
        gum_get_by_id(usr, "hid")->state |= GUM_CELL_HIDDEN;
        usr = usr->next;
    }

    if (cell != selected_user) {
        selected_user = cell;
        printf("Select user '%s' \n", gum_get_by_id(cell, "usr")->text);
        gum_get_by_id(cell, "hid")->state &= ~GUM_CELL_HIDDEN;
        gum_invalid_measure(gum_get_by_id(cell, "usr")) ;
        gum_set_focus(evm, gum_get_by_id(cell, "pwd")) ;
    } else if (selected_user != NULL) {
        printf("Unselect user\n");
        gum_get_by_id(selected_user, "hid")->state |= GUM_CELL_HIDDEN;
        selected_user = NULL;
        gum_set_focus(evm, NULL);
    }
    // give focus
    gum_refresh(evm);
}

void on_login(GUM_event_manager *evm, GUM_cell *cell, int event)
{
    GUM_cell *usr = gum_get_by_id(selected_user, "usr");
    GUM_cell *pwd = gum_get_by_id(selected_user, "pwd");
    printf("Login for user '%s' and password '%s' \n",
        usr->text,
        pwd->text);
    // gum_set_text(pwd, "");

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
        gum_get_by_id(usr, "usr")->text = strdup(strtok(buf, ";\n"));
        gum_get_by_id(usr, "img")->img_src = strdup(strtok(NULL, ";\n"));
        gum_event_bind(evm, usr, GUM_EV_CLICK, on_select);
        gum_event_bind(evm, gum_get_by_id(usr, "go"), GUM_EV_CLICK, on_login);
        gum_cell_pushback(users, usr) ;
    }
    gum_refresh(evm);
}


void on_lang(GUM_event_manager *evm, GUM_cell *cell, int event)
{
    printf("Language menu\n") ;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

/* Graphical User-interface Module */
int main(int argc, char **argv, char **env)
{
    int width = 680 * 2;
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
    gum_event_bind(evm, gum_get_by_id(root, "btn-lang"), GUM_EV_CLICK, on_lang);
    load_users() ;

    gum_event_loop(evm);
    gum_destroy_surface(win);
    // TODO -- Free cells and skins
    return EXIT_SUCCESS;
}


