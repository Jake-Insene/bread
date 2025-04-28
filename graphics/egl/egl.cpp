#include "graphics/egl/egl.h"

#include "graphics/gles/gles_vtable.h"
#include "objects/scene_manager.h"

#if defined(ENGINE_ANDROID)
#include "platform/android/android_egl.h"
using PlatformEGL = AndroidEGL;
#elif defined(ENGINE_WIN32)
#include "platform/win32/win32_egl.h"
using PlatformEGL = Win32EGL;
#endif // ENGINE_ANDROID

void EGL::initialize(mem::Allocator& allocator)
{
    data.allocator = allocator;
    PlatformEGL::initialize(allocator);

    REQUIRED_LOAD(glGetIntegerv);
    REQUIRED_LOAD(glGetString);
    REQUIRED_LOAD(glGetStringi);

    REQUIRED_LOAD(glEnable);
    REQUIRED_LOAD(glDisable);
    REQUIRED_LOAD(glViewport);

    // Debug only
    REQUIRED_LOAD(glDebugMessageCallback);
    REQUIRED_LOAD(glDebugMessageControl);

    REQUIRED_LOAD(glCreateShader);
    REQUIRED_LOAD(glDeleteShader);
    REQUIRED_LOAD(glShaderSource);
    REQUIRED_LOAD(glCompileShader);
    REQUIRED_LOAD(glGetShaderiv);
    REQUIRED_LOAD(glGetShaderInfoLog);

    REQUIRED_LOAD(glCreateProgram);
    REQUIRED_LOAD(glDeleteProgram);
    REQUIRED_LOAD(glAttachShader);
    REQUIRED_LOAD(glLinkProgram);
    REQUIRED_LOAD(glGetProgramiv);
    REQUIRED_LOAD(glGetProgramInfoLog);

    REQUIRED_LOAD(glUseProgram);

    REQUIRED_LOAD(glGenBuffers);
    REQUIRED_LOAD(glDeleteBuffers);
    REQUIRED_LOAD(glBindBuffer);
    REQUIRED_LOAD(glBufferData);
    REQUIRED_LOAD(glBufferSubData);
    REQUIRED_LOAD(glBindBufferBase);

    REQUIRED_LOAD(glGenVertexArrays);
    REQUIRED_LOAD(glDeleteVertexArrays);
    REQUIRED_LOAD(glBindVertexArray);
    REQUIRED_LOAD(glVertexAttribPointer);
    REQUIRED_LOAD(glEnableVertexAttribArray);
    REQUIRED_LOAD(glVertexAttribDivisor);

    REQUIRED_LOAD(glGenTextures);
    REQUIRED_LOAD(glDeleteTextures);
    REQUIRED_LOAD(glBindTexture);
    REQUIRED_LOAD(glTexParameteri);
    REQUIRED_LOAD(glTexImage2D);

    REQUIRED_LOAD(glGenFramebuffers);
    REQUIRED_LOAD(glDeleteFramebuffers);
    REQUIRED_LOAD(glBindFramebuffer);
    REQUIRED_LOAD(glFramebufferTexture2D);
    REQUIRED_LOAD(glBlitFramebuffer);

    REQUIRED_LOAD(glClearColor);
    REQUIRED_LOAD(glClear);

    REQUIRED_LOAD(glActiveTexture);

    REQUIRED_LOAD(glDrawArrays);
    REQUIRED_LOAD(glDrawElementsInstanced);
}

void EGL::shutdown()
{
    PlatformEGL::shutdown();
}

void EGL::recreate_window_surface()
{
    PlatformEGL::recreate_window_surface();
}

void EGL::uncreate_window_surface()
{
    PlatformEGL::destroy_window_surface();
}

void EGL::present()
{
    PlatformEGL::present();
}

Vector2I EGL::get_surface_size()
{
    return data.surface_size;
}
