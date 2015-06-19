#include "stdafx.h"
#include "shader_code.h"

/**
 * This code was taken from here: http://iquilezles.org/www/material/isystem1k4k/isystem1k4k.htm
 */

#define XRES 1280
#define YRES 720

typedef struct
{
    HINSTANCE   hInstance;
    HDC         hDC;
    HGLRC       hRC;
    HWND        hWnd;
    int         full;
    char        wndclass[4];
} WININFO;

static WININFO wininfo = { 0, 0, 0, 0, 0, { 'i', 'q', '_', 0 } };

static PIXELFORMATDESCRIPTOR pfd =
{
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL,
    PFD_TYPE_RGBA,
    32,
    0, 0, 0, 0, 0, 0, 8, 0,
    32, 0, 0, 0, 0,
    0, // depth
    0,
    0,
    PFD_MAIN_PLANE,
    0, 0, 0, 0
};


static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_SYSCOMMAND && (wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER))
        return(0);

    if (uMsg == WM_CLOSE || uMsg == WM_DESTROY || (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE))
    {
        PostQuitMessage(0);
        return(0);
    }

    if (uMsg == WM_SIZE)
    {
        glViewport(0, 0, lParam & 65535, lParam >> 16);
    }

    if (uMsg == WM_CHAR || uMsg == WM_KEYDOWN)
    {
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
            return(0);
        }
    }

    return(DefWindowProc(hWnd, uMsg, wParam, lParam));
}


static void window_end()
{
    if (wininfo.hRC)
    {
        wglMakeCurrent(0, 0);
        wglDeleteContext(wininfo.hRC);
    }

    if (wininfo.hDC) ReleaseDC(wininfo.hWnd, wininfo.hDC);
    if (wininfo.hWnd) DestroyWindow(wininfo.hWnd);

    UnregisterClassA(wininfo.wndclass, wininfo.hInstance);

    if (wininfo.full)
    {
        ChangeDisplaySettings(0, 0);
        while (ShowCursor(1) < 0);
    }
}

static int window_init()
{
    unsigned int    PixelFormat;
    DWORD            dwExStyle, dwStyle;
    DEVMODE            dmScreenSettings;
    RECT            rec;

    WNDCLASSA        wc;

    ZeroMemory(&wc, sizeof(WNDCLASSA));
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = wininfo.hInstance;
    wc.lpszClassName = wininfo.wndclass;
    wc.hbrBackground = 0;

    if (!RegisterClassA(&wc))
        return(0);

    if (wininfo.full)
    {
        dmScreenSettings.dmSize = sizeof(DEVMODE);
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmPelsWidth = XRES;
        dmScreenSettings.dmPelsHeight = YRES;

        if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
            return(0);

        dwExStyle = WS_EX_APPWINDOW;
        dwStyle = WS_VISIBLE | WS_POPUP;

        while (ShowCursor(0) >= 0);    // hide cursor
    }
    else
    {
        dwExStyle = 0;
        dwStyle = WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_OVERLAPPED;
        dwStyle = WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_POPUP;

    }

    rec.left = 0;
    rec.top = 0;
    rec.right = XRES;
    rec.bottom = YRES;

    AdjustWindowRect(&rec, dwStyle, 0);

    wininfo.hWnd = CreateWindowExA(dwExStyle, wc.lpszClassName, "reducto", dwStyle,
                                   (GetSystemMetrics(SM_CXSCREEN) - rec.right + rec.left) >> 1,
                                   (GetSystemMetrics(SM_CYSCREEN) - rec.bottom + rec.top) >> 1,
                                   rec.right - rec.left, rec.bottom - rec.top, 0, 0, wininfo.hInstance, 0);

    if (!wininfo.hWnd)
        return(0);

    if (!(wininfo.hDC = GetDC(wininfo.hWnd)))
        return(0);

    if (!(PixelFormat = ChoosePixelFormat(wininfo.hDC, &pfd)))
        return(0);

    if (!SetPixelFormat(wininfo.hDC, PixelFormat, &pfd))
        return(0);

    if (!(wininfo.hRC = wglCreateContext(wininfo.hDC)))
        return(0);

    if (!wglMakeCurrent(wininfo.hDC, wininfo.hRC))
        return(0);

    return 1;
}

PFNGLCREATESHADERPROGRAMVPROC glCreateShaderProgramv;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocation;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameteriv;
PFNGLGETINFOLOGARBPROC glGetInfoLog;

int WINAPI WinMain(HINSTANCE instance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    int done = 0;
    int        result;
    char    info[1536];

    wininfo.hInstance = GetModuleHandle(0);

    if (!window_init())
    {
        window_end();
        MessageBoxA(0, "window_init()!", "error", MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }

    glCreateShaderProgramv = (PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv");
    glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONARBPROC)wglGetProcAddress("glGetUniformLocation");
    glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
    glGetObjectParameteriv = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
    glGetInfoLog = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");

    int program_id = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, (const GLchar**)&shader_glsl);

    sprintf_s(info, "Program ID: %i\n ", program_id);
    OutputDebugStringA(info);

    glGetObjectParameteriv(program_id, GL_OBJECT_LINK_STATUS_ARB, &result);
    glGetInfoLog(program_id, 1024, NULL, (char*)info);
    if (!result)
    {
        MessageBoxA(0, info, "error", 0);
        return 1;
    }

    glUseProgram(program_id);

    float offset = -1.0f;
    while (!done)
    {
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) done = 1;
            TranslateMessage(&msg);

            DispatchMessage(&msg);
        }

        //glUniform1i(glGetUniformLocation(program_id, VAR_TIME), ++time);

        if (offset < 2.0f)
        {
            glRectf(offset, -1.0f, offset + 0.02f, 1.0f);
            offset += 0.02f;
        }

        glFlush();
        SwapBuffers(wininfo.hDC);
    }

    window_end();
    return 0;
}
