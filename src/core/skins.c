#include <kora/gum/cells.h>
#include <kora/hmap.h>
#include <kora/css.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct GUM_skins
{
    HMP_map map;
};

GUM_skin* gum_skin_property_setter(GUM_skin *skin, const char* property, const char* value)
{
    if (skin->read_only) {
        GUM_skin *tmp = (GUM_skin*)calloc(1, sizeof(GUM_skin));
        memcpy(tmp, skin, sizeof(*tmp));
        skin = tmp;
        skin->read_only = 0;
    }

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
    else if (!strcmp("gradient-angle", property))
        skin->grad_angle = strtol(value, NULL, 10);

    else if (!strcmp("border-top-left-radius", property))
        skin->u_top_left = css_parse_usize(value, &skin->r_top_left);
    else if (!strcmp("border-top-right-radius", property))
        skin->u_top_right = css_parse_usize(value, &skin->r_top_right);
    else if (!strcmp("border-radius", property)) {
        skin->u_top_left = css_parse_usize(value, &skin->r_top_left);
        skin->r_top_right = skin->r_bottom_right = skin->r_bottom_left = skin->r_top_left;
        skin->u_top_right = skin->u_bottom_right = skin->u_bottom_left = skin->u_top_left;
    }

    else if (!strcmp("width", property))
        skin->wunit |= css_parse_usize(value, &skin->width);
    else if (!strcmp("height", property))
        skin->hunit |= css_parse_usize(value, &skin->height);

    else if (!strcmp("text-align", property)) {
        if (!strcmp("left", value)) skin->align = 1;
        else if (!strcmp("right", value)) skin->align = 2;
        else if (!strcmp("center", value)) skin->align = 0;
    } else if (!strcmp("vertical-align", property)) {
        if (!strcmp("top", value)) skin->align = 1;
        else if (!strcmp("bottom", value)) skin->align = 2;
        else if (!strcmp("middle", value)) skin->align = 0;
    }

    return skin;
}


void gum_skins_setter(GUM_skins *skins, const char* name, const char* property, const char* value)
{
    if (*name == '.') {
        name = &name[1];
    }
    int lg = strlen(name);
    GUM_skin *skin = hmp_get(&skins->map, name, lg);
    if (skin == NULL) {
        skin = (GUM_skin*)calloc(1, sizeof(GUM_skin));
        hmp_put(&skins->map, name, lg, skin);
    }

    gum_skin_property_setter(skin, property, value);
}


GUM_skins *gum_skins_loadcss(GUM_skins *skins, const char *filename)
{
    if (skins == NULL) {
        skins = (GUM_skins*)malloc(sizeof(GUM_skins));
        hmp_init(&skins->map, 16);
    }

    FILE *fp = fopen(filename, "r");
        if (fp == NULL) {
            return skins;
    }

    css_read_file(fp, skins, (css_setter)&gum_skins_setter);
    fclose(fp);
    return skins;
}

GUM_skin *gum_style_find(GUM_skins *skins, const char* name)
{
    int lg = strlen(name);
    GUM_skin *skin = hmp_get(&skins->map, name, lg);
    return skin;
}