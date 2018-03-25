# Gum - Graphical User-interface Module

The Gum library is the default gui library for Kora system. It's provide a
realy basic interface for regular framed applications.

The library doesn't provide widgets like most commun UI library. Unstead it
provide an abstract to an even more basic element refrered to as a __cell__.

A cell is a rectangle store in a tree, for which may be associated a
background, a border or a text. Some others attributes are attached to each
cell in order to control their layout and behaviour.

Cells are the basic building block on which more complicated widgets are
created.


## Build a interface

A cell-tree, can be created programmaticaly or imported from an XML file,
allowing to specifiy a large variaty of parameters. In order to share some
properties, the skins (or style) of a cell is stored in a seperated
structure. Those can be pre-imported using a file with a CSS-like format.

The library provide some method to create compatible surface but this part is
delegate to an external library. By default, GUM use __cairo-graphics__ as
painter. In turn _Cairo_ required also an external library to send surface to
_a window manager_. It can be _xlib_ on linux, or _frame buffers_ on KoraOS.

The window manager used for the window will send back events to the
applications which will be converted by GUM event manager and will triggers
application routines.


Here's an exemple of a basic GUM application (C code only).

```c
void on_click(GUM_event_manager *evm, GUM_cell *cell, int event);

int main() {
    /* Load resources to create cell-tree */
    GUM_skins *skins = gum_skins_loadcss(NULL, "./resx/app.css");
    root = gum_cell_loadxml("./resx/app.xml", skins);
    if (root == NULL) {
        printf("Unable to create render model.\n");
        return -1;
    }

    /* Create a surface to use as window */
    GUM_surface *win = gum_create_surface(width, height);
    if (win == NULL) {
        printf("Unable to initialize window.\n");
        return -1;
    }

    /* The event manager use the surface to paint cells and transform
       user-interaction to events */
    GUM_event_manager *evm = gum_event_manager(root, win);

    /* Now, application can bind cell's events to application routines... */
    gum_event_bind(evm, gum_get_by_id(root, "my-button"), GUM_EV_CLICK, on_click);
    gum_event_bind(evm, gum_get_by_id(root, "close-button"), GUM_EV_CLICK, gum_on_close);

    gum_event_loop(evm);

    /* Release ressources */
    gum_destroy_manager(evm);
    gum_destroy_surface(win);
    gum_destroy_cells(root);
    gum_release_skins(skins);
    return 0;
}

```


### Cells XML format

 On a XML file, each DOM element correspond to a new cell. Note that no other
 XML node will be read (text node, comment or CDATA are all ignored by the loader).
 The tag-name of the element will be used to try matching a pre-defined skin.
 If not, no skins will be attached.

 Here's the list of attribute supported, with their type:

 - `id`: _string_. This filed should be uniq, as it will be used to identify the cell on the tree.
 - `left`: _css size_. For absolute layout, offset between the left of cell and left of container.
 - `right`: _css size_. For absolute layout, offset between the right of cell and right of container.
 - `horizontal-center`: _css size_. For absolute layout, offset between the center of cell and center of container.
 - `min-width`: _css unsigned_. Minimum width required to display the cell.
 - `width`: _css unsigned_ or _"wrap"_ keyword.
 - `top`: _css size_. For absolute layout, offset between the top of cell and top of container.
 - `bottom`: _css size_. For absolute layout, offset between the bottom of cell and bottom of container.
 - `vertical-center`: _css size_. For absolute layout, offset between the middle of cell and middle of container.
 - `min-height`: _css size_. Minimum height required to display the cell.
 - `height`: _css unsigned_ or _"wrap"_ keyword.
 - `gap-x`: _css size_.
 - `gap-y`: _css size_.
 - `gap`: Alias for both `gap-x` and `gap-y`.
 - `padding-left`: _css size_.
 - `padding-right`: _css size_.
 - `padding-top`: _css size_.
 - `padding-bottom`: _css size_.
 - `padding`: Alias for all size paddings.
 - `text`: _string_.
 - `img`: _string_. Name of the image to draw on the cell.
 - `layout`: _enum_. The name of the layout used to set children position and size.
 - `editable`: _boolean_. Tell the cell text might be edited using keyboard if the cell have focus.
 - `solid`: _boolean_. Tell the cell to react with pointer events.
 - `overflow-x`: _boolean_.
 - `overflow-y`: _boolean_.
 - `substyle`: This flag can be used for item which are not `solid` but for which the skin should change in case parent item skin does. If parent also use `substyle` propagate to next parent.

 Types:

 - _string_: Any text.
 - _enum_: A keyword amongst several possibilitiy.
 - _boolean_: Either true or false, other value have no effect.
 - _css size_: An double value follow by the unit used (`px`, `pt`, `mm`, `in`, `dp`, `%`);
 - _css unsigned_: Like a CSS size, but can only be positive.


## Layouts

Even if you can set position and size for each cell individualy chance are
you'll quickly wanna cells to oragnize themself automatically. The only way to
do that is to give them some rules to place themself. The library provide
several smart solutions that can place cells inside a container. Those
solution are called layouts. Here's the basic layout the library provide:

 - _Absolute_: Children cells are placed themself depending on the space
   given by their container cell.
 - _Groups_: At the number of 8. Cells can be placed in a single line or
   column, each one after an other. Ideal for list, toolbars, navbars...
 - _Grids_: Cell are placed in line or columns, but once the place is fulled,
   next cells will be placed beside, creating a sort of table. Options
   provide up to 6 different layouts.

 > Those pretty straigth-forward layout defined how a cell-container place
 > their direct children. This allow for a lot configuration but isn't flexible
 > enough to provide a full HTML like layout -- probably the most complex out-there.

Each of those layout might use some cells-attribute to place their childrens.
Note that distances in GUM can be expressed in 6 different units:

 - in pixels `px`: Give the final pixels length.
 - in points `pt`: Depend of resolution we can place 72 points in one inch (2.54cm).
 - in points `mm`: Depend of resolution will match 1 mm on the screen.
 - in points `in`: Depend of resolution will match 1 inch on the screen.
 - in points `dp`: Depend of viewport settings, used to take grossly-around some space on screen depending on device (TV, mobile...) from 0.75 to 5 pixels.
 - in points `%`: Is relative to parent size.

### Absolute

This layout give freedom to children cells to place themself whereever in the
container area. This means children attribute will be used to place themself.
A group of five attribute are used for horizonal positioning, sames for
vertical. We will take horizontal exemple.

You can fix the position of the left side relative to the left of the
container. You can also fix the position of the right side. Positive values
for this attribute will always place you inside the container (meaning left
and right offsets are reversed). You can also specify the width of the item,
which will be use only if one of the left or right attirbute is not used.
Min-width attribute will however always be used. In case those attributes
aren't enough we look at the last attribute center which give an offset
between the center of the cells and the center of the containers.

Same attributes exists for vertical positioning (using top, bottom and height).


### Groups and Grids

Declinaision of those layouts are made to control the size of the children.

For vertical groups, item can be place at the left, the right, centered or
fillout all the width available. Same options exists for horizontal group
(with top or bottom).

Grid give three choice for chidren size, either each item take the minium size,
or their all take the size of the largest item, or a compromise they take the
size of the largest item on the same row.


## Manipulating the cell tree




## Play with Skins

 - `background`: -
 - `shadow`: _Work-in-progress_
 - `color`: -
 - `border-color`: -
 - `border-color`: -
 - `border-top-left-radius`: -
 - `border-top-right-radius`: -
 - `border-bottom-left-radius`: -
 - `border-bottom-right-radius`: -
 - `border-radius`: Alias for all `border-*-*-radius`.
 - `width`: -
 - `height`: -
 - `text-align`: -
 - `vertical-align`: -

 > At this moment, background only support one color. I don't plan in
 > supporting image as they should use cell attributes. However I add support
 > for gradient. For the moment they can only be used with temporary
 > attributes `background-gradient` and `gradient-angle`.


