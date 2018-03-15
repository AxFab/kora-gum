#include <kora/gum/rendering.h>
#include <kora/gum/events.h>
#include <kora/xml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>

#include <cairo/cairo.h>

#define M_PI 3.141592653589793

/* Graphical User-interface Module */
int main ()
{
    int width = 680;
    int height = width * 10 / 16; // 425

    // Load models
    GUM_skins *skins = gum_skins_loadcss(NULL, "./resx/browser/app.css");
    GUM_cell *cell = gum_cell_loadxml("./resx/browser/app.xml", skins);
    if (cell == NULL) {
        printf("Unable to create render model.\n");
        return -1;
    }

    // Open Window
    void *win = gum_create_surface(width, height);
    if (win == NULL) {
        printf("Unable to initialize window.\n");
        return -1;
    }

    // Remouve context menu
    // GUM_cell *ctx_view = gum_get_by_id(cell, "ctx-menu-view");
    // GUM_cell *ctx_file = gum_get_by_id(cell, "ctx-menu-file");
    // // gum_cell_dettach(ctx_view);
    // // gum_cell_dettach(ctx_file);

    // Create widget using template
    GUM_cell *view = gum_get_by_id(cell, "view");
    GUM_cell *icon = gum_get_by_id(cell, "icon");
    GUM_cell *ico_txt = gum_get_by_id(icon, "icon-text");
    // GUM_cell *ico_img = gum_get_by_id(icon, "icon-img");
    gum_cell_dettach(icon);

    void *dir = opendir("/home/fabien");
    for (;;) {
        struct dirent *en = readdir(dir);
        if (en == NULL)
            break;
        if (ico_txt->text)
            free(ico_txt->text);
        ico_txt->text = strdup(en->d_name);
        fprintf(stderr, "Dirent %s\n", en->d_name);
        GUM_cell *cpy = gum_cell_copy(icon);
        gum_cell_pushback(view, cpy);
    }
    closedir(dir);

    GUM_event_manager * evm = gum_event_manager(cell, win);
    // TODO -- bind event ! manager, CellById(root, "btn-previous"), GUM_CE_CLICK, onPrevious);

    // gum_resize(cell, 680, 480, 96, 0.75);
    fprintf(stderr, "View: %dx%d  - zone %dx%d\n",
        view->box.w, view->box.h,
        view->box.mincw, view->box.minch);
    view->box.sy = 20;

    // struct timespec ts = { 0, 5000000 };
    for(;;) {
        // gum_check_events(win, 0);
        gum_event_loop(evm);

        // nanosleep(&ts, NULL);
    }
    gum_destroy_surface(win);
    // TODO -- Free cells and skins
    return 0;
}


