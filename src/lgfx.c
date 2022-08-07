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
#include <gum/events.h>
#include <gum/cells.h>
#include <gum/xml.h>
#include <gum/css.h>
#include <gfx.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include "mcrs.h"
#include "win.h"

#define __USE_CAIRO 1
// #define __USE_FT 1
#define __USE_FT_CAIRO 1

#define MIN_ALPHA 0x1000000
#define MAX_ALPHA 0xFF000000
#define M_PI 3.141592653589793
#define GET_X_LPARAM(s)   ((int)(short)((int32_t)(s) & 0xffff))
#define GET_Y_LPARAM(s)   ((int)(short)(((int32_t)(s) >> 16) & 0xffff))

#define RESX "C:/Users/Aesga/develop/kora-disto/sources/desktop/resx"


#ifdef __USE_CAIRO
#include <cairo.h>
#include <cairo-lgfx.h>
#endif

#if defined(__USE_FT) || defined(__USE_FT_CAIRO)
#include <ft2build.h>
#include <freetype/freetype.h>
#ifndef FT_LOAD_DEFAULT
# define FT_LOAD_DEFAULT 0
#endif
#endif



/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void* memrchr(const void* buf, int byte, size_t len)
{
    const char* ptr = (const char*)buf + len;
    while (ptr-- > (const char*)buf)
        if (*ptr == byte)
            return (void*)ptr;
    return NULL;
}


long long gum_system_time()
{
    clock_t ticks = clock();
    ticks *= _PwNano_ / CLOCKS_PER_SEC;
    return ticks;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

bool timerActive = false;

#ifdef __USE_FT

FT_Library ftLibrary;
bool ftLibraryInitialized = false;

typedef struct gfx_text gfx_text_t;
struct gfx_text {
    int width;
    int height;
    int x_bearing;
    int y_bearing;
};

void gfx_copy_glyph(gfx_t* gfx, gfx_clip_t* clip, FT_Bitmap* glyph, int x, int y, uint32_t fontcolor)
{
    int i, j;
    int minx = MAX(x, clip->left);
    int maxx = MIN(x + glyph->width, clip->right);
    int miny = MAX(y, clip->top);
    int maxy = MIN(y + glyph->rows, clip->bottom);

    for (j = miny; j < maxy; ++j) {
        for (i = minx; i < maxx; ++i) {
            uint8_t val = glyph->buffer[(j - y) * glyph->pitch + (i - x)];
            uint32_t color = (fontcolor & 0xffffff) | (val << 24);
            uint32_t* dst = &gfx->pixels4[j * gfx->width + i];
            uint32_t nw = gfx_upper_alpha_blend(*dst, color);
            *dst = nw;
        }
    }
}

//void gfx_measure_text(FT_Face face, const char* text, gfx_text_t* measures)
//{
//    if (face == NULL)
//        return;
//    int i;
//    memset(measures, 0, sizeof(*measures));
//    FT_GlyphSlot  slot = face->glyph;
//    int miny = 0, maxy = 0;
//    for (i = 0; ; ++i) {
//        int ch;
//        int len = mbtouc(&ch, &text[i], 6);
//        i += len - 1;
//        if (ch == 0)
//            break;
//
//        FT_UInt  glyph_index;
//        glyph_index = FT_Get_Char_Index(face, ch);
//        FT_Error error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
//        if (error)
//            continue;
//
//        error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
//        if (error)
//            continue;
//
//        measures->width += slot->bitmap_left + slot->advance.x >> 6;
//        if (miny > -slot->bitmap_top)
//            miny = -slot->bitmap_top;
//        if (maxy < slot->bitmap.rows - slot->bitmap_top)
//            maxy = slot->bitmap.rows - slot->bitmap_top;
//    }
//    measures->height = maxy - miny;
//    measures->y_bearing = miny;
//}

void gfx_ft_write_text(gfx_t* gfx, FT_Face face, const char* text, gfx_clip_t* clip, uint32_t txcolor, int align, int valign)
{
    if (face == NULL)
        return;
    FT_Error error;
    gfx_text_t measures;

    int i;
    int pen_x = clip->left;
    int pen_y = clip->top;
    int box_w = clip->right - clip->left;
    int box_h = clip->bottom - clip->top;

    FT_GlyphSlot  slot = face->glyph;
    gfx_measure_text(face, text, &measures);

    if (align == 2)
        pen_x += box_w - (measures.width + measures.x_bearing);
    else if (align == 0)
        pen_x += box_w / 2 - (measures.width / 2 + measures.x_bearing);

    if (valign == 2)
        pen_y += box_h - (measures.height + measures.y_bearing);
    else if (valign == 0)
        pen_y += box_h / 2 - (measures.height / 2 + measures.y_bearing);

    for (i = 0; ; ++i) {
        int ch;
        int len = mbtouc(&ch, &text[i], 6);
        i += len - 1;
        if (ch == 0)
            break;
        if (len > 1)
            len++;

        FT_UInt  glyph_index;
        glyph_index = FT_Get_Char_Index(face, ch);
        error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
        if (error)
            continue;

        /* convert to an anti-aliased bitmap */
        error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
        if (error)
            continue;

        // COPY !!!
        gfx_copy_glyph(gfx, clip, &slot->bitmap, pen_x + slot->bitmap_left, pen_y - slot->bitmap_top, txcolor);


        /* increment pen position */
        pen_x += slot->advance.x >> 6;
        pen_y += slot->advance.y >> 6;
    }
}
#endif



typedef struct gum_gfx_data gum_gfx_data_t;

struct gum_gfx_data {
    int x, y;
    int px, py;
    gfx_t* gfx;
    // gfx_clip_t clip;
#ifdef __USE_CAIRO
    cairo_t* cr;
    cairo_surface_t* srf;
#endif
};

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

LIBAPI void gum_lgfx_setup(gum_window_t* win, gfx_t* gfx, gfx_clip_t* vw)
{
    gum_gfx_data_t* ctx = malloc(sizeof(gum_gfx_data_t));
    ctx->gfx = gfx;
    ctx->x = vw ? vw->left : 0;
    ctx->y = vw ? vw->top : 0;

    win->ctx.width = vw ? vw->right - vw->left : gfx->width;
    win->ctx.height = vw ? vw->bottom - vw->top : gfx->height;

#ifdef __USE_CAIRO
    ctx->srf = cairo_lgfx_surface(gfx, vw);
    ctx->cr = cairo_create(ctx->srf);
#endif
#ifdef __USE_FT
    if (!ftLibraryInitialized) {
        FT_Error err = FT_Init_FreeType(&ftLibrary);
        if (!err)
            ftLibraryInitialized = true;
    }
#endif
    win->data = ctx;
}

void gum_lgfx_destroy(gum_window_t* win)
{
    gum_gfx_data_t* ctx = win->data;
#ifdef __USE_CAIRO
    cairo_destroy(ctx->cr);
    cairo_surface_destroy(ctx->srf);
#endif
    free(ctx);
}


LIBAPI void gum_gfx_handle(gum_window_t* win, gfx_msg_t* msg)
{
    gum_gfx_data_t* ctx = win->data;
    switch (msg->message) {
    case GFX_EV_MOUSEMOVE:
        gum_event_motion(win, GET_X_LPARAM(msg->param1) - ctx->x, GET_Y_LPARAM(msg->param1) - ctx->y);
        break;
    case GFX_EV_BTNDOWN:
        if (msg->param1 == 1)
            gum_event_left_press(win);
        else
            gum_event_button_press(win, msg->param1);
        break;
    case GFX_EV_BTNUP:
        if (msg->param1 == 1)
            gum_event_left_release(win);
        else
            gum_event_button_release(win, msg->param1);
        break;
    case GFX_EV_MOUSEWHEEL:
        gum_event_wheel(win, msg->param1 > 0 ? 20 : -20);
        break;

    case GFX_EV_KEYDOWN:
        // gum_event_key_release();
    case GFX_EV_KEYUP:
    case GFX_EV_KEYPRESS:
        // gum_event_key_press(win, 0, msg->param1);
    case GFX_EV_RESIZE:
    case GFX_EV_QUIT:
    case GFX_EV_TIMER:
        break;

    case GUM_EV_ASYNC:
        gum_event_async(win, (void*)msg->param1);
        break;

    default:
        printf("Unsupported\n");
        break;
    }
}

void gum_push_event(gum_window_t* win, int type, size_t param)
{
    gum_gfx_data_t* ctx = win->data;
    gfx_push(ctx->gfx, type, param, 0);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

#if 0

void gum_start_paint(gum_window_t* win)
{
}

void gum_end_paint(gum_window_t* win) {}

void gum_push_clip(gum_window_t* win, GUM_box* box)
{
    gum_gfx_data_t* ctx = win->data;
    ctx->clip.left += box->cx - box->sx;
    ctx->clip.top += box->cy - box->sy;
}

void gum_pop_clip(gum_window_t* win, GUM_box* box, GUM_box* prev)
{
    gum_gfx_data_t* ctx = win->data;
    ctx->clip.left -= box->cx - box->sx;
    ctx->clip.top -= box->cy - box->sy;
}

void gum_draw_cell(gum_window_t* win, gum_cell_t* cell)
{
    gum_gfx_data_t* ctx = win->data;
    GUM_skin* skin = gum_skin(cell);
    if (skin == NULL)
        return;

    if (cell->cachedSkin != skin) {
        cell->path = NULL;
        cell->gradient = NULL;
        cell->cachedSkin = skin;
        cell->font = NULL;
    }

    if (cell->image == NULL && cell->img_src != NULL)
        cell->image = gum_load_image(cell->img_src);

    gfx_clip_t clip;
    clip.left = cell->box.x + ctx->clip.left;
    clip.top = cell->box.y + ctx->clip.top;
    clip.right = clip.left + cell->box.w;
    clip.bottom = clip.top + cell->box.h;

    if (cell->image)
        gfx_blit(ctx->gfx, (gfx_t*)cell->image, GFX_NOBLEND, &clip, NULL);
    else if (skin->bgcolor >= MIN_ALPHA)
        gfx_fill(ctx->gfx, skin->bgcolor, GFX_NOBLEND, &clip);
}
#endif

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

#if __USE_CAIRO


static void gum_clear_cache(gum_gfx_data_t* ctx, gum_cell_t* cell)
{
    if (cell->image)
        cairo_surface_destroy((cairo_surface_t*)cell->image);
    if (cell->gradient)
        cairo_pattern_destroy((cairo_pattern_t*)cell->gradient);
    if (cell->path)
        cairo_path_destroy((cairo_path_t*)cell->path);
    if (cell->font)
#if defined(__USE_FT)
        gfx_clear_font((gfx_font_t*)cell->font);
#elif defined(__USE_FT_CAIRO)

#else
        cairo_font_face_destroy((cairo_font_face_t*)cell->font);
#endif
    cell->image = NULL;
    cell->gradient = NULL;
    cell->path = NULL;
    cell->font = NULL;
}

static void gum_draw_path(gum_gfx_data_t* ctx, gum_cell_t* cell, GUM_skin* skin, GUM_gctx* gctx)
{
    if (cell->path) {
        cairo_new_path(ctx->cr);
        cairo_append_path(ctx->cr, (cairo_path_t*)cell->path);
        return;
    }

    float sz = MIN(cell->box.w, cell->box.h);
    float r_top_left = CSS_GET_SIZE(skin->r_top_left, gctx->dpi_x, gctx->dsp_x, sz);
    float r_top_right = CSS_GET_SIZE(skin->r_top_right, gctx->dpi_x, gctx->dsp_x, sz);
    float r_bottom_right = CSS_GET_SIZE(skin->r_bottom_right, gctx->dpi_x, gctx->dsp_x, sz);
    float r_bottom_left = CSS_GET_SIZE(skin->r_bottom_left, gctx->dpi_x, gctx->dsp_x, sz);

    cairo_new_path(ctx->cr);
    cairo_move_to(ctx->cr, cell->box.x + r_top_left, cell->box.y);
    cairo_line_to(ctx->cr, cell->box.x + cell->box.w - r_top_right, cell->box.y);
    if (r_top_right != 0)
        cairo_arc(ctx->cr, cell->box.x + cell->box.w - r_top_right, cell->box.y + r_top_right, r_top_right, -M_PI / 2.0, 0.0);
    cairo_line_to(ctx->cr, cell->box.x + cell->box.w, cell->box.y + cell->box.h - r_bottom_right);
    if (r_bottom_right != 0)
        cairo_arc(ctx->cr, cell->box.x + cell->box.w - r_bottom_right, cell->box.y + cell->box.h - r_bottom_right, r_bottom_right, 0.0, M_PI / 2.0);
    cairo_line_to(ctx->cr, cell->box.x + r_bottom_left, cell->box.y + cell->box.h);
    if (r_bottom_left != 0)
        cairo_arc(ctx->cr, cell->box.x + r_bottom_left, cell->box.y + cell->box.h - r_bottom_left, r_bottom_left, M_PI / 2.0, M_PI);
    cairo_line_to(ctx->cr, cell->box.x, cell->box.y + r_top_left);
    if (r_top_left != 0)
        cairo_arc(ctx->cr, cell->box.x + r_top_left, cell->box.y + r_top_left, r_top_left, M_PI, 3 * M_PI / 2.0);

    cell->path = cairo_copy_path(ctx->cr);
}

static cairo_pattern_t* gum_build_gradient(gum_gfx_data_t* ctx, gum_cell_t* cell, GUM_skin* skin)
{
    if (cell->gradient)
        return cell->gradient;

    cairo_pattern_t* grad;

    if (skin->grad_angle == 90)
        grad = cairo_pattern_create_linear(cell->box.x + cell->box.w, 0, cell->box.x, 0);
    else if (skin->grad_angle == 270)
        grad = cairo_pattern_create_linear(cell->box.x, 0, cell->box.x + cell->box.w, 0);
    else
        grad = cairo_pattern_create_linear(0, cell->box.y, 0, cell->box.y + cell->box.h);

    cairo_pattern_add_color_stop_rgba(grad, 0.0, //0, 0.5, 0.5, 0);
        ((skin->bgcolor >> 16) & 255) / 255.0,
        ((skin->bgcolor >> 8) & 255) / 255.0,
        ((skin->bgcolor >> 0) & 255) / 255.0,
        ((skin->bgcolor >> 24) & 255) / 255.0);
    // cairo_pattern_add_color_stop_rgba(grad, 0.25, 0.5, 0.5, 0);
    // ((skin->grcolor >> 16) & 255) / 255.0,
    // ((skin->grcolor >> 8) & 255) / 255.0,
    // ((skin->grcolor >> 0) & 255) / 255.0,
    // ((skin->grcolor >> 24) & 255) / 255.0);
    cairo_pattern_add_color_stop_rgba(grad, 1.0, //0, 0.5, 0.5, 0);
        ((skin->grcolor >> 16) & 255) / 255.0,
        ((skin->grcolor >> 8) & 255) / 255.0,
        ((skin->grcolor >> 0) & 255) / 255.0,
        ((skin->grcolor >> 24) & 255) / 255.0);

    cell->gradient = grad;
    return grad;
}

#ifdef __USE_FT

void *gum_load_fontface(const char *family)
{
    char buf[256];
    char* exts[] = {
        "ttf", "otf",
    };
    for (int i = 0; i < 2; ++i) {
        snprintf(buf, 256, "%s/%s.%s", RESX "/fonts", family, exts[i]);
        FT_Face face;
        FT_Error err = FT_New_Face(ftLibrary, buf, 0, &face);
        // FT_Set_Pixel_Sizes(face, 0, fontsize);
        // FT_Set_Char_Size();
        if (!err)
            return face;
    }
    return NULL;
}

static void *gum_load_font(gum_gfx_data_t* ctx, gum_cell_t* cell, const char* family, GUM_skin* skin)
{
    if (cell->font)
        return cell->font;
    int style = 0;
    if (stricmp("Font Awesome 5 Free", family))
        style = GFXFT_SOLID;
    cell->font = gfx_font(family ? family :  "arial", skin->font_size, style);
    return cell->font;
}

void gum_text_size(gum_cell_t* cell, float* w, float* h, GUM_skin* skin)
{
    /* char_height in 1/64th of points  */
    cell->font = gum_load_font(NULL, cell, skin->font_family, skin);
    // FT_Error err = FT_Set_Char_Size(cell->font->face, 0, skin->font_size * 64, 96, 96);
    gfx_text_t mesures;
    gfx_measure_text(cell->font, cell->text, &mesures);
    *w = mesures.width;
    *h = mesures.height;
}

void gum_text_paint(gum_window_t* win, gum_gfx_data_t * ctx, gum_cell_t * cell, gum_skin_t * skin)
{
    // cairo_pop_group_to_source(ctx->cr);
    // cairo_paint(ctx->cr);
    cairo_surface_flush(ctx->srf);

    void *data = cairo_image_surface_get_data(ctx->srf);
    void *font = gum_load_font(ctx, cell, skin->font_family, skin);
    /* char_height in 1/64th of points  */
    // FT_Error err = FT_Set_Char_Size(face, 0, skin->font_size * 64, 96, 96);
    // if (err) {
    //     fprintf(stderr, "Error with set face size: %d\n", err);
    // }


    gfx_clip_t clip;
    clip.left = ctx->px + cell->box.cx;
    clip.top = ctx->py + cell->box.cy;
    clip.right = clip.left + cell->box.cw;
    clip.bottom = clip.top + cell->box.ch;
    gfx_write(ctx->gfx, font, cell->text, &clip, skin->txcolor, skin->align, skin->valign);
    // cairo_surface_mark_dirty(ctx->srf);
    // cairo_push_group(ctx->cr);
    // cairo_paint(ctx->cr);
    // cairo_surface_mark_dirty_rectangle(ctx->srf, clip.left, clip.top, cell->box.cw, cell->box.ch);
}



#else

#if defined(__USE_FT_CAIRO)

#include <cairo-ft.h>

hmap_t font_map;
bool init_font_map = false;

static cairo_font_face_t *gum_load_font(gum_gfx_data_t *ctx, gum_cell_t *cell, const char *family, GUM_skin *skin)
{
    if (!init_font_map) {
        hmp_init(&font_map, 16);
        init_font_map = true;
    }

    cairo_font_face_t *ct = hmp_get(&font_map, family, strlen(family));
    if (ct != NULL) {
        return ct;
    }

    FT_Face face;
    FT_Error err;
    int style = 0;
    if (stricmp(family, "Font Awesome 5 free") == 0)
        style = GFXFT_SOLID;
    char filename[256];
    int no = gfx_ft_search_font(family, style, filename, 256);
    if (no < 0) {
        fprintf(stderr, "Error unable to find font %s.\n", family);
        exit(EXIT_FAILURE);
    }
    err = FT_New_Face(gfx_ft_libary(), filename, no, &face);
    if (err != 0) {
        fprintf(stderr, "Error %d opening %s.\n", err, filename);
        exit(EXIT_FAILURE);
    }

    ct = cairo_ft_font_face_create_for_ft_face(face, 0);
    hmp_put(&font_map, family, strlen(family), ct);
    return ct;
}

#else

static cairo_font_face_t* gum_load_font(gum_gfx_data_t* ctx, gum_cell_t* cell, const char* family, GUM_skin *skin)
{
    if (cell->font)
        return cell->font;
    cairo_font_face_t* face = cairo_toy_font_face_create(family ? family : "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cell->font = face;
    return face;
}

#endif

void gum_text_size(gum_cell_t* cell, float* w, float* h, GUM_skin* skin, GUM_gctx* ctx)
{
    gum_window_t* win = gum_fetch_window(cell);
    gum_gfx_data_t* gctx = win->data; 
    cairo_text_extents_t extents;
    if (skin == NULL) {
        *w = 0;
        *h = 0;
        return;
    }
        
    cairo_font_face_t* face = gum_load_font(gctx, cell, skin->font_family, skin);
    cairo_set_font_face(gctx->cr, face);
    cairo_set_font_size(gctx->cr, skin->font_size * 96.0 * ctx->dsp_y / 64.0);
    cairo_text_extents(gctx->cr, cell->text, &extents);
    *w = extents.width;
    *h = extents.height;
}

void gum_text_paint(gum_window_t *win, gum_gfx_data_t* ctx, gum_cell_t* cell, gum_skin_t* skin)
{
    int tx = cell->box.cx;
    int ty = cell->box.cy;
    cairo_font_face_t* face = gum_load_font(ctx, cell, skin->font_family, skin);
    cairo_set_font_face(ctx->cr, face);
    cairo_set_font_size(ctx->cr, skin->font_size * 96.0 * win->ctx.dsp_y / 64.0);
    cairo_set_source_rgb(ctx->cr, GFX_RED(skin->txcolor) / 255.0, GFX_GREEN(skin->txcolor) / 255.0, GFX_BLUE(skin->txcolor) / 255.0);

    cairo_text_extents_t extents;
    cairo_text_extents(ctx->cr, cell->text, &extents);

    if (skin->align == 2)
        tx += cell->box.cw - (extents.width + extents.x_bearing);
    else if (skin->align == 0)
        tx += cell->box.cw / 2 - (extents.width / 2 + extents.x_bearing);

    if (skin->valign == 2)
        ty += cell->box.ch - (extents.height + extents.y_bearing);
    else if (skin->valign == 0)
        ty += cell->box.ch / 2 - (extents.height / 2 + extents.y_bearing);

    cairo_move_to(ctx->cr, tx, ty);
    cairo_show_text(ctx->cr, cell->text);
}
#endif


void* gum_load_image(const char* source)
{
    // gfx_t* img = gfx_load_image(source);
    cairo_surface_t* img = cairo_image_surface_create_from_png(source);
    return img;
}

void gum_start_paint(gum_window_t* win)
{
    gum_gfx_data_t* ctx = win->data;
    // cairo_push_group(ctx->cr);
    // cairo_set_source_rgb(ctx->cr, 1, 1, 1);
    // cairo_paint(ctx->cr);
    cairo_reset_clip(ctx->cr);
    ctx->px = ctx->x;
    ctx->py = ctx->y;
}

void gum_end_paint(gum_window_t* win)
{
    gum_gfx_data_t* ctx = win->data;
    // cairo_pop_group_to_source(ctx->cr);
    // cairo_paint(ctx->cr);
    cairo_surface_flush(ctx->srf);
}

void gum_push_clip(gum_window_t* win, GUM_box* box)
{
    gum_gfx_data_t* ctx = win->data;
    cairo_save(ctx->cr);
    cairo_new_path(ctx->cr);
    float w = ceil(box->cw + box->cx + 0.5) - box->cx;
    float h = ceil(box->ch + box->cy + 0.5) - box->cy;
    cairo_rectangle(ctx->cr, box->cx, box->cy, w, h);
    cairo_clip(ctx->cr);
    cairo_translate(ctx->cr, box->cx - box->sx, box->cy - box->sy);
    ctx->px += box->cx - box->sx;
    ctx->py += box->cy - box->sy;
}

void gum_pop_clip(gum_window_t* win, GUM_box* box, GUM_box* prev)
{
    gum_gfx_data_t* ctx = win->data;
    cairo_translate(ctx->cr, box->sx - box->cx, box->sy - box->cy);
    cairo_restore(ctx->cr);
    ctx->px -= box->cx - box->sx;
    ctx->py -= box->cy - box->sy;
}


void gum_draw_cell(gum_window_t* win, gum_cell_t* cell)
{
    gum_gfx_data_t* ctx = win->data;
    GUM_skin* skin = gum_skin(cell);
    if (skin == NULL)
        return;

    if (cell->cachedSkin != skin) {
        gum_clear_cache(ctx, cell);
        cell->cachedSkin = skin;
    }
    if (cell->image == NULL && cell->img_src != NULL)
        cell->image = gum_image(cell->img_src);

    // Draw path
    gum_draw_path(ctx, cell, skin, &win->ctx);

    if (cell->image) {
        cairo_surface_t* img = (cairo_surface_t*)cell->image;
        if (cairo_surface_status(img) == 0) {
            int img_sz = MAX(cairo_image_surface_get_width(img), cairo_image_surface_get_height(img));
            double rt = (double)MAX(cell->box.w, cell->box.h) / (double)img_sz;
            cairo_save(ctx->cr);
            cairo_translate(ctx->cr, cell->box.x, cell->box.y);
            cairo_scale(ctx->cr, rt, rt);
            cairo_set_source_surface(ctx->cr, img, 0, 0);
            cairo_fill_preserve(ctx->cr);
            cairo_restore(ctx->cr);
        }
    } else if (skin->grcolor >= MIN_ALPHA) {
        cairo_pattern_t* grad = gum_build_gradient(ctx, cell, skin);
        cairo_set_source(ctx->cr, grad);
        cairo_fill_preserve(ctx->cr);
    } else if (skin->bgcolor >= MAX_ALPHA) {
        cairo_set_source_rgb(ctx->cr,
            ((skin->bgcolor >> 16) & 255) / 255.0,
            ((skin->bgcolor >> 8) & 255) / 255.0,
            ((skin->bgcolor >> 0) & 255) / 255.0);
        cairo_fill_preserve(ctx->cr);
    } else if (skin->bgcolor >= MIN_ALPHA) {
        cairo_set_source_rgba(ctx->cr,
            ((skin->bgcolor >> 16) & 255) / 255.0,
            ((skin->bgcolor >> 8) & 255) / 255.0,
            ((skin->bgcolor >> 0) & 255) / 255.0,
            ((skin->bgcolor >> 24) & 255) / 255.0);
        cairo_fill_preserve(ctx->cr);
    }

    if (skin->brcolor >= MAX_ALPHA) {
        double size = skin->brsize.unit == 0 ? 1.0 : CSS_GET_SIZE(skin->brsize, win->ctx.dpi_x, win->ctx.dsp_x, cell->box.w);
        cairo_set_line_width(ctx->cr, size);
        cairo_set_source_rgb(ctx->cr,
            ((skin->brcolor >> 16) & 255) / 255.0,
            ((skin->brcolor >> 8) & 255) / 255.0,
            ((skin->brcolor >> 0) & 255) / 255.0);
        cairo_stroke(ctx->cr);

    } else if (skin->brcolor >= MIN_ALPHA) {
        double size = skin->brsize.unit == 0 ? 1.0 : CSS_GET_SIZE(skin->brsize, win->ctx.dpi_x, win->ctx.dsp_x, cell->box.w);
        cairo_set_line_width(ctx->cr, size);
        cairo_set_source_rgba(ctx->cr,
            ((skin->brcolor >> 16) & 255) / 255.0,
            ((skin->brcolor >> 8) & 255) / 255.0,
            ((skin->brcolor >> 0) & 255) / 255.0,
            ((skin->brcolor >> 24) & 255) / 255.0);
        cairo_stroke(ctx->cr);
    }


    if (cell->text)
        gum_text_paint(win, ctx, cell, skin);
}

#endif

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */


#if 0


uint32_t gfx_alpha_blend(uint32_t low, uint32_t upr);
uint32_t gfx_upper_alpha_blend(uint32_t low, uint32_t upr);

void gum_draw_cell(GUM_window* win, GUM_cell* cell, bool top)
{
    //cairo_t *ctx = win->ctx;
    GUM_skin* skin = gum_skin(cell);
    if (skin == NULL)
        return;

    if (cell->cachedSkin != skin) {
        cell->path = NULL;
        cell->gradient = NULL;
        cell->cachedSkin = skin;
        cell->font = NULL;
    }

    if (cell->image == NULL && cell->img_src != NULL)
        cell->image = gum_load_image(cell->img_src);
    gfx_clip_t clip;
    clip.left = cell->box.x + win->clip.left;
    clip.top = cell->box.y + win->clip.top;
    clip.right = clip.left + cell->box.w;
    clip.bottom = clip.top + cell->box.h;

    //    gum_draw_path(ctx, cell, skin);

    if (cell->image) {
        gfx_blit(win->gfx, (gfx_t*)cell->image, GFX_NOBLEND, &clip, NULL);
        //        cairo_surface_t *img = (cairo_surface_t *)cell->image;
        //        int img_sz = MAX(cairo_image_surface_get_width(img), cairo_image_surface_get_height(img));
        //        double rt = (double)MAX(cell->box.w, cell->box.h) / (double)img_sz;
        //        cairo_save(ctx);
        //        cairo_translate(ctx, cell->box.x, cell->box.y);
        //        cairo_scale(ctx, rt, rt);
        //        cairo_set_source_surface(ctx, img, 0, 0);
        //        cairo_fill_preserve(ctx);
        //        cairo_restore(ctx);

    //    } else if (skin->grcolor >= MIN_ALPHA) {

    //        cairo_pattern_t *grad = gum_build_gradient(cell, skin);
    //        cairo_set_source(ctx, grad);
    //        cairo_fill_preserve(ctx);

    }
    else if (skin->bgcolor >= MIN_ALPHA) {
        gfx_fill(win->gfx, skin->bgcolor, GFX_NOBLEND, &clip);
    }

    //    } else if (skin->bgcolor >= MIN_ALPHA) {
    //        cairo_set_source_rgb(ctx, //0.8, 0.8, 0.8);
    //                             ((skin->bgcolor >> 16) & 255) / 255.0,
    //                             ((skin->bgcolor >> 8) & 255) / 255.0,
    //                             ((skin->bgcolor >> 0) & 255) / 255.0);
    //        cairo_fill_preserve(ctx);
    //    }

    //    if (skin->brcolor >= MIN_ALPHA) {
    //        cairo_set_line_width(ctx, 1.0);
    //        cairo_set_source_rgb(ctx, //0.1, 0.1, 0.1);
    //                             ((skin->brcolor >> 16) & 255) / 255.0,
    //                             ((skin->brcolor >> 8) & 255) / 255.0,
    //                             ((skin->brcolor >> 0) & 255) / 255.0);
    //        cairo_stroke(ctx);
    //    }
    //}

    //// fprintf(stderr, " -- %s\n", cell->text);

    if (cell->text) {
#ifdef __USE_FT
        FT_Error error;
        FT_Face face = (FT_Face)cell->font;
        if (face == NULL) {
            char buf[128];
            snprintf(buf, 128, RESX "/fonts/%s.ttf", skin->font_family);
            error = FT_New_Face(ftLibrary, buf, 0, &face);
            if (error)
                return;
            cell->font = face;
        }

        GUM_gctx ctx;
        gum_fill_context(win, &ctx);
        /* char_height in 1/64th of points  */
        error = FT_Set_Char_Size(face, 0, skin->font_size * 64, ctx.dpi_x, ctx.dpi_y);

        gfx_ft_write_text(win->gfx, cell->font, cell->text, &clip, skin->txcolor, skin->align, skin->valign);
#endif


        //    cairo_set_source_rgb(ctx, //0.8, 0.8, 0.8);
        //                         ((skin->txcolor >> 16) & 255) / 255.0,
        //                         ((skin->txcolor >> 8) & 255) / 255.0,
        //                         ((skin->txcolor >> 0) & 255) / 255.0);

        //    cairo_text_extents_t extents;

        //    const char *ffamily = skin->font_family ? skin->font_family : "Sans";
        //    cairo_select_font_face(ctx, ffamily, CAIRO_FONT_SLANT_NORMAL, 0);
        //    cairo_set_font_size(ctx, (float)skin->font_size);
        //    cairo_text_extents(ctx, cell->text, &extents);
        //    int tx = cell->box.x;
        //    int ty = cell->box.y;

        //    if (skin->align == 2)
        //        tx += cell->box.w - (extents.width + extents.x_bearing);
        //    else if (skin->align == 0)
        //        tx += cell->box.w / 2 - (extents.width / 2 + extents.x_bearing);

        //    if (skin->valign == 2)
        //        ty += cell->box.h - (extents.height + extents.y_bearing);
        //    else if (skin->valign == 0)
        //        ty += cell->box.h / 2 - (extents.height / 2 + extents.y_bearing);

        //    cairo_move_to(ctx, tx, ty);
        //    cairo_show_text(ctx, cell->text);

    }
}



void gum_text_size(gum_cell_t* cell, float* w, float* h, GUM_skin* skin)
{
#ifdef __USE_FT

    FT_Error error;
    FT_Face face = (FT_Face)cell->font;
    if (face == NULL) {
        char buf[128];
        snprintf(buf, 128, RESX "/fonts/%s.ttf", skin->font_family);
        error = FT_New_Face(ftLibrary, buf, 0, &face);
        if (error)
            return;
        cell->font = face;
    }

    GUM_gctx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.dpi_x = 96;
    ctx.dpi_x = 96;
    // gum_fill_context(win, &ctx);
    /* char_height in 1/64th of points  */
    error = FT_Set_Char_Size(face, 0, skin->font_size * 64, ctx.dpi_x, ctx.dpi_y);
    if (error)
        return;

    gfx_text_t measures;
    gfx_measure_text(face, cell->text, &measures);
    *w = measures.width;
    *h = measures.height;

#endif
    //if (srf == NULL) {
    //    srf = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 10, 10);
    //    ctx = cairo_create(srf);
    //}
    //const char *ffamily = skin->font_family ? skin->font_family : "Sans";
    //fprintf(stderr, "Select Font %s %d\n", ffamily, skin->font_size);
    //cairo_select_font_face(ctx, ffamily, CAIRO_FONT_SLANT_NORMAL, 0);
    //cairo_set_font_size(ctx, (float)skin->font_size);

    //cairo_text_extents_t extents;
    //cairo_text_extents(ctx, text, &extents);
    //*w = extents.width + extents.x_bearing;
    //*h = extents.height + extents.y_bearing;
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void gum_handle_event(GUM_event_manager* evm, GUM_event* event)
{
    int i;
    GUM_async* async;
    fprintf(stderr, "Event %d enter\n", event->type);
    switch (event->type) {
    case GUM_EV_RESIZE:
        // fprintf(stderr, "W %d - H %d\n", event->param0, event->param1);
        gum_resize_win(evm->win, event->param0, event->param1);
        evm->ctx.width = event->param0;
        evm->ctx.height = event->param1;
        gum_resize_px(evm->root, evm->ctx.width, evm->ctx.height);
        break;

    case GUM_EV_MOTION:
        gum_event_motion(evm, event->param0, event->param1);
        break;

    case GUM_EV_BTN_PRESS:
        if (event->param0 == 1)
            gum_event_left_press(evm);
        else
            gum_event_button_press(evm, event->param0);
        break;

    case GUM_EV_WHEEL_UP:
        gum_event_wheel(evm, -20);
        break;

    case GUM_EV_WHEEL_DOWN:
        gum_event_wheel(evm, 20);
        break;

    case GUM_EV_BTN_RELEASE:
        if (event->param0 == 1)
            gum_event_left_release(evm);
        else
            gum_event_button_release(evm, event->param0);
        break;

    case GUM_EV_KEY_ENTER:
        gum_event_key_press(evm, event->param0, event->param1);
        break;
    case GUM_EV_KEY_PRESS:
        // gum_event_key_press(evm, event->param0, event->param1);
        break;
    case GUM_EV_KEY_RELEASE:
        // gum_event_key_release(evm, event->param0, event->param1);
        break;
    case GUM_EV_TICK:
        gum_emit_event(evm, NULL, GUM_EV_TICK);
        // TODO properties
        gum_update_mesure(evm);
        gum_update_layout(evm);
        if (gum_update_visual(evm))
            gfx_flip(ctx->gfx);
        break;
    case GUM_EV_ASYNC:
        async = (GUM_async*)(size_t)event->param0;
        async->callback(evm, async->res);
        break;
    }
    // fprintf(stderr, "Event %d leave\n", event->type);
}

#endif
