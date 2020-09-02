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
#include <gum/core.h>
#include <gum/cells.h>
#include <gum/events.h>
#include <kora/hmap.h>
#include <windows.h>
#include <stdio.h>
#include <windowsx.h>
#include <tchar.h>
#include <kora/keys.h>

struct GUM_window {
    HWND hwnd;
    HDC hdc;
    int x, y;
    PAINTSTRUCT ps;
    HBITMAP hbmp;
    HBITMAP old;
    RECT inval;
};

long long gum_system_time()
{
    // January 1, 1970 (start of Unix epoch) in ticks
    const INT64 UNIX_START = 0x019DB1DED53E8000;

    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    LARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    // Convert ticks since EPOCH into micro-seconds
    return (li.QuadPart - UNIX_START) / 10;
}


static TCHAR szWindowClass[] = _T("gum");
static TCHAR szTitle[] = _T("Application");
static WNDCLASSEX wcex;
HINSTANCE appInstance;


LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool win32_init = false;

void gum_win32_setup()
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    appInstance = hInstance;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    if (!RegisterClassEx(&wcex)) {
        MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Win32 Error"), 0);
        exit(-1);
    }
    win32_init = true;
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

bool fonts_init = false;
HMP_map fonts_map;

void *gum_load_font(GUM_window *win, GUM_skin *skin)
{
    wchar_t name[52];
    mbstowcs(name, skin->font_family ? skin->font_family : "Arial", 50);
    int sz = MulDiv(skin->font_size, GetDeviceCaps(win->hdc, LOGPIXELSY), 72);
    return (void *)CreateFont(sz, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, name);
}

void *gum_fetch_font(GUM_window *win, GUM_skin *skin)
{
    if (!fonts_init) {
        hmp_init(&fonts_map, 16);
        fonts_init = true;
    }
    char buf[120];
    int lg = snprintf(buf, 120, "%s.%d.%d00.%c", skin->font_family ? skin->font_family : "Arial", skin->font_size, 3, 'N');
    void *font = hmp_get(&fonts_map, buf, lg);
    if (font == NULL) {
        font = gum_load_font(win, skin);
        hmp_put(&fonts_map, buf, lg, font);
    }
    return font;
}

GUM_window *gum_create_surface(int width, int height)
{
    if (!win32_init)
        gum_win32_setup();
    HWND hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, width + (16), height + (39), NULL, NULL,
                             appInstance, NULL);
    if (!hwnd) {
        DWORD err = GetLastError();
        MessageBox(NULL, _T("Can't create the window!"), _T("Win32 Error"), 0);
        return NULL;
    }

    UINT timer;
    SetTimer(hwnd, (UINT_PTR)&timer, 20, NULL);


    GUM_window *win = (GUM_window *)malloc(sizeof(GUM_window));
    win->hwnd = hwnd;
    win->hdc = GetDC(win->hwnd);
    win->hbmp = 0;
    //RECT rcClient;
    //GetClientRect(hwnd, &rcClient);

    win->inval.left = 0;
    win->inval.right = 0;
    win->inval.top = 0;
    win->inval.bottom = 0;
    return win;
}

void gum_destroy_surface(GUM_window *win)
{
    ReleaseDC(win->hwnd, win->hdc);
}

GUM_window *gum_surface(GUM_window *parent, int width, int height)
{
    HBITMAP hbmp = CreateCompatibleBitmap(parent->hdc, width, height);
    GUM_window *win = (GUM_window *)malloc(sizeof(GUM_window));
    win->hwnd = 0;
    win->hbmp = hbmp;
    win->hdc = CreateCompatibleDC(parent->hdc);
    return win;
}

void gum_fill_context(GUM_window *win, GUM_gctx *ctx)
{
    RECT rect;
    GetWindowRect(win->hwnd, &rect);
    ctx->width = rect.right - rect.left;
    ctx->height = rect.bottom - rect.top;
    ctx->dpi_x = ctx->dpi_y = 96;
    ctx->dsp_x = ctx->dsp_y = 0.75;
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */


int kdb_state = 0, kdb_sec = 0;
int gum_event_poll(GUM_window *win, GUM_event *event, int timeout)
{
    MSG msg;
    if (!GetMessage(&msg, win->hwnd, 0, 0)) {
        event->type = GUM_EV_DESTROY;
        return -1;
    }

    if (msg.message > WM_USER && msg.message < WM_USER + 4096) {
        event->type = msg.message - WM_USER;
        event->param0 = msg.lParam;
        event->param1 = msg.wParam;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        return 0;
    }

    event->param0 = 0;
    event->param1 = 0;
    switch (msg.message) {
    case 0:
    case WM_QUIT:
    case WM_DESTROY:
    case WM_CLOSE:
        event->type = GUM_EV_DESTROY;
        break;
    case WM_PAINT:
        event->type = GUM_EV_EXPOSE;
        return 0;
    case WM_ERASEBKGND:
        return 0;
    case WM_NCMOUSEMOVE:
        event->type = GUM_EV_MOTION;
        event->param0 = GET_X_LPARAM(msg.lParam) - 8;
        event->param1 = GET_Y_LPARAM(msg.lParam) - 16;
        break;
    case WM_MOUSEMOVE:
        event->type = GUM_EV_MOTION;
        event->param0 = GET_X_LPARAM(msg.lParam);
        event->param1 = GET_Y_LPARAM(msg.lParam);
        break;
    case WM_LBUTTONDOWN:
        event->type = GUM_EV_BTN_PRESS;
        event->param0 = 1;
        break;
    case WM_LBUTTONUP:
        event->type = GUM_EV_BTN_RELEASE;
        event->param0 = 1;
        break;
    case WM_MBUTTONDOWN:
        event->type = GUM_EV_BTN_PRESS;
        event->param0 = 2;
        break;
    case WM_MBUTTONUP:
        event->type = GUM_EV_BTN_RELEASE;
        event->param0 = 2;
        break;
    case WM_RBUTTONDOWN:
        event->type = GUM_EV_BTN_PRESS;
        event->param0 = 3;
        break;
    case WM_RBUTTONUP:
        event->type = GUM_EV_BTN_RELEASE;
        event->param0 = 3;
        break;
    case WM_MOUSEWHEEL: {
        short delta = GET_WHEEL_DELTA_WPARAM(msg.wParam);
        event->param0 = abs(delta);
        event->type = delta < 0 ? GUM_EV_WHEEL_DOWN : GUM_EV_WHEEL_UP;
        break;
    }
    case WM_KEYDOWN:
        event->type = GUM_EV_KEY_PRESS;
        event->param0 = keyboard_down((msg.lParam >> 16) & 0x7f, &kdb_state, &kdb_sec);
        event->param1 = kdb_state;
        break;
    case WM_KEYUP:
        event->type = GUM_EV_KEY_RELEASE;
        event->param0 = keyboard_up((msg.lParam >> 16) & 0x7f, &kdb_state);
        event->param1 = kdb_state;
        break;
    case WM_TIMER:
        event->type = GUM_EV_TICK;
        break;
    case 0x60:
    case WM_NCLBUTTONDOWN:
    case 799:
    case 1847:
    case 1848:
    case 49303:
    case 49305:
    case 260:
    case 674:
    default:
        event->type = -1;
        break;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    return 0;
}


void gum_push_event(GUM_window *win, int type, size_t param0, size_t param1, void *data)
{
    // PostMessage(win->hwnd, WM_USER + type, param1, param0);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void gum_do_visual(GUM_cell *cell, GUM_window *win, GUM_rect *inval)
{
    RECT r;
    r.left = inval->left;
    r.right = inval->right;
    r.top = inval->top;
    r.bottom = inval->bottom;
    if (r.left != r.right && r.top != r.bottom)
        InvalidateRect(win->hwnd, &r, FALSE);
}

void gum_resize_win(GUM_window *win, int width, int height)
{
}

void gum_text_size(const char *text, int *w, int *h, GUM_skin *skin)
{
    *h = 10;
    *w = strlen(text) * 8;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

#define MIN_ALPHA 0x1000000
#define M_PI 3.141592653589793
#define COLOR_REF(n) ( (((n) & 0xFF0000) >> 16) | ((n) & 0xFF00) | (((n) & 0xFF) << 16) )
void gum_start_paint(GUM_window *win)
{
    win->x = 0;
    win->y = 0;
    if (win->hbmp != NULL)
        win->old = SelectObject(win->hdc, win->hbmp);
    else
        win->hdc = BeginPaint(win->hwnd, &win->ps);
}

void gum_end_paint(GUM_window *win)
{
    if (win->hbmp != NULL)
        SelectObject(win->hdc, win->old);
    else
        EndPaint(win->hwnd, &win->ps);
}

void gum_push_clip(GUM_window *win, GUM_box *box)
{
    win->x += box->cx - box->sx;
    win->y += box->cy - box->sy;
    HRGN region = CreateRectRgn(win->x, win->y, win->x + box->cw, win->y + box->ch);
    SelectClipRgn(win->hdc, region);
}

void gum_pop_clip(GUM_window *win, GUM_box *box, GUM_box *prev)
{
    win->x -= box->cx - box->sx;
    win->y -= box->cy - box->sy;
    if (prev != NULL) {
        HRGN region = CreateRectRgn(win->x, win->y, win->x + prev->cw, win->y + prev->ch);
        SelectClipRgn(win->hdc, region);
    } else
        SelectClipRgn(win->hdc, NULL);
}

void gum_draw_pic(GUM_window *win, GUM_window *sub, GUM_box *box, GUM_anim *anim)
{
    // TODO -- animation
    BITMAP bm;
    HBITMAP hbmOld = SelectObject(sub->hdc, sub->hbmp);
    GetObject(sub->hbmp, sizeof(bm), &bm);

    double prg = 1.0;
    anim->duration = 300;
    int dy = 0;
    if (anim->delay < anim->duration) {
        long long now = gum_system_time();
        anim->delay += (int)((now - anim->last) / 1000LL);
        anim->last = now;
        prg = MIN(1.0, anim->delay * 1.0 / anim->duration);
        // invalid frame
        dy = (int)((box->h - anim->ow) / 2.0 * (1.0 - prg));
    }

    StretchBlt(win->hdc, box->x + win->x, box->y + win->y - dy, box->w, box->h - 2 * dy, sub->hdc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    SelectObject(win->hdc, hbmOld);
    // DeleteDC(hdcMem);
}

void gum_draw_img(GUM_window *win, HBITMAP bmp, GUM_box *box)
{
    // TODO -- animation
    BITMAP bm;
    HDC hdcMem = CreateCompatibleDC(win->hdc);
    HBITMAP hbmOld = SelectObject(hdcMem, bmp);
    GetObject(bmp, sizeof(bm), &bm);
    StretchBlt(win->hdc, box->x + win->x, box->y + win->y, box->w, box->h, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    SelectObject(win->hdc, hbmOld);
    DeleteDC(hdcMem);
}


void gum_draw_cell(GUM_window *win, GUM_cell *cell, bool top)
{
    GUM_skin *skin = gum_skin(cell);
    if (skin == NULL)
        return;

    if (cell->cachedSkin != skin) {
        cell->path = NULL;
        cell->gradient = NULL;
        cell->cachedSkin = skin;
    }

    if (! top && cell->state & GUM_CELL_BUFFERED) {
        if (cell->surface == NULL)
            cell->surface = gum_surface(win, cell->box.w, cell->box. h);
        if (cell->surface != NULL) {
            // TODO -- sub-surface
            gum_paint(cell->surface, cell);
            gum_draw_pic(win, cell->surface, &cell->box, &cell->anim);
        }
        return;
    }

    if (cell->image == NULL && cell->img_src != NULL)
        cell->image = gum_image(cell->img_src);

    RECT r;
    r.left = cell->box.x + win->x;
    r.top = cell->box.y + win->y;
    r.right = cell->box.x + cell->box.w + win->x;
    r.bottom = cell->box.y + cell->box.h + win->y;
    SetBkMode(win->hdc, TRANSPARENT);
    if (cell->image != NULL)
        gum_draw_img(win, (HBITMAP)cell->image, &cell->box);

    else if (skin->bgcolor >= MIN_ALPHA && skin->grcolor >= MIN_ALPHA) {
        for (int i = 0; i < cell->box.h; ++i) {
            r.top = cell->box.y + win->y + i;
            r.bottom = r.top + 1;
            unsigned color = gum_mix(skin->grcolor, skin->bgcolor, (float)i / cell->box.h);
            HBRUSH brush = CreateSolidBrush(COLOR_REF(color));
            FillRect(win->hdc, &r, brush);
            DeleteObject(brush);
        }
    } else if (skin->bgcolor >= MIN_ALPHA) {
        HBRUSH brush = CreateSolidBrush(COLOR_REF(skin->bgcolor));
        FillRect(win->hdc, &r, brush);
        DeleteObject(brush);
    }

    if (skin->brcolor >= MIN_ALPHA) {
        HPEN pen = CreatePen(PS_SOLID, 1, COLOR_REF(skin->brcolor));
        SelectObject(win->hdc, GetStockObject(NULL_BRUSH));
        SelectObject(win->hdc, pen);
        Rectangle(win->hdc, cell->box.x + win->x, cell->box.y + win->y, cell->box.x + cell->box.w + win->x, cell->box.y + cell->box.h + win->y);
        DeleteObject(pen);
    }

    if (cell->text) {
        TCHAR szBuf[64];
        mbstowcs(szBuf, cell->text, 64);
        int alg = 0;
        if (skin->align == 2)
            alg |= DT_RIGHT;
        else if (skin->align == 0)
            alg |= DT_CENTER;
        else
            alg |= DT_LEFT;


        if (skin->valign == 2)
            alg |= DT_BOTTOM;
        else if (skin->valign == 0)
            alg |= DT_VCENTER;
        else
            alg |= DT_TOP;

        if (skin->font == NULL)
            skin->font = gum_fetch_font(win, skin);
        if (skin->font != NULL) {
            SelectObject(win->hdc, (HFONT)skin->font);
            SetTextColor(win->hdc, COLOR_REF(skin->txcolor));
            DrawText(win->hdc, szBuf, wcslen(szBuf), &r, alg | DT_SINGLELINE);
        }
    }
}

void gum_draw_scrolls(GUM_window *win, GUM_cell *cell)
{
}

void *gum_load_image(const char *name)
{
    TCHAR szBuf[64];
    mbstowcs(szBuf, name, 64);
    for (int i = 0; i < 64; ++i)
        if (szBuf[i] == '/')
            ((char*)name)[i/2] = szBuf[i] = '\\';
    HBITMAP bmp = (HBITMAP)LoadImage(NULL, szBuf, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (bmp == NULL) {
        bmp = (HBITMAP)LoadImage(NULL, name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    }
    return bmp;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void *memrchr(const void *buf, int byte, size_t len)
{
    const char *ptr = (const char *)buf + len;
    while (ptr-- > (const char *)buf)
        if (*ptr == byte)
            return (void *)ptr;
    return NULL;
}


