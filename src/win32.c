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
#include <windowsx.h>
#include <tchar.h>

struct GUM_window {
    HWND hwnd;
    HDC hdc;
    int x, y;
    PAINTSTRUCT ps;
};

static TCHAR szWindowClass[] = _T("gum");
static TCHAR szTitle[] = _T("Application");
static WNDCLASSEX wcex;
HINSTANCE appInstance;

int app_main(int argc, char **argv);

LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT   uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LIBAPI void gum_win32_setup(_In_ HINSTANCE hInstance)
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
        MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Win32 Guided Tour"), 0);
        exit(-1);
    }
}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
HFONT  hFont;

GUM_window *gum_create_surface(int width, int height)
{
    HWND hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, width + (16), height + (39), NULL, NULL,
                             appInstance, NULL);
    if (!hwnd) {
        DWORD err = GetLastError();
        MessageBox(NULL, _T("Can't create the window!"), _T("Win32 Guided Tour"), 0);
        return NULL;
    }

    GUM_window *win = (GUM_window *)malloc(sizeof(GUM_window));
    win->hwnd = hwnd;
    win->hdc = GetDC(win->hwnd);
    //RECT rcClient;
    //GetClientRect(hwnd, &rcClient);

    int sz = MulDiv(10, GetDeviceCaps(win->hdc, LOGPIXELSY), 72);
    hFont = CreateFont(sz, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                       VARIABLE_PITCH, TEXT("Arial"));
    return win;
}

void gum_destroy_surface(GUM_window *win)
{
    ReleaseDC(win->hwnd, win->hdc);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

int gum_event_poll(GUM_window *win, GUM_event *event, int timeout)
{
    MSG msg;
    if (!GetMessage(&msg, win->hwnd, 0, 0)) {
        event->type = GUM_EV_DESTROY;
        return -1;
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
    case WM_TIMER:
    case 0x60:
    case WM_NCLBUTTONDOWN:
    case 799:
    case 1847:
    case 1848:
    case 257:
    case 49303:
    case 49305:
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
    RECT r;
    r.left = x;
    r.right = x + w;
    r.top = y;
    r.bottom = y + h;
    InvalidateRect(win->hwnd, &r, TRUE);
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
    win->hdc = BeginPaint(win->hwnd, &win->ps);
}

void gum_end_paint(GUM_window *win)
{
    EndPaint(win->hwnd, &win->ps);
}

void gum_push_clip(GUM_window *win, GUM_box *box)
{
    win->x += box->cx - box->sx;
    win->y += box->cy - box->sy;
}

void gum_pop_clip(GUM_window *win, GUM_box *box)
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
    if (cell->image == NULL && cell->img_src != NULL)
        cell->image = gum_image(cell->img_src);

    RECT r;
    r.left = cell->box.x + win->x;
    r.top = cell->box.y + win->y;
    r.right = cell->box.x + cell->box.w + win->x;
    r.bottom = cell->box.y + cell->box.h + win->y;
    SetBkMode(win->hdc, TRANSPARENT);
    if (cell->image != NULL) {
        BITMAP bm;
        HBITMAP bmp = (HBITMAP)cell->image;
        HDC hdcMem = CreateCompatibleDC(win->hdc);
        HBITMAP hbmOld = SelectObject(hdcMem, bmp);
        GetObject(bmp, sizeof(bm), &bm);
        StretchBlt(win->hdc, cell->box.x + win->x, cell->box.y + win->y, cell->box.w, cell->box.h, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);

    } else if (skin->bgcolor >= MIN_ALPHA && skin->grcolor >= MIN_ALPHA) {
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

        SelectObject(win->hdc, hFont);
        SetTextColor(win->hdc, COLOR_REF(skin->txcolor));
        DrawText(win->hdc, szBuf, wcslen(szBuf), &r, alg);
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
            szBuf[i] = '\\';
    HBITMAP bmp = (HBITMAP)LoadImage(NULL, szBuf, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    return bmp;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

const char *memrchr(const void *buf, int byte, size_t len)
{
    const char *ptr = (const char *)buf + len;
    while (ptr-- > (const char *)buf)
        if (*ptr == byte)
            return ptr;
    return NULL;
}



/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */ 
#if 0
// Creates a stream object initialized with the data from an executable resource.
IStream * CreateStreamOnResource(LPCTSTR lpName, LPCTSTR lpType)
{
    // initialize return value
    IStream * ipStream = NULL;

    // find the resource
    HRSRC hrsrc = FindResource(NULL, lpName, lpType);
    if (hrsrc == NULL)
        goto Return;

    // load the resource
    DWORD dwResourceSize = SizeofResource(NULL, hrsrc);
    HGLOBAL hglbImage = LoadResource(NULL, hrsrc);
    if (hglbImage == NULL)
        goto Return;

    // lock the resource, getting a pointer to its data
    LPVOID pvSourceResourceData = LockResource(hglbImage);
    if (pvSourceResourceData == NULL)
        goto Return;

    // allocate memory to hold the resource data
    HGLOBAL hgblResourceData = GlobalAlloc(GMEM_MOVEABLE, dwResourceSize);
    if (hgblResourceData == NULL)
        goto Return;

    // get a pointer to the allocated memory
    LPVOID pvResourceData = GlobalLock(hgblResourceData);
    if (pvResourceData == NULL)
        goto FreeData;

    // copy the data from the resource to the new memory block
    CopyMemory(pvResourceData, pvSourceResourceData, dwResourceSize);
    GlobalUnlock(hgblResourceData);

    // create a stream on the HGLOBAL containing the data
    if (SUCCEEDED(CreateStreamOnHGlobal(hgblResourceData, TRUE, &ipStream)))
        goto Return;

FreeData:
    // couldn't create stream; free the memory
    GlobalFree(hgblResourceData);

Return:
    // no need to unlock or free the resource
    return ipStream;
}


// Loads a PNG image from the specified stream (using Windows Imaging Component).
IWICBitmapSource * LoadBitmapFromStream(IStream * ipImageStream)
{
    // initialize return value
    IWICBitmapSource * ipBitmap = NULL;

    // load WIC's PNG decoder
    IWICBitmapDecoder * ipDecoder = NULL;
    if (FAILED(CoCreateInstance(CLSID_WICPngDecoder, NULL, CLSCTX_INPROC_SERVER, __uuidof(ipDecoder), reinterpret_cast<void**>(&ipDecoder))))
        goto Return;

    // load the PNG
    if (FAILED(ipDecoder->Initialize(ipImageStream, WICDecodeMetadataCacheOnLoad)))
        goto ReleaseDecoder;

    // check for the presence of the first frame in the bitmap
    UINT nFrameCount = 0;
    if (FAILED(ipDecoder->GetFrameCount(&nFrameCount)) || nFrameCount != 1)
        goto ReleaseDecoder;

    // load the first frame (i.e., the image)
    IWICBitmapFrameDecode * ipFrame = NULL;
    if (FAILED(ipDecoder->GetFrame(0, &ipFrame)))
        goto ReleaseDecoder;

    // convert the image to 32bpp BGRA format with pre-multiplied alpha
    //   (it may not be stored in that format natively in the PNG resource,
    //   but we need this format to create the DIB to use on-screen)
    WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, ipFrame, &ipBitmap);
    ipFrame->Release();

ReleaseDecoder:
    ipDecoder->Release();
Return:
    return ipBitmap;
}

// Creates a 32 - bit DIB from the specified WIC bitmap.
HBITMAP CreateHBITMAP(IWICBitmapSource * ipBitmap)
{
    // initialize return value
    HBITMAP hbmp = NULL;

    // get image attributes and check for valid image
    UINT width = 0;
    UINT height = 0;
    if (FAILED(ipBitmap->GetSize(&width, &height)) || width == 0 || height == 0)
        goto Return;

    // prepare structure giving bitmap information (negative height indicates a top-down DIB)
    BITMAPINFO bminfo;
    ZeroMemory(&bminfo, sizeof(bminfo));
    bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bminfo.bmiHeader.biWidth = width;
    bminfo.bmiHeader.biHeight = -((LONG)height);
    bminfo.bmiHeader.biPlanes = 1;
    bminfo.bmiHeader.biBitCount = 32;
    bminfo.bmiHeader.biCompression = BI_RGB;

    // create a DIB section that can hold the image
    void * pvImageBits = NULL;
    HDC hdcScreen = GetDC(NULL);
    hbmp = CreateDIBSection(hdcScreen, &bminfo, DIB_RGB_COLORS, &pvImageBits, NULL, 0);
    ReleaseDC(NULL, hdcScreen);
    if (hbmp == NULL)
        goto Return;

    // extract the image into the HBITMAP
    const UINT cbStride = width * 4;
    const UINT cbImage = cbStride * height;
    if (FAILED(ipBitmap->CopyPixels(NULL, cbStride, cbImage, static_cast<BYTE *>(pvImageBits))))
    {
        // couldn't extract image; delete HBITMAP
        DeleteObject(hbmp);
        hbmp = NULL;
    }

Return:
    return hbmp;
}

// Loads the PNG containing the splash image into a HBITMAP.
HBITMAP LoadPNGImage(TCHAR *file)
{
    HBITMAP hbmpSplash = NULL;

    // load the PNG image data into a stream
    IStream * ipImageStream = CreateStreamOnResource(file, _T("PNG"));
    if (ipImageStream == NULL)
        goto Return;

    // load the bitmap with WIC
    IWICBitmapSource * ipBitmap = LoadBitmapFromStream(ipImageStream);
    if (ipBitmap == NULL)
        goto ReleaseStream;

    // create a HBITMAP containing the image
    hbmpSplash = CreateHBITMAP(ipBitmap);
    ipBitmap->Release();

ReleaseStream:
    ipImageStream->Release();
Return:
    return hbmpSplash;
}
#endif
