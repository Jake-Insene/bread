#include "platform/win32/win32_egl.h"

#include "graphics/gles/gles_vtable.h"
#include "engine/engine.h"


static inline HMODULE gl_lib = nullptr;

static inline Opaque get_proc_address(const char* name)
{
	Opaque proc = (Opaque)GetProcAddress(gl_lib, name);
	if (proc == NULL)
		return (Opaque)wglGetProcAddress(name);
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
   
	data.current_window = Engine::get_main_window().get_native_handle().cast<HWND>();
	data.device_context = GetDC(Win32EGL::data.current_window);

	gl_lib = LoadLibraryA("opengl32.dll");

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
    
	int format = ChoosePixelFormat(data.device_context, &pfd);
	SetPixelFormat(data.device_context, format, &pfd);

	HGLRC tmp_ctx = wglCreateContext(data.device_context);
	wglMakeCurrent(data.device_context, tmp_ctx);

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

	HGLRC real_context = wgl.wglCreateContextAttribsARB(data.device_context, 0, attribs);
	DebugAssert(real_context, "couldn't create the OpenGL context");

	wglDeleteContext(tmp_ctx);
	wglMakeCurrent(data.device_context, real_context);

	data.context = real_context;
	
	wgl.wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
}

void Win32EGL::shutdown()
{
	wglDeleteContext(data.context);

	ReleaseDC(data.current_window, data.device_context);

	FreeLibrary(gl_lib);

	data.current_window = nullptr;
	data.device_context = nullptr;
	data.context = nullptr;
}

void Win32EGL::recreate_window_surface()
{
}

void Win32EGL::destroy_window_surface()
{
}

void Win32EGL::present()
{
	wglSwapLayerBuffers(data.device_context, WGL_SWAP_MAIN_PLANE);
}

void Win32EGL::set_vsync(bool vsync)
{
	wgl.wglSwapIntervalEXT(vsync ? 1 : 0);
}


