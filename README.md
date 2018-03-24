# Internal

## Render Tree

  Our render tree is composed of 3-level of metadata which allow to store all
  needed properties. Each levels have been introduced for factorisation
  purpose as several elements of one level can refer to the same element of
  the next.

  This tree is composed of `SWN_cell` which represent a small rectangle area
  on the screen.
  Those cells reprsent the first level of the rending tree and we create a
  new instance of them for every graphics representation. Note that some
  cells, mainly those being used as container, might not draw any pixel on
  the screen.

  Those cells contains only 2 major informations: the current size or box
  model, and the value beeing either a text or a picture. The second will
  often varies with the excution of the program and might only be initialize
  by metadata.

  The box model for its part can't be updated by the user but
  will resond to countless-infromation: changes on the cells-model, user
  interaction, window resize... The value computed on the box-model depend of
  two components, the layout and the underlying model.

  The layout is the alorithm used to place cell in relation to each others.
  The library use by default an aboslute model that allow child cell to place
  themself into the client area. The library propose also several layout that
  allow the parent to decide the place of its children cells.

  > Note that for the time beeing, the layouts are only capable of placing
  direct children. This allow for a lot configuration but isn't flexible
  enough to provide a full-HTML-layout -- probably the most complex layout
  out-there.

  The underlying model is a different structure named `SWN_model` which
  represent the second level for the render tree.

  The model find it's utility especialy in case of reusable widgets. Each
  widget will make use of the same model tree.

  [...]

### Copy on write

  Skin should be most of the time loaded from a CSS file. Each CSS class can
  be refered by a model element. If a model don't have any skin class specify
  it will create its own.

  Loaded skin are accessible as resources but are marked as readonly. A model
  that request a change into his skin class will make a copy first.

  The same logic apply for model used by cell. Model however can be place
  under a hierachy. However this hierarchy only apply for cell facotry. Once
  the cell have been created, each model item will act independently.


