#include <kora/gum/display.h>
#include <kora/gum/rendering.h>
#include <kora/gum/events.h>
#include <kora/xml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>


/* Graphical User-interface Module */
int main ()
{
    int width = 680;
    int height = width * 10 / 16; // 425

    // Load models
    GUM_skins *skins = gum_skins_loadcss(NULL, "./resx/browser/app.css");
    GUM_cell *cell = gum_cell_loadxml("./resx/browser/app.xml", skins);
    if (cell == NULL) {
        printf("Unable to create render model: %s.\n", gum_error());
        return -1;
    }

    // Open Window
    GUM_surface *win = gum_surface(NULL, width, height, 0, 0);
    if (win == NULL) {
        printf("Unable to initialize window: %s.\n", gum_error());
        return -1;
    }

    GUM_cell *ctx_view = gum_get_by_id(cell, "ctx-menu-view");
    GUM_cell *ctx_file = gum_get_by_id(cell, "ctx-menu-file");
    gum_cell_dettach(ctx_view);
    gum_cell_dettach(ctx_file);

    GUM_cell *view = gum_get_by_id(cell, "view");
    GUM_cell *icon = gum_get_by_id(cell, "icon");
    GUM_cell *ico_txt = gum_get_by_id(icon, "icon-text");
    GUM_cell *ico_img = gum_get_by_id(icon, "icon-img");
    gum_cell_dettach(icon);

    void *dir = opendir("/home/fabien");
    for (;;) {
        struct dirent *en = readdir(dir);
        if (en == NULL)
            break;
        fprintf(stderr, "Dirent %s\n", en->d_name);
        free(ico_txt->text);
        ico_txt->text = strdup(en->d_name);
        GUM_cell *cpy = gum_cell_copy(icon);
        gum_cell_pushback(view, cpy);
    }
        fprintf(stderr, "R4\n");
    closedir(dir);


    GUM_event_manager * evm = gum_event_manager(cell, win);
    // TODO -- bind event ! manager, CellById(root, "btn-previous"), GUM_CE_CLICK, onPrevious);
    gum_event_loop(evm);
    gum_close_surface(win);
    // TODO -- Free cells and skins
    return 0;
}
