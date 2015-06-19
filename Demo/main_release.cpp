#include "stdafx.h"
#include "shader_code.h"
#include "floats.h"
#include <immintrin.h>

#define XRES 1920
#define YRES 1080

static PIXELFORMATDESCRIPTOR pfd =
{
    sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_SUPPORT_OPENGL, PFD_TYPE_RGBA,
    32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
};

const float gStep = p0d01;
__declspec(align(16)) __m128 gCoords = { -1.0f, -1.0f, -p0d99, 1.0f };
__declspec(align(16)) const __m128 gCoordsOffset = { gStep, 0.0f, gStep, 0.0f };

__declspec(naked) void entrypoint()
{
    __asm enter 0x50, 0;
    __asm pushad;
    {
        // create window
        HWND hWnd = CreateWindowA("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);
        HDC hDC = GetDC(hWnd);

        // initalize opengl
        SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
        wglMakeCurrent(hDC, wglCreateContext(hDC));

        // init shader
        auto glCreateShaderProgramv = (PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv");
        GLuint prog = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, (const GLchar**)&shader_glsl);

        ((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(prog);

        for (;;)
        {
            // render - by drawing a rectangle
            glRectfv(gCoords.m128_f32, gCoords.m128_f32 + 2);

            // move rendering region
            gCoords = _mm_add_ps(gCoords, gCoordsOffset);

            // display
            glFlush();
            SwapBuffers(hDC);

            if (GetAsyncKeyState(VK_ESCAPE))
                ExitProcess(0);
        }
    }
}
