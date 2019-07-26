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
#include <string.h>

extern GUM_skins *global_skins;


static void gum_widget_on_box_click(GUM_event_manager *evm, GUM_cell *cell, int event, GUM_widget *widget)
{
    int btn = (widget->mode & ~7) >> 3;
    int txt = widget->mode & 3;
    if (txt == 2) {
        printf("Select text\n");
        gum_set_focus(evm, &widget->txt);
    } else if (btn == 0)
        printf("DoAction\n");
    else
        printf("DoAction or Menu\n");
}


static void gum_widget_on_txt_click(GUM_event_manager *evm, GUM_cell *cell, int event, GUM_widget *widget)
{
    int txt = widget->mode & 3;
    if (txt != 2)
        return;
    printf("Select text\n");
    gum_set_focus(evm, &widget->txt);
}

static void gum_widget_on_btn_click(GUM_event_manager *evm, GUM_cell *cell, int event, GUM_widget *widget)
{
    int btn = (widget->mode & ~7) >> 3;
    if (btn < 2)
        printf("DoAction or Menu\n");
    else
        printf("Increment\n");
}

static void gum_widget_on_btn2_click(GUM_event_manager *evm, GUM_cell *cell, int event, GUM_widget *widget)
{
    int btn = (widget->mode & ~7) >> 3;
    if (btn < 2)
        return;
    printf("Decrement\n");
}

GUM_widget *gum_widget_allocate(GUM_cell *parent, GUM_skins *skins)
{
    GUM_widget *widget = calloc(1, sizeof(GUM_widget));
    widget->skins = skins;

    gum_cell_pushback(parent, &widget->box);
    gum_cell_pushback(&widget->box, &widget->trk);
    gum_cell_pushback(&widget->box, &widget->tra);
    gum_cell_pushback(&widget->box, &widget->bup);
    gum_cell_pushback(&widget->box, &widget->bdw);
    gum_cell_pushback(&widget->box, &widget->txt);
    gum_cell_pushback(&widget->box, &widget->ico);

    widget->box.skin = gum_style_find(skins, "Button");
    widget->box.skin_over = gum_style_find(skins, "Button:over");
    widget->box.skin_down = gum_style_find(skins, "Button:down");
    widget->trk.skin = gum_style_find(skins, "Button-tray");
    widget->tra.skin = gum_style_find(skins, "Button-tray-green");

    widget->bup.skin = gum_style_find(skins, "Button-arrow");
    widget->bup.skin_over = gum_style_find(skins, "Button-arrow:over");
    widget->bup.skin_down = gum_style_find(skins, "Button-arrow:down");
    widget->bdw.skin = gum_style_find(skins, "Button-arrow");
    widget->bdw.skin_over = gum_style_find(skins, "Button-arrow:over");
    widget->bup.skin_down = gum_style_find(skins, "Button-arrow:down");

    widget->txt.skin = gum_style_find(skins, "Button-text");
    widget->txt.skin_over = gum_style_find(skins, "Button-text:over");
    widget->bup.skin_down = gum_style_find(skins, "Button-arrow:down");
    widget->ico.skin = gum_style_find(skins, "Button-img");

    GUM_event_manager *evm = gum_fetch_manager(parent);
    gum_event_bind(evm, &widget->box, GUM_EV_CLICK, (void *)gum_widget_on_box_click, widget);
    gum_event_bind(evm, &widget->txt, GUM_EV_CLICK, (void *)gum_widget_on_txt_click, widget);
    gum_event_bind(evm, &widget->bup, GUM_EV_CLICK, (void *)gum_widget_on_btn_click, widget);
    gum_event_bind(evm, &widget->bdw, GUM_EV_CLICK, (void *)gum_widget_on_btn2_click, widget);

    return widget;
}

void gum_widget_reskin(GUM_widget *widget, int mode, bool bg)
{
    // Settings
    int icoSize = 16;
    int btnSize = 10;
    int btnPad = 2;
    int trackHeight = 4;

    int btnHeight = icoSize + btnPad * 2;
    int txtBefore = 0;
    int txtAfter = 0;

    // Is the display valid
    int txt = mode & 3;
    int btn = (mode & ~7) >> 3;
    bool ico = mode & 4 ? true : false;
    if (txt >= 3 || btn >= 5)
        return;
    if (!ico && txt == 0)
        return;
    if (btn > 2 && txt != 2)
        return;
    if (btn == 2 && mode != 17 && mode != 20)
        return;

    widget->mode = mode;
}

void gum_widget_set_text(GUM_widget *widget, const char *value)
{
    if (widget->txt.text)
        free(widget->txt.text);
    widget->txt.text = strdup(value);
    // TODO -- Invalid min size!
}

GUM_widget *gum_create_widget(GUM_container *parent, const char *type, const char *text/*, menu, action*/)
{
    GUM_widget *widget = gum_widget_allocate(parent->group, global_skins);
    if (strcmp(type, "Label") == 0)
        gum_widget_reskin(widget, 1, false);
    else if (strcmp(type, "ComboBox") == 0)
        gum_widget_reskin(widget, 10, true);
    else if (strcmp(type, "CheckBox") == 0)
        gum_widget_reskin(widget, 5, false);
    else if (strcmp(type, "RadioButton") == 0)
        gum_widget_reskin(widget, 5, false);
    else if (strcmp(type, "PushButton") == 0)
        gum_widget_reskin(widget, 1, true);
    else if (strcmp(type, "TextEdit") == 0)
        gum_widget_reskin(widget, 2, true);
    else if (strcmp(type, "SpinBox") == 0)
        gum_widget_reskin(widget, 34, true);
    else if (strcmp(type, "ProgressBar") == 0)
        gum_widget_reskin(widget, 17, false);
    else if (strcmp(type, "Slider") == 0) {
        gum_widget_reskin(widget, 20, false);
        text = NULL;
    }

    if (widget && text)
        gum_widget_set_text(widget, text);

    return widget;
}


