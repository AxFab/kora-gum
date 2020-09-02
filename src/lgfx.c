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
#include <gum/events.h>
#include <gum/cells.h>
#include <gum/xml.h>
#include <gum/css.h>
#include <gfx.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <ft2build.h>
#include <freetype/freetype.h>
#include <kora/mcrs.h>


struct GUM_window {
    bool redraw;
    gfx_t *gfx;
    gfx_clip_t clip;
    gfx_seat_t seat;
};

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


FT_Library ftLibrary;
bool ftLibraryInitialized = false;



GUM_window* gum_open_surface(gfx_t *gfx)
{
    if (!ftLibraryInitialized) {

        FT_Error error = FT_Init_FreeType(&ftLibrary);
        if (!error)
            ftLibraryInitialized = true;
    }


    GUM_window* win = (GUM_window*)malloc(sizeof(GUM_window));
    win->gfx = gfx;
    win->redraw = true;
    memset(&win->seat, 0, sizeof(win->seat));

    return win;
}

GUM_window *gum_create_surface(int width, int height)
{
    gfx_t * gfx = gfx_create_window(NULL, width, height, 0);
    return gum_open_surface(gfx);
}

void gum_destroy_surface(GUM_window *win)
{
    //cairo_destroy(win->ctx);
    //Display *dsp = cairo_xlib_surface_get_display(win->srf);
    //cairo_surface_destroy(win->srf);
    //XCloseDisplay(dsp);
    gfx_destroy(win->gfx);
    free(win);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

#define MIN_ALPHA 0x1000000
#define M_PI 3.141592653589793


uint32_t gfx_alpha_blend(uint32_t low, uint32_t upr);
uint32_t gfx_upper_alpha_blend(uint32_t low, uint32_t upr);

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
            uint32_t color = (fontcolor & 0xffffff) | (val  << 24);
            uint32_t* dst = &gfx->pixels4[j * gfx->pitch / 4 + i];
            uint32_t nw = gfx_upper_alpha_blend(*dst, color);
            *dst = nw;
        }
    }
}


void gum_draw_cell(GUM_window *win, GUM_cell *cell, bool top)
{
    //cairo_t *ctx = win->ctx;
    GUM_skin *skin = gum_skin(cell);
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

    if (skin->bgcolor >= MIN_ALPHA)
        gfx_fill(win->gfx, skin->bgcolor, GFX_NOBLEND, &clip);

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
    }
    //    } else if (skin->grcolor >= MIN_ALPHA) {

    //        cairo_pattern_t *grad = gum_build_gradient(cell, skin);
    //        cairo_set_source(ctx, grad);
    //        cairo_fill_preserve(ctx);

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
    if (cell->text && ftLibraryInitialized) {

        FT_Error error;
        FT_Face face = (FT_Face)cell->font;
        if (face == NULL) {
            char buf[64];
            snprintf(buf, 64, "./resx/%s.ttf", skin->font_family);
            error = FT_New_Face(ftLibrary, buf, 0, &face);
            if (error) {
                return;
            }
            cell->font = face;
        }

        GUM_gctx ctx;
        gum_fill_context(win, &ctx);
        /* char_height in 1/64th of points  */
        error = FT_Set_Char_Size(face, 0, skin->font_size * 64, ctx.dpi_x, ctx.dpi_y);


        int pen_x = clip.left; // +cell->box.x;
        int pen_y = clip.top; // cell->box.y;

        int text_width = 0;
        int text_x_bearing = 0;
        int text_height = 0;
        int text_y_bearing = 0;


        int i;
        FT_GlyphSlot  slot = face->glyph;
        int miny = 0, maxy = 0;
        for (i = 0; ; ++i) {
            int ch;
            int len = mbtouc(&ch, &cell->text[i], 6);
            i += len - 1;
            if (ch == 0)
                break;

            FT_UInt  glyph_index;
            glyph_index = FT_Get_Char_Index(face, ch);
            error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
            if (error)
                continue;

            error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
            if (error)
                continue;

            text_width += slot->bitmap_left + slot->advance.x >> 6;
            if (miny > -slot->bitmap_top)
                miny = -slot->bitmap_top;
            if (maxy < slot->bitmap.rows - slot->bitmap_top)
                maxy = slot->bitmap.rows - slot->bitmap_top;
        }
        text_height = maxy - miny;
        text_y_bearing = miny;

        if (skin->align == 2)
            pen_x += cell->box.w - (text_width + text_x_bearing);
        else if (skin->align == 0)
            pen_x += cell->box.w / 2 - (text_width / 2 + text_x_bearing);

        if (skin->valign == 2)
            pen_y += cell->box.h - (text_height + text_y_bearing);
        else if (skin->valign == 0)
            pen_y += cell->box.h / 2 - (text_height / 2 + text_y_bearing);

        for (i = 0; ; ++i) {
            int ch;
            int len = mbtouc(&ch, &cell->text[i], 6);
            i += len - 1;
            if (ch == 0)
                break;

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
            gfx_copy_glyph(win->gfx, &clip, &slot->bitmap, pen_x + slot->bitmap_left, pen_y - slot->bitmap_top, skin->txcolor);


            /* increment pen position */
            pen_x += slot->advance.x >> 6;
            pen_y += slot->advance.y >> 6;
        }

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



void gum_text_size(const char *text, int *w, int *h, GUM_skin *skin)
{
    FT_Face face;
    FT_Error error = FT_New_Face(ftLibrary, "./resx/arial.ttf", 0, &face);
    if (error) {
        return;
    }

    /* char_height in 1/64th of points  */
    error = FT_Set_Char_Size(face, 0, skin->font_size * 64, 96, 96);


    int text_width = 0;
    int text_x_bearing = 0;
    int text_height = 0;
    int text_y_bearing = 0;


    int i;
    FT_GlyphSlot  slot = face->glyph;
    int miny = 0, maxy = 0;
    for (i = 0; ; ++i) {
        int ch;
        int len = mbtouc(&ch, &text[i], 6);
        i += len - 1;
        if (ch == 0)
            break;

        FT_UInt  glyph_index;
        glyph_index = FT_Get_Char_Index(face, ch);
        error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
        if (error)
            continue;

        error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
        if (error)
            continue;

        text_width += slot->bitmap_left + slot->advance.x >> 6;
        if (miny > -slot->bitmap_top)
            miny = -slot->bitmap_top;
        if (maxy < slot->bitmap.rows - slot->bitmap_top)
            maxy = slot->bitmap.rows - slot->bitmap_top;
    }
    text_height = maxy - miny;
    text_y_bearing = miny;

    *w = text_width;
    *h = text_height;

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


void gum_draw_scrolls(GUM_window *win, GUM_cell *cell)
{
    //cairo_t *ctx = win->ctx;
    //if (cell->state & GUM_CELL_OVERFLOW_X) {
    //    cairo_new_path(ctx);
    //    cairo_rectangle(ctx,
    //                    cell->box.x,
    //                    cell->box.y + cell->box.h - 7,
    //                    cell->box.w - 7, 7);
    //    cairo_set_source_rgb(ctx, 0.7, 0.7, 0.7);
    //    cairo_fill(ctx);

    //    int sz = cell->box.cw * (cell->box.w - 7) / cell->box.ch_w;
    //    int st = cell->box.sx * (cell->box.w - 7) / cell->box.ch_w;

    //    cairo_new_path(ctx);
    //    cairo_rectangle(ctx,
    //                    cell->box.x + st,
    //                    cell->box.y + cell->box.h - 7,
    //                    sz, 7);
    //    cairo_set_source_rgb(ctx, 66.0 / 255.0, 165.0 / 255.0, 245.0 / 255.0);
    //    cairo_fill(ctx);
    //}

    //if (cell->state & GUM_CELL_OVERFLOW_Y) {
    //    cairo_new_path(ctx);
    //    cairo_rectangle(ctx,
    //                    cell->box.x + cell->box.w - 7,
    //                    cell->box.y, 7, cell->box.h - 7);
    //    cairo_set_source_rgb(ctx, 0.7, 0.7, 0.7);
    //    cairo_fill(ctx);

    //    int sz = cell->box.ch * (cell->box.h - 7) / cell->box.ch_h;
    //    int st = cell->box.sy * (cell->box.h - 7) / cell->box.ch_h;

    //    cairo_new_path(ctx);
    //    cairo_rectangle(ctx,
    //                    cell->box.x + cell->box.w - 7,
    //                    cell->box.y + st, 7, sz);
    //    cairo_set_source_rgb(ctx, 66.0 / 255.0, 165.0 / 255.0, 245.0 / 255.0);
    //    cairo_fill(ctx);
    //}
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

#include <kora/keys.h>


void gum_fill_context(GUM_window *win, GUM_gctx *ctx)
{
    ctx->dpi_x = 96;
    ctx->dpi_y = 96;
    ctx->dsp_x = 0.75;
    ctx->dsp_y = 0.75;
    ctx->width = win->gfx->width;
    ctx->height = win->gfx->height;
}

#define GET_X_LPARAM(s)   ((int)(short)((int32_t)(s) & 0xffff))
#define GET_Y_LPARAM(s)   ((int)(short)(((int32_t)(s) >> 16) & 0xffff))

int gum_event_poll(GUM_window *win, GUM_event *event, int timeout)
{
    gfx_msg_t msg;
    do {
        gfx_poll(win->gfx, &msg);
        gfx_handle(win->gfx, &msg, &win->seat);
        event->type = msg.message;
        event->param0 = msg.param1;
        if (msg.message < 128) {
            event->type = -1;
            switch (msg.message) {
            case GFX_EV_QUIT:
                event->type = GUM_EV_DESTROY;
                break;
            case GFX_EV_MOUSEMOVE:
                event->type = GUM_EV_MOTION;
                event->param0 = GET_X_LPARAM(msg.param1);
                event->param1 = GET_Y_LPARAM(msg.param1);
                // printf("Mouse at %dx%d\n", event->param0, event->param1);
                break;
            case GFX_EV_BTNDOWN:
                event->type = GUM_EV_BTN_PRESS;
                event->param0 = msg.param1;
                break;
            case GFX_EV_BTNUP:
                event->type = GUM_EV_BTN_RELEASE;
                event->param0 = msg.param1;
                break;
            case GFX_EV_MOUSEWHEEL:
                break;
            case GFX_EV_KEYDOWN:
                event->type = GUM_EV_KEY_PRESS;
                break;
            case GFX_EV_KEYUP:
                event->type = GUM_EV_KEY_RELEASE;
                break;
            case GFX_EV_KEYPRESS:
                event->type = GUM_EV_KEY_ENTER;
                event->param0 = msg.param1;
                break;
             case GFX_EV_TIMER:
                event->type = GUM_EV_TICK;
                break;
            case GFX_EV_RESIZE:
                break;
            case GFX_EV_PAINT:
                event->type = GUM_EV_EXPOSE;
                break;
            default:
                printf("Unsupported\n");
                break;
            }
        }

    } while (event->type == -1);

    return 0;
}


void gum_start_paint(GUM_window *win)
{
    if (win->gfx->pixels == NULL)
        gfx_map(win->gfx);
    //cairo_push_group(win->ctx);
    //cairo_set_source_rgb(win->ctx, 1, 1, 1);
    //cairo_paint(win->ctx);
    //cairo_reset_clip(win->ctx);
    // cairo_translate(win->ctx, x, y);
    win->clip.left = 0;
    win->clip.top = 0;
}

void gum_end_paint(GUM_window *win)
{
    //cairo_pop_group_to_source(win->ctx);
    //cairo_paint(win->ctx);
    //cairo_surface_flush(win->srf);
    win->redraw = false;
    gfx_flip(win->gfx);
}

void gum_push_clip(GUM_window *win, GUM_box *box)
{
    //cairo_save(win->ctx);
    //cairo_new_path(win->ctx);
    //cairo_rectangle(win->ctx, box->cx, box->cy, box->cw, box->ch);
    //cairo_clip(win->ctx);
    //cairo_translate(win->ctx, box->cx - box->sx, box->cy - box->sy);
    win->clip.left += box->cx - box->sx;
    win->clip.top += box->cy - box->sy;
}

void gum_pop_clip(GUM_window *win, GUM_box *box, GUM_box *prev)
{
    win->clip.left -= box->cx - box->sx;
    win->clip.top -= box->cy - box->sy;
    //cairo_translate(win->ctx, box->sx - box->cx, box->sy - box->cy);
    //cairo_restore(win->ctx);
}

void gum_resize_win(GUM_window *win, int width, int height)
{
    gfx_unmap(win->gfx);
    // gfx_resize(win->gfx, width, height);
    //cairo_xlib_surface_set_size(win->srf, width, height);
}


void *gum_load_image(const char *name)
{
    gfx_t* img = gfx_load_image(name);
    //cairo_surface_t *img = cairo_image_surface_create_from_png(name);
    //if (cairo_surface_status(img) != 0)
    //    return NULL;
    return img;
}

void gum_do_visual(GUM_cell *cell, GUM_window *win, GUM_sideruler *inval)
{
    win->redraw = true;
    gfx_invalid(win->gfx);

}

void gum_push_event(GUM_window *win, int type, size_t param0, size_t param1, void *data)
{
    gfx_push(win->gfx, type, param0);
}

