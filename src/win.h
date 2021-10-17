/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2021  <Fabien Bavent>
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
#ifndef _SRC_WIN_H
#define _SRC_WIN_H 1

#include <gum/cells.h>
#include <gum/events.h>
#include "hmap.h"


struct gum_window {
    //void* pixels;
    //int width;
    //int height;
    //int pitch;

    //int dpi_x;
    //int dpi_y;
    //float dsp_x;
    //float dsp_y;

    gum_cell_t* root;
    gum_cell_t* menus[GUM_MAX_MCTX];
    int menu_sp;

    hmap_t actions;

    gum_cell_t* over;
    gum_cell_t* down;
    gum_cell_t* focus;
    gum_cell_t* click;
    gum_cell_t* edit;
    gum_cell_t* grab;

    int mouse_x, mouse_y;
    int grab_x, grab_y;
    int click_cnt;
    int spec_btn;
    long long last_click;

    bool measure;
    gum_cell_t* layout;

    GUM_rect inval;

    GUM_gctx ctx;
    void* data;
};



#endif /* _SRC_WIN_H */
