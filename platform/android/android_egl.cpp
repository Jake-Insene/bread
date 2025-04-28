#include "platform/android/android_egl.h"

#include "core/array.h"
#include "objects/scene_manager.h"
#include "platform/android/android_engine.h"

#include "android/native_window_jni.h"

void AndroidEGL::initialize(mem::Allocator&)
{
    // Initialize OpenGL ES and EGL
    // Format R8G8B8A8 D24 S8
    const EGLint attributes[] =
    {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    EGLint num_configs = 0;
    
    data.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(data.display, nullptr, nullptr);
    
    eglChooseConfig(data.display, attributes, nullptr, 0, &num_configs);
    auto supported_configs = Array<EGLConfig>::with_size(
        EGL::data.allocator, num_configs
    );
    supported_configs.resize(num_configs);
    
    eglChooseConfig(
        data.display, attributes, supported_configs.slice().ptr(),
        num_configs, &num_configs
    );

    DebugAssert(num_configs != 0, "It should be at least one config.");
    EGLint i = 0;
    for (; i < (EGLint)supported_configs.count; i++)
    {
        auto& cfg = supported_configs[i];
        EGLint r, g, b, d, s;
        if (eglGetConfigAttrib(data.display, cfg, EGL_RED_SIZE, &r) &&
            eglGetConfigAttrib(data.display, cfg, EGL_GREEN_SIZE, &g) &&
            eglGetConfigAttrib(data.display, cfg, EGL_BLUE_SIZE, &b) &&
            eglGetConfigAttrib(data.display, cfg, EGL_DEPTH_SIZE, &d) && 
            eglGetConfigAttrib(data.display, cfg, EGL_STENCIL_SIZE, &s) &&
            r == 8 && g == 8 && b == 8 && d == 24 && s == 8)
        {
            data.config = supported_configs[i];
            break;
        }
    }
    
    supported_configs.destroy();
    FailOn(i == num_configs, "Unable to initialize EGLConfig");
    
    data.surface = eglCreateWindowSurface(
        data.display, data.config,
        AndroidEngine::app->window, nullptr
    );

    const EGLint context_attributes[] =
    {
        EGL_CONTEXT_MAJOR_VERSION, 3, // OpenGL ES 3.2
        EGL_CONTEXT_MINOR_VERSION, 2,
#if defined(NDEBUG)
        EGL_CONTEXT_OPENGL_NO_ERROR_KHR, EGL_TRUE,
#endif
        EGL_NONE
    };
    
    data.context = eglCreateContext(
        data.display, data.config,
        EGL_NO_CONTEXT, context_attributes
    );

    EGLBoolean result = eglMakeCurrent(
        data.display, data.surface,
        data.surface, data.context
    );
    FailOn(result == EGL_FALSE, "Unable to setting up the EGL context");
    
    eglQuerySurface(data.display, data.surface, EGL_WIDTH, &EGL::data.surface_size.x);
    eglQuerySurface(data.display, data.surface, EGL_HEIGHT, &EGL::data.surface_size.y);
}

void AndroidEGL::shutdown()
{
    if (data.display != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(
            data.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT
        );
        if (data.context != EGL_NO_CONTEXT)
        {
            eglDestroyContext(data.display, data.context);
        }
        if (data.surface != EGL_NO_SURFACE)
        {
            eglDestroySurface(data.display, data.surface);
        }
        eglTerminate(data.display);
    }
    
    data.display = EGL_NO_DISPLAY;
    data.context = EGL_NO_CONTEXT;
    data.surface = EGL_NO_SURFACE;
    data.config = nullptr;
}

void AndroidEGL::recreate_window_surface()
{
    data.surface = eglCreateWindowSurface(
        data.display, data.config,
        AndroidEngine::app->window, nullptr
    );
    
    EGLBoolean result = eglMakeCurrent(
        data.display, data.surface,
        data.surface, data.context
    );
    FailOn(result == EGL_FALSE, "Unable to setting up the EGL context");
    
    eglQuerySurface(data.display, data.surface, EGL_WIDTH, &EGL::data.surface_size.x);
    eglQuerySurface(data.display, data.surface, EGL_HEIGHT, &EGL::data.surface_size.y);
}

void AndroidEGL::destroy_window_surface()
{
    if (data.surface != EGL_NO_SURFACE)
    {
        eglDestroySurface(data.display, data.surface);
    }
    
    data.surface = EGL_NO_SURFACE;
    
    eglMakeCurrent(data.display, EGL_NO_SURFACE, EGL_NO_SURFACE, data.context);
}

void AndroidEGL::present()
{
    eglSwapBuffers(data.display, data.surface);
}


