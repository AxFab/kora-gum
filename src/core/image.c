#include <kora/hmap.h>
#include <cairo/cairo.h>
#include <string.h>

HMP_map img_map;
int is_map_init = 0;

void *gum_load_image(const char *name)
{
    if (!is_map_init)
        hmp_init(&img_map, 16);

    cairo_surface_t *img;
    int lg = strlen(name);
    img = hmp_get(&img_map, name, lg);
    if (img != NULL)
        return img;

    img = cairo_image_surface_create_from_png(name);
    if (cairo_surface_status(img) != 0)
        return NULL;

    hmp_put(&img_map, name, lg, img);
    return img;
}

