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


GUM_window *gum_create_surface(int width, int height)
{
    return NULL;
}

void gum_destroy_surface(GUM_window *win)
{
}

void gum_invalid_surface(GUM_window *win, int x, int y, int w, int h)
{
}

void gum_draw_cell(GUM_window *win, GUM_cell *cell)
{
}

void gum_text_size(const char *text, int *w, int *h, GUM_skin *skin)
{
}

void gum_draw_scrolls(GUM_window *win, GUM_cell *cell)
{
}

int gum_event_poll(GUM_window *win, GUM_event *event, int timeout)
{
    return 0;
}

void gum_reset_clip(GUM_window *win)
{
}

void gum_push_clip(GUM_window *win, GUM_box *box)
{
}

void gum_pop_clip(GUM_window *win, GUM_box *box)
{
}

void gum_painter(GUM_window *win, GUM_cell *root)
{
    gum_paint(win, root);
}

void gum_resize_win(GUM_window *win, int width, int height)
{
}

