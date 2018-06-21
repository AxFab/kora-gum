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
#include <kora/gum/core.h>
#include <kora/gum/cells.h>
#include <kora/gum/events.h>
#include <windows.h>  
#include <tchar.h>

struct GUM_window {
    HWND hwnd;
    HDC hdc;
    int x, y;
};

static TCHAR szWindowClass[] = _T("gum");
static TCHAR szTitle[] = _T("Application");
static WNDCLASSEX wcex;
HINSTANCE appInstance;

LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
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
        MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Win32 Guided Tour"), NULL);
        return 1;
    }

    return S_main();
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

GUM_window *gum_create_surface(int width, int height)
{
    HWND hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, width + (16), height + (39), NULL, NULL, appInstance, NULL);
    if (!hwnd) {
        DWORD err = GetLastError();
        MessageBox(NULL, _T("Can't create the window!"), _T("Win32 Guided Tour"), NULL);
        return NULL;
    }

    GUM_window *win = (GUM_window*)malloc(sizeof(GUM_window));
    win->hwnd = hwnd;
    win->hdc = GetDC(win->hwnd);
    //RECT rcClient;
    //GetClientRect(hwnd, &rcClient);
    return win;
}

void gum_destroy_surface(GUM_window *win)
{
    ReleaseDC(win->hwnd, win->hdc);
}

int last;
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

int gum_event_poll(GUM_window *win, GUM_event *event, int timeout)
{
    MSG msg;
    if (!GetMessage(&msg, win->hwnd, 0, 0)) {
        event->type = GUM_EV_DESTROY;
        return -1;
    }

    last = msg.message;
    switch (msg.message) {
    case 0:
    case WM_QUIT:
    case WM_DESTROY:
    case WM_CLOSE:
        event->type = GUM_EV_DESTROY;
        break;
    case WM_PAINT:
        event->type = GUM_EV_EXPOSE;
        break;
    case WM_TIMER:
    case 0x60:
    case WM_NCMOUSEMOVE:
    case WM_NCLBUTTONDOWN:
    case 799:
    case 1847:
    case 1848:
    case 512:
    case 257:
    case 49303:
    case 260:
    case 674:
        event->type = -1;
        break;
    default:
        event->type = -1;
        break;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    /*
    for (int i = 0; i < 1000; i++)
    {
        int x = rand() % 300;
        int y = rand() % 300;
        SetPixel(win->hdc, x, y, RGB(rand() % 255, 0, 0));
    }
    */
    return 0;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

void gum_invalid_surface(GUM_window *win, int x, int y, int w, int h)
{
}

void gum_resize_win(GUM_window *win, int width, int height)
{
}

void gum_text_size(const char *text, int *w, int *h)
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
    win->x = win->y = 0;
}

void gum_end_paint(GUM_window *win)
{
}

void gum_push_clip(GUM_window *win, struct GUM_box *box)
{
    win->x += box->cx - box->sx;
    win->y += box->cy - box->sy;
}

void gum_pop_clip(GUM_window *win, struct GUM_box *box)
{
    win->x -= box->cx - box->sx;
    win->y -= box->cy - box->sy;
}

void gum_draw_cell(GUM_window *win, GUM_cell *cell)
{
    GUM_skin *skin = gum_skin(cell);
    if (skin == NULL)
        return;

    if (cell->cachedSkin != skin) {
        cell->path = NULL;
        cell->gradient = NULL;
        cell->cachedSkin = skin;
    }

    RECT r;
    r.left = cell->box.x + win->x;
    r.top = cell->box.y + win->y;
    r.right = cell->box.x + cell->box.w + win->x;
    r.bottom = cell->box.y + cell->box.h + win->y;
    SetBkMode(win->hdc, TRANSPARENT);
    if (skin->bgcolor >= MIN_ALPHA) {
        HBRUSH brush = CreateSolidBrush(COLOR_REF(skin->bgcolor));
        FillRect(win->hdc, &r, brush);
    }

    if (skin->brcolor >= MIN_ALPHA) {
        SetDCPenColor(win->hdc, COLOR_REF(skin->brcolor));
        SetDCBrushColor(win->hdc, COLOR_REF(skin->brcolor));
        // HBRUSH brush = CreateSolidBrush(COLOR_REF(skin->brcolor));
        // SetDCBrushColor(win->hdc, brush);
        Rectangle(win->hdc, cell->box.x + win->x, cell->box.y + win->y, cell->box.x + cell->box.w + win->x, cell->box.y + cell->box.h + win->y);
    }

    if (cell->text) {
        TCHAR szBuf[64];
        mbstowcs(szBuf, cell->text, 64);
        DrawText(win->hdc, szBuf, wcslen(szBuf), &r, 0);
    }
}

void gum_draw_scrolls(GUM_window *win, GUM_cell *cell)
{
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */


#include <dirent.h>


void *opendir(const char *path)
{
    return NULL;
}

int closedir(void *dir)
{
    return 0;
}

struct dirent *readdir_r(void *dir, struct dirent *en)
{
    return en;
}

struct dirent readdir_en;

struct dirent * readdir(void *dir)
{
    return readdir_r(dir, &readdir_en);
}

const char *memrchr(const void *buf, int byte, size_t len)
{
    const char *ptr = (const char*)buf + len;
    while (ptr-- > (const char*)buf)
        if (*ptr == byte)
            return ptr;
    return NULL;
}

