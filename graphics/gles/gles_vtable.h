#pragma  once

#include "platform/platform_header.h"


#if defined(ENGINE_ANDROID)
inline EGLAPI __eglMustCastToProperFunctionPointerType (*platform_get_proc)(const char* name) = eglGetProcAddress;
#elif defined(ENGINE_WIN32)
inline void* (*platform_get_proc)(const char* name) = nullptr;
#endif 

#define REQUIRED_LOAD(name) \
    gl.name = (decltype(gl.name))platform_get_proc(#name); \
    FailOn(gl.name == nullptr, "Couldn't load the function %s", #name);

struct GLESVTable {
    PFNGLGETINTEGERVPROC glGetIntegerv;
    PFNGLGETSTRINGPROC glGetString;
    PFNGLGETSTRINGIPROC glGetStringi;

    PFNGLENABLEPROC glEnable;
    PFNGLDISABLEPROC glDisable;
    PFNGLVIEWPORTPROC glViewport;

    // Debug only
    PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
    PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl;

    PFNGLCREATESHADERPROC glCreateShader;
    PFNGLDELETESHADERPROC glDeleteShader;
    PFNGLSHADERSOURCEPROC glShaderSource;
    PFNGLCOMPILESHADERPROC glCompileShader;
    PFNGLGETSHADERIVPROC glGetShaderiv;
    PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;

    PFNGLCREATEPROGRAMPROC glCreateProgram;
    PFNGLDELETEPROGRAMPROC glDeleteProgram;
    PFNGLATTACHSHADERPROC glAttachShader;
    PFNGLLINKPROGRAMPROC glLinkProgram;
    PFNGLGETPROGRAMIVPROC glGetProgramiv;
    PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
    PFNGLUSEPROGRAMPROC glUseProgram;

    PFNGLGENBUFFERSPROC glGenBuffers;
    PFNGLDELETEBUFFERSPROC glDeleteBuffers;
    PFNGLBINDBUFFERPROC glBindBuffer;
    PFNGLBUFFERDATAPROC glBufferData;
    PFNGLBUFFERSUBDATAPROC glBufferSubData;
    PFNGLBINDBUFFERBASEPROC glBindBufferBase;

    PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
    PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
    PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
    PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
    PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
    PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;

    PFNGLGENTEXTURESPROC glGenTextures;
    PFNGLDELETETEXTURESPROC glDeleteTextures;
    PFNGLBINDTEXTUREPROC glBindTexture;
    PFNGLTEXPARAMETERIPROC glTexParameteri;
    PFNGLTEXIMAGE2DPROC glTexImage2D;

    PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
    PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
    PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
    PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
    PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;

    PFNGLCLEARCOLORPROC glClearColor;
    PFNGLCLEARPROC glClear;

    PFNGLACTIVETEXTUREPROC glActiveTexture;

    PFNGLDRAWARRAYSPROC glDrawArrays;
    PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
};

inline GLESVTable gl{};