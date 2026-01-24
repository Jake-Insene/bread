#include "platform/android/android_egl.h"

#include "collections/array.h"
#include "platform/android/android_display.h"
#include "platform/android/android_engine.h"
#include "scene/scene_manager.h"
#include "graphics/gles/gles_vtable.h"


static constexpr EGLint context_attributes_es31[] =
{
    EGL_CONTEXT_MAJOR_VERSION, 3, // OpenGL ES 3.1
    EGL_CONTEXT_MINOR_VERSION, 1,
#if !defined(DEBUG)
    EGL_CONTEXT_OPENGL_NO_ERROR_KHR, EGL_TRUE,
#endif
    EGL_NONE
};

static constexpr EGLint context_attributes_es32[] =
{
    EGL_CONTEXT_MAJOR_VERSION, 3, // OpenGL ES 3.2
    EGL_CONTEXT_MINOR_VERSION, 2,
#if !defined(DEBUG)
    EGL_CONTEXT_OPENGL_NO_ERROR_KHR, EGL_TRUE,
#endif
    EGL_NONE
};


EGL::VTable AndroidEGL::get_vtable()
{
    return EGL::VTable
    {
        .initialize = &AndroidEGL::initialize,
        .shutdown = &AndroidEGL::shutdown,

        .recreate_window_surface = &AndroidEGL::recreate_window_surface,
        .destroy_window_surface = &AndroidEGL::destroy_window_surface,
        .present = &AndroidEGL::present,

        .set_vsync = &AndroidEGL::set_vsync,
    };
}

void AndroidEGL::initialize(const mem::Allocator&)
{
    platform_get_proc = eglGetProcAddress;

    // Initialize OpenGL ES and EGL
    // Format R8G8B8A8 D24 S8
    const EGLint attributes[] =
    {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 0,
        EGL_STENCIL_SIZE, 0,
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

    DebugAssert(num_configs != 0, "it should be at least one config.");
    EGLint i = 0;
    for (; i < (EGLint)supported_configs.count; i++)
    {
        auto& cfg = supported_configs.get(i);
        EGLint r, g, b, d, s;
        if (eglGetConfigAttrib(data.display, cfg, EGL_RED_SIZE, &r) &&
            eglGetConfigAttrib(data.display, cfg, EGL_GREEN_SIZE, &g) &&
            eglGetConfigAttrib(data.display, cfg, EGL_BLUE_SIZE, &b) &&
            eglGetConfigAttrib(data.display, cfg, EGL_DEPTH_SIZE, &d) && 
            eglGetConfigAttrib(data.display, cfg, EGL_STENCIL_SIZE, &s) &&
            r == 8 && g == 8 && b == 8 && d == 0 && s == 0)
        {
            data.config = supported_configs.get(i);
            break;
        }
    }
    
    supported_configs.destroy();
    FailOn(i == num_configs, "unable to initialize EGLConfig");
    
    data.surface = eglCreateWindowSurface(
        data.display, data.config,
        AndroidEngine::data.app->window, nullptr
    );

    data.context = eglCreateContext(
            data.display, data.config,
            EGL_NO_CONTEXT, context_attributes_es32
    );

    if(data.context == nullptr)
    {
        // Fallback to GL ES 3.1
        data.context = eglCreateContext(
            data.display, data.config,
            EGL_NO_CONTEXT, context_attributes_es31
        );
        EGL::data.gles32 = false;
    }
    else
    {
        EGL::data.gles32 = true;
    }
    FailOn(data.context == nullptr, "unable to setting up the EGL context");

    EGLBoolean result = eglMakeCurrent(
            data.display, data.surface,
            data.surface, data.context
    );
    FailOn(result == EGL_FALSE, "unable to setting up the EGL context");

    if(AndroidEngine::data.app->window != nullptr)
    {
        AndroidDisplay::update_native_size(
            Vector2I(
                ANativeWindow_getWidth(AndroidEngine::data.app->window),
                ANativeWindow_getHeight(AndroidEngine::data.app->window)
            )
        );
    }
}

void AndroidEGL::shutdown() {
    if (data.display != EGL_NO_DISPLAY) {
        eglMakeCurrent(
                data.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT
        );
        if (data.context != EGL_NO_CONTEXT) {
            eglDestroyContext(data.display, data.context);
        }
        if (data.surface != EGL_NO_SURFACE) {
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
    if(data.surface == nullptr)
    {
        data.surface = eglCreateWindowSurface(
                data.display, data.config,
                AndroidEngine::data.app->window, nullptr
        );
    }

    if(AndroidEngine::data.app->window != nullptr)
    {
        AndroidDisplay::update_native_size(
                Vector2I(
                        ANativeWindow_getWidth(AndroidEngine::data.app->window),
                        ANativeWindow_getHeight(AndroidEngine::data.app->window)
                )
        );
    }

    EGLBoolean result = eglMakeCurrent(
        data.display, data.surface,
        data.surface, data.context
    );
    FailOn(result == EGL_FALSE, "unable to setting up the EGL context");
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

void AndroidEGL::set_vsync(bool enable)
{
    eglSwapInterval(data.display, enable);
}

