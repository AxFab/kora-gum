/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2018  <Fabien Bavent>
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
#include <kora/xml.h>
#include <kora/css.h>

struct GUM_cellbuilder {
    GUM_cell *root;
    GUM_cell *cursor;
    GUM_skins *skins;
};

GUM_layout_algo gum_fetch_layout(const char *value)
{
    if (!strcmp("Absolute", value))
        return gum_layout_absolute;
    else if (!strcmp("Wrap", value))
        return gum_layout_wrap;
    else if (!strcmp("VGroupExtend", value))
        return gum_layout_vgroup_extend;
    else if (!strcmp("HGroupExtend", value))
        return gum_layout_hgroup_extend;
    else if (!strcmp("VGroupLeft", value))
        return gum_layout_vgroup_left;
    else if (!strcmp("HGroupTop", value))
        return gum_layout_hgroup_top;
    else if (!strcmp("VGroupCenter", value))
        return gum_layout_vgroup_center;
    else if (!strcmp("HGroupMiddle", value))
        return gum_layout_hgroup_middle;
    else if (!strcmp("VGroupRight", value))
        return gum_layout_vgroup_right;
    else if (!strcmp("HGroupBottom", value))
        return gum_layout_hgroup_bottom;
    else if (!strcmp("ColumnGrid", value))
        return gum_layout_column_grid;
    else if (!strcmp("RowGrid", value))
        return gum_layout_row_grid;
    else if (!strcmp("Grid", value))
        return gum_layout_fixgrid;
    return NULL;
}

static void gum_cell_xmlattribute(GUM_cell *cell, const char *key, const char *value)
{
    if (!strcmp("id", key))
        cell->id = strdup(value);

    else if (!strcmp("left", key)) {
        if (value[0] == '{') {
            CSS_SET_PX(cell->rulerx.before.unit, 0);
            cell->rell = strdup(&value[1]);
            strchr(cell->rell, '}') [0] = '\0';
        } else
            cell->rulerx.before = css_parse_size(value);
    } else if (!strcmp("right", key)) {
        if (value[0] == '{') {
            CSS_SET_PX(cell->rulerx.after.unit, 0);
            cell->relr = strdup(&value[1]);
            strchr(cell->relr, '}') [0] = '\0';
        } else
            cell->rulerx.after = css_parse_size(value);
    } else if (!strcmp("horizontal-center", key))
        cell->rulerx.center = css_parse_size(value);
    else if (!strcmp("min-width", key))
        cell->rulerx.min = css_parse_usize(value);
    else if (!strcmp("width", key)) {
        if (!strcmp("wrap", value)) {
            CSS_SET_PX(cell->rulerx.before, 0);
            cell->rulerx.after = cell->rulerx.before;
        } else
            cell->rulerx.size = css_parse_usize(value);
    }

    else if (!strcmp("top", key))
        cell->rulery.before = css_parse_size(value);
    else if (!strcmp("bottom", key))
        cell->rulery.after = css_parse_size(value);
    else if (!strcmp("vertical-center", key))
        cell->rulery.center = css_parse_size(value);
    else if (!strcmp("min-height", key))
        cell->rulery.min = css_parse_usize(value);
    else if (!strcmp("height", key)) {
        if (!strcmp("wrap", value)) {
            CSS_SET_PX(cell->rulery.before, 0);
            cell->rulery.after = cell->rulery.before;
        } else
            cell->rulery.size = css_parse_usize(value);
    }

    else if (!strcmp("gap-x", key))
        cell->gap_x = css_parse_usize(value);
    else if (!strcmp("gap-y", key))
        cell->gap_y = css_parse_usize(value);
    else if (!strcmp("gap", key)) {
        cell->gap_x = css_parse_usize(value);
        cell->gap_y = cell->gap_x;
    }

    else if (!strcmp("padding-left", key))
        cell->padding.left = css_parse_size(value);
    else if (!strcmp("padding-right", key))
        cell->padding.right = css_parse_size(value);
    else if (!strcmp("padding-top", key))
        cell->padding.top = css_parse_size(value);
    else if (!strcmp("padding-bottom", key))
        cell->padding.bottom = css_parse_size(value);
    else if (!strcmp("padding", key)) {
        cell->padding.left = css_parse_size(value);
        cell->padding.right = cell->padding.top = cell->padding.bottom = cell->padding.left;
    }

    else if (!strcmp("text", key))
        cell->text = strdup(value);
    else if (!strcmp("img", key))
        cell->img_src = strdup(value);
    else if (!strcmp("layout", key))
        cell->layout = gum_fetch_layout(value);
    else if (!strcmp("editable", key)) {
        if (!strcmp("true", value))
            cell->state |= GUM_CELL_EDITABLE;
        else if (!strcmp("false", value))
            cell->state &= ~GUM_CELL_EDITABLE;
    } else if (!strcmp("solid", key)) {
        if (!strcmp("true", value))
            cell->state |= GUM_CELL_SOLID;
        else if (!strcmp("false", value))
            cell->state &= ~GUM_CELL_SOLID;
    } else if (!strcmp("hidden", key)) {
        if (!strcmp("true", value))
            cell->state |= GUM_CELL_HIDDEN;
        else if (!strcmp("false", value))
            cell->state &= ~GUM_CELL_HIDDEN;
    } else if (!strcmp("buffered", key)) {
        if (!strcmp("true", value))
            cell->state |= GUM_CELL_BUFFERED;
        else if (!strcmp("false", value))
            cell->state &= ~GUM_CELL_BUFFERED;
    } else if (!strcmp("overflow-x", key)) {
        if (!strcmp("true", value))
            cell->state |= GUM_CELL_OVERFLOW_X;
        else if (!strcmp("false", value))
            cell->state &= ~GUM_CELL_OVERFLOW_X;
    } else if (!strcmp("overflow-y", key)) {
        if (!strcmp("true", value))
            cell->state |= GUM_CELL_OVERFLOW_Y;
        else if (!strcmp("false", value))
            cell->state &= ~GUM_CELL_OVERFLOW_Y;
    } else if (!strcmp("substyle", key)) {
        if (!strcmp("true", value))
            cell->state |= GUM_CELL_SUBSTYLE;
        else if (!strcmp("false", value))
            cell->state &= ~GUM_CELL_SUBSTYLE;
    } else if (!strcmp("dragable", key)) {
        if (!strcmp("true", value))
            cell->state |= GUM_CELL_DRAGABLE;
        else if (!strcmp("false", value))
            cell->state &= ~GUM_CELL_DRAGABLE;
    }

    else {
        // TODO - Avoid copy is not necessary, - how to change over/down skins
        // cell->skin = gum_skin_property_setter(cell->skin, key, value);
    }
}

static XML_node *gum_cell_xmlnode(XML_node *cursor, XML_node *node, struct GUM_cellbuilder *builder)
{
    if (node->type != XML_ELEMENT) {
        // Nothing special for node other than elements
        xml_add_child_node(cursor, node);
        return cursor;
    } else if (node->build_flags & XML_BLD_CLOSING) {
        // Close and free nodes as we go (low memory reading)
        node = cursor->parent;
        xml_remove_node(cursor);
        xml_free_node(cursor);
        builder->cursor = builder->cursor->parent;
        return node;
    }

    // Create a new cell and attach it to the tree
    GUM_cell *cell = (GUM_cell *)calloc(1, sizeof(GUM_cell));
    cell->name = strdup(node->node_name);
    cell->depth = builder->cursor ? builder->cursor->depth + 1 : 0;
    if (builder->cursor)
        gum_cell_pushback(builder->cursor, cell);

    else
        builder->root = cell;

    // TODO -- Find a skin for this cell
    char name[50];
    cell->skin = gum_style_find(builder->skins, node->node_name);
    strcpy(name, node->node_name);
    strcat(name, ":over");
    cell->skin_over = gum_style_find(builder->skins, name);
    strcpy(name, node->node_name);
    strcat(name, ":down");
    cell->skin_down = gum_style_find(builder->skins, name);


    // Read attributes
    XML_attribute *att = node->first_attribute;
    while (att) {
        gum_cell_xmlattribute(cell, att->key, att->value);
        att = att->next;
    }

    // Continue XML process
    xml_add_child_node(cursor, node);
    if ((node->build_flags & XML_BLD_CLOSED))
        return cursor;

    builder->cursor = cell;
    return node;
}

GUM_cell *gum_cell_loadxml(const char *filename, GUM_skins *skins)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
        return NULL;

    struct GUM_cellbuilder builder;
    builder.cursor = NULL;
    builder.skins = skins;
    xml_read_file_with(fp, (XML_pusher)gum_cell_xmlnode, &builder);
    fclose(fp);
    return builder.root;
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

#include <kora/hmap.h>

HMP_map img_map;
int is_map_init = 0;
void *gum_image(const char *name)
{
    if (!is_map_init) {
        hmp_init(&img_map, 16);
        is_map_init = 1;
    }
    int lg = strlen(name);
    void *img = hmp_get(&img_map, name, lg);
    if (img != NULL)
        return img;

    img = gum_load_image(name);
    if (img != NULL)
        hmp_put(&img_map, name, lg, img);
    return img;
}
