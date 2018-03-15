// #include <kora/gum/display.h>
#include <kora/gum/rendering.h>
#include <kora/xml.h>
#include <kora/css.h>

struct GUM_cellbuilder
{
    GUM_cell *root;
    GUM_cell *cursor;
    GUM_skins *skins;
};

static void gum_cell_xmlattribute(GUM_cell *cell, const char *key, const char *value)
{
    if (!strcmp("id", key))
        cell->id = strdup(value);

    else if (!strcmp("left", key))
        cell->rulerx.bunit = css_parse_size(value, &cell->rulerx.before);
    else if (!strcmp("right", key))
        cell->rulerx.aunit = css_parse_size(value, &cell->rulerx.after);
    else if (!strcmp("horizontal-center", key))
        cell->rulerx.cunit = css_parse_size(value, &cell->rulerx.center);
    else if (!strcmp("min-width", key))
        cell->rulerx.munit = css_parse_usize(value, &cell->rulerx.min);
    else if (!strcmp("width", key)) {
        if (!strcmp("wrap", value)) {
            cell->rulerx.before = cell->rulerx.after = 0;
            cell->rulerx.bunit = cell->rulerx.aunit = CSS_SIZE_PX;
        } else {
            cell->rulerx.sunit = css_parse_usize(value, &cell->rulerx.size);
        }
    }

    else if (!strcmp("top", key))
        cell->rulery.bunit = css_parse_size(value, &cell->rulery.before);
    else if (!strcmp("bottom", key))
        cell->rulery.aunit = css_parse_size(value, &cell->rulery.after);
    else if (!strcmp("vertical-center", key))
        cell->rulery.cunit = css_parse_size(value, &cell->rulery.center);
    else if (!strcmp("min-height", key))
        cell->rulery.munit = css_parse_usize(value, &cell->rulery.min);
    else if (!strcmp("height", key)) {
        if (!strcmp("wrap", value)) {
            cell->rulery.before = cell->rulery.after = 0;
            cell->rulery.bunit = cell->rulery.aunit = CSS_SIZE_PX;
        } else {
            cell->rulery.sunit = css_parse_usize(value, &cell->rulery.size);
        }
    }

    else if (!strcmp("gap", key))
        cell->gunit = css_parse_usize(value, &cell->gap);
    else if (!strcmp("padding-left", key))
        cell->padding.lunit = css_parse_size(value, &cell->padding.left);
    else if (!strcmp("padding-right", key))
        cell->padding.runit = css_parse_size(value, &cell->padding.right);
    else if (!strcmp("padding-top", key))
        cell->padding.tunit = css_parse_size(value, &cell->padding.top);
    else if (!strcmp("padding-bottom", key))
        cell->padding.bunit = css_parse_size(value, &cell->padding.bottom);
    else if (!strcmp("padding-bottom", key)) {
        cell->padding.lunit = css_parse_size(value, &cell->padding.left);
        cell->padding.right = cell->padding.top = cell->padding.bottom = cell->padding.left;
        cell->padding.runit = cell->padding.tunit = cell->padding.bunit = cell->padding.lunit;
    }

    else if (!strcmp("text", key))
        cell->text = strdup(value);
    else if (!strcmp("layout", key)) {
        if (!strcmp("Absolute", value)) cell->layout = gum_layout_absolute;
        else if (!strcmp("Wrap", value)) cell->layout = gum_layout_wrap;
        else if (!strcmp("VGroupExtend", value)) cell->layout = gum_layout_vgroup_extend;
        else if (!strcmp("HGroupExtend", value)) cell->layout = gum_layout_hgroup_extend;
        else if (!strcmp("VGroupLeft", value)) cell->layout = gum_layout_vgroup_left;
        else if (!strcmp("HGroupTop", value)) cell->layout = gum_layout_hgroup_top;
        else if (!strcmp("VGroupCenter", value)) cell->layout = gum_layout_vgroup_center;
        else if (!strcmp("HGroupMiddle", value)) cell->layout = gum_layout_hgroup_middle;
        else if (!strcmp("VGroupRight", value)) cell->layout = gum_layout_vgroup_right;
        else if (!strcmp("HGroupBottom", value)) cell->layout = gum_layout_hgroup_bottom;
    }

    else if (!strcmp("editable", key)) {
        if (!strcmp("true", value)) cell->state |= GUM_CELL_EDITABLE;
        else if (!strcmp("false", value)) cell->state &= ~GUM_CELL_EDITABLE;
    } else if (!strcmp("solid", key)) {
        if (!strcmp("true", value)) cell->state |= GUM_CELL_SOLID;
        else if (!strcmp("false", value)) cell->state &= ~GUM_CELL_SOLID;
    } else if (!strcmp("overflow-x", key)) {
        if (!strcmp("true", value)) cell->state |= GUM_CELL_OVERFLOW_X;
        else if (!strcmp("false", value)) cell->state &= ~GUM_CELL_OVERFLOW_X;
    } else if (!strcmp("overflow-y", key)) {
        if (!strcmp("true", value)) cell->state |= GUM_CELL_OVERFLOW_Y;
        else if (!strcmp("false", value)) cell->state &= ~GUM_CELL_OVERFLOW_Y;
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
    GUM_cell *cell = (GUM_cell*)calloc(1, sizeof(GUM_cell));
    cell->id = strdup(node->node_name); // TODO -- Debug only
    if (builder->cursor) {
        gum_cell_pushback(builder->cursor, cell);
    } else {
        builder->root = cell;
    }

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
    if ((node->build_flags & XML_BLD_CLOSED)) {
        return cursor;
    }

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
