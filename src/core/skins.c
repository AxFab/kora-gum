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
#include <gum/cells.h>
#include "../utils/hmap.h"
#include <gum/css.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct GUM_skins {
    hmap_t map;
};

LIBAPI gum_skin_t *gum_skin_open(gum_skin_t* skin)
{
    if (skin == NULL) {
        skin = (GUM_skin*)calloc(1, sizeof(GUM_skin));
        skin->font_size = 10;
    }
    if (skin->read_only != 0) {
        GUM_skin* tmp = (GUM_skin*)calloc(1, sizeof(GUM_skin));
        memcpy(tmp, skin, sizeof(*tmp));
        if (skin->font_family)
            tmp->font_family = strdup(skin->font_family);
        skin = tmp;
        skin->read_only = 0;
    }
    return skin;
}

GUM_skin *gum_skin_property_setter(GUM_skin *skin, const char *property, const char *value)
{
    skin = gum_skin_open(skin);

    if (!strcmp("background", property))
        skin->bgcolor = css_parse_color(value);
    else if (!strcmp("background-gradient", property))
        skin->grcolor = css_parse_color(value);
    else if (!strcmp("shadow", property))
        skin->shcolor = css_parse_color(value);
    else if (!strcmp("color", property))
        skin->txcolor = css_parse_color(value);
    else if (!strcmp("border-color", property))
        skin->brcolor = css_parse_color(value);
    else if (!strcmp("border-size", property))
        skin->brsize = css_parse_usize(value);
    else if (!strcmp("gradient-angle", property))
        skin->grad_angle = strtol(value, NULL, 10);

    else if (!strcmp("border-top-left-radius", property))
        skin->r_top_left = css_parse_usize(value);
    else if (!strcmp("border-top-right-radius", property))
        skin->r_top_right = css_parse_usize(value);
    else if (!strcmp("border-bottom-left-radius", property))
        skin->r_bottom_left = css_parse_usize(value);
    else if (!strcmp("border-bottom-right-radius", property))
        skin->r_bottom_right = css_parse_usize(value);
    else if (!strcmp("border-radius", property)) {
        skin->r_top_left = css_parse_usize(value);
        skin->r_top_right = skin->r_bottom_right = skin->r_bottom_left = skin->r_top_left;
    }

    else if (!strcmp("width", property))
        skin->width = css_parse_usize(value);
    else if (!strcmp("height", property))
        skin->height = css_parse_usize(value);

    else if (!strcmp("font-family", property))
        skin->font_family = strdup(value);
    else if (!strcmp("font-size", property))
        skin->font_size = strtod(value, NULL); // TODO !

    else if (!strcmp("text-align", property)) {
        if (!strcmp("left", value))
            skin->align = 1;
        else if (!strcmp("right", value))
            skin->align = 2;
        else if (!strcmp("center", value))
            skin->align = 0;
    } else if (!strcmp("vertical-align", property)) {
        if (!strcmp("top", value))
            skin->align = 1;
        else if (!strcmp("bottom", value))
            skin->align = 2;
        else if (!strcmp("middle", value))
            skin->align = 0;
    }

    return skin;
}


void gum_skins_setter(GUM_skins *skins, const char *name, const char *property, const char *value)
{
    if (*name == '.')
        name = &name[1];
    int lg = strlen(name);
    GUM_skin *skin = hmp_get(&skins->map, name, lg);
    if (skin == NULL) {
        skin = (GUM_skin *)calloc(1, sizeof(GUM_skin));
        skin->font_size = 10;
        hmp_put(&skins->map, name, lg, skin);
    }

    gum_skin_property_setter(skin, property, value);
}


GUM_skins *gum_skins_loadcss(GUM_skins *skins, const char *filename)
{
    if (skins == NULL) {
        skins = (GUM_skins *)malloc(sizeof(GUM_skins));
        hmp_init(&skins->map, 16);
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
        return skins;

    css_read_file(fp, skins, (css_setter)&gum_skins_setter);
    fclose(fp);
    return skins;
}

GUM_skin *gum_style_find(GUM_skins *skins, const char *name)
{
    if (skins == NULL)
        return NULL;
    int lg = strlen(name);
    GUM_skin *skin = hmp_get(&skins->map, name, lg);
    return skin;
}

unsigned gum_mix(unsigned src, unsigned dest, float mx)
{
    unsigned char rs = (src & 0xFF0000) >> 16;
    unsigned char rd = (dest & 0xFF0000) >> 16;

    unsigned char gs = (src & 0xFF00) >> 8;
    unsigned char gd = (dest & 0xFF00) >> 8;

    unsigned char bs = (src & 0xFF);
    unsigned char bd = (dest & 0xFF);

    unsigned char r = (unsigned char)(rs + (rd - rs) * mx);
    unsigned char g = (unsigned char)(gs + (gd - gs) * mx);
    unsigned char b = (unsigned char)(bs + (bd - bs) * mx);
    return r << 16 | g << 8 | b;
}


void gum_reset_style(GUM_skins *skins, gum_cell_t*cell, const char *name)
{
    char sname[50];
    strcpy(sname, name);
    cell->skin = gum_style_find(skins, sname);
    strcpy(sname, name);
    strcat(sname, ":over");
    cell->skin_over = gum_style_find(skins, sname);
    strcpy(sname, name);
    strcat(sname, ":down");
    cell->skin_down = gum_style_find(skins, sname);
    cell->cachedSkin = NULL;
    gum_invalid_visual(gum_fetch_window(cell), cell);
}

void gum_skin_close(GUM_skin *skin)
{
    // TODO -- LRU or is registered
}

void gum_destroy_skins(GUM_skins *skins)
{
    // TODO -- Remove alls
    hmp_destroy(&skins->map);
}
