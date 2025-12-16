#include "graphics/egl/egl.h"

#include "collections/string_view.h"
#include "graphics/gles/gles_vtable.h"


#if defined(BREAD_ANDROID)
#include "platform/android/android_egl.h"
#elif defined(BREAD_WIN32)
#include "platform/win32/win32_egl.h"
#endif

static inline EGL::VTable get_vtable()
{
#if defined(BREAD_ANDROID)
    return AndroidEGL::get_vtable();
#elif defined(BREAD_WIN32)
    return Win32EGL::get_vtable();
#endif
}

void EGL::initialize(const mem::Allocator& allocator)
{
    data.allocator = allocator;

    vtable = get_vtable();
    vtable.initialize(allocator);

    EGL_REQUIRED_LOAD(glGetIntegerv);
    EGL_REQUIRED_LOAD(glGetString);
    EGL_REQUIRED_LOAD(glGetStringi);
    EGL_REQUIRED_LOAD(glPixelStorei);

    EGL_REQUIRED_LOAD(glEnable);
    EGL_REQUIRED_LOAD(glDisable);
    EGL_REQUIRED_LOAD(glBlendFunc);
    EGL_REQUIRED_LOAD(glCullFace);
    EGL_REQUIRED_LOAD(glViewport);

    // Debug only
    EGL_NOT_REQUIRED_LOAD(glDebugMessageCallback);
    EGL_NOT_REQUIRED_LOAD(glDebugMessageControl);

    EGL_REQUIRED_LOAD(glCreateShader);
    EGL_REQUIRED_LOAD(glDeleteShader);
    EGL_REQUIRED_LOAD(glShaderSource);
    EGL_REQUIRED_LOAD(glCompileShader);
    EGL_REQUIRED_LOAD(glGetShaderiv);
    EGL_REQUIRED_LOAD(glGetShaderInfoLog);

    EGL_REQUIRED_LOAD(glCreateProgram);
    EGL_REQUIRED_LOAD(glDeleteProgram);
    EGL_REQUIRED_LOAD(glAttachShader);
    EGL_REQUIRED_LOAD(glLinkProgram);
    EGL_REQUIRED_LOAD(glGetProgramiv);
    EGL_REQUIRED_LOAD(glGetProgramInfoLog);

    EGL_REQUIRED_LOAD(glUseProgram);

    EGL_REQUIRED_LOAD(glGenBuffers);
    EGL_REQUIRED_LOAD(glDeleteBuffers);
    EGL_REQUIRED_LOAD(glBindBuffer);
    EGL_REQUIRED_LOAD(glBufferData);
    EGL_REQUIRED_LOAD(glBufferSubData);
    EGL_REQUIRED_LOAD(glBindBufferBase);
    EGL_REQUIRED_LOAD(glMapBufferRange);
    EGL_REQUIRED_LOAD(glUnmapBuffer);

    EGL_REQUIRED_LOAD(glGenVertexArrays);
    EGL_REQUIRED_LOAD(glDeleteVertexArrays);
    EGL_REQUIRED_LOAD(glBindVertexArray);
    EGL_REQUIRED_LOAD(glVertexAttribPointer);
    EGL_REQUIRED_LOAD(glVertexAttribFormat);
    EGL_REQUIRED_LOAD(glEnableVertexAttribArray);
    EGL_REQUIRED_LOAD(glVertexAttribBinding);
    EGL_REQUIRED_LOAD(glVertexAttribDivisor);
    EGL_REQUIRED_LOAD(glBindVertexBuffer);

    EGL_REQUIRED_LOAD(glGenTextures);
    EGL_REQUIRED_LOAD(glDeleteTextures);
    EGL_REQUIRED_LOAD(glBindTexture);
    EGL_REQUIRED_LOAD(glTexParameteri);
    EGL_REQUIRED_LOAD(glTexImage2D);
    EGL_REQUIRED_LOAD(glTexStorage2D);

    EGL_REQUIRED_LOAD(glGenFramebuffers);
    EGL_REQUIRED_LOAD(glDeleteFramebuffers);
    EGL_REQUIRED_LOAD(glBindFramebuffer);
    EGL_REQUIRED_LOAD(glFramebufferTexture2D);
    EGL_REQUIRED_LOAD(glCheckFramebufferStatus);
    EGL_REQUIRED_LOAD(glBlitFramebuffer);

    EGL_REQUIRED_LOAD(glClearColor);
    EGL_REQUIRED_LOAD(glClear);

    EGL_REQUIRED_LOAD(glActiveTexture);

    EGL_REQUIRED_LOAD(glDrawArrays);
    EGL_REQUIRED_LOAD(glDrawArraysInstanced);
    EGL_REQUIRED_LOAD(glDrawElementsInstanced);

    EGL_REQUIRED_LOAD(glLineWidth);
}

