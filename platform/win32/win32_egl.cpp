#include "platform/win32/win32_egl.h"

#include "core/array.h"
#include "graphics/gles/gles_vtable.h"
#include "platform/win32/win32_engine.h"
#include "objects/scene_manager.h"

static inline HMODULE gllib = nullptr;

static inline void* get_proc_address(const char* name)
{
	void* proc = (void*)GetProcAddress(gllib, name);
	if (proc == NULL)
		return (void*)wglGetProcAddress(name);
	return proc;
}

EGL::VTable Win32EGL::get_vtable()
{
	return EGL::VTable
	{
		.initialize = &Win32EGL::initialize,
		.shutdown = &Win32EGL::shutdown,

		.recreate_window_surface = &Win32EGL::recreate_window_surface,
		.destroy_window_surface = &Win32EGL::destroy_window_surface,
		.present = &Win32EGL::present,
		.set_vsync = &Win32EGL::set_vsync,
	};
}

void Win32EGL::initialize(const mem::Allocator&)
{
	platform_get_proc = &get_proc_address;

    Win32EGL::data.current_window = (HWND)Engine::data.main_window.get_native_handle();
	Win32EGL::data.device_context = GetDC(Win32EGL::data.current_window);

	gllib = LoadLibraryA("opengl32.dll");

	// Initialize OpenGL ES and EGL
	// Format R8G8B8A8 D24 S8

    PIXELFORMATDESCRIPTOR pfd{};
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
    
	int format = ChoosePixelFormat(Win32EGL::data.device_context, &pfd);
	SetPixelFormat(Win32EGL::data.device_context, format, &pfd);

	HGLRC tmp_ctx = wglCreateContext(Win32EGL::data.device_context);
	wglMakeCurrent(Win32EGL::data.device_context, tmp_ctx);

	wgl.wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#if DEBUG
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
		0
	};

	HGLRC real_context = wgl.wglCreateContextAttribsARB(Win32EGL::data.device_context, 0, attribs);
	DebugAssert(real_context, "couldn't create the OpenGL context");

	wglDeleteContext(tmp_ctx);
	wglMakeCurrent(Win32EGL::data.device_context, real_context);

	Win32EGL::data.context = real_context;

	RECT rect{};
	GetClientRect(Win32EGL::data.current_window, &rect);
	
	wgl.wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
}

void Win32EGL::shutdown()
{
	wglDeleteContext(Win32EGL::data.context);

	ReleaseDC(Win32EGL::data.current_window, Win32EGL::data.device_context);

	FreeLibrary(gllib);

	Win32EGL::data.current_window = nullptr;
	Win32EGL::data.device_context = nullptr;
	Win32EGL::data.context = nullptr;
}

void Win32EGL::recreate_window_surface()
{
}

void Win32EGL::destroy_window_surface()
{
}

void Win32EGL::present()
{
	wglSwapLayerBuffers(Win32EGL::data.device_context, WGL_SWAP_MAIN_PLANE);
}

void Win32EGL::set_vsync(bool vsync)
{
	wgl.wglSwapIntervalEXT(vsync ? 1 : 0);
}


