#pragma  once
#include "platform/platform_header.h"



using GLESFuncGeneric = void(*)();
#if defined(BREAD_ANDROID)
inline EGLAPI __eglMustCastToProperFunctionPointerType (*platform_get_proc)(const char* name) = nullptr;
#elif defined(BREAD_WIN32)
inline GLESFuncGeneric(*platform_get_proc)(const char* name) = nullptr;
#endif 

#define EGL_REQUIRED_LOAD(name) \
    gl.name = reinterpret_cast<decltype(gl.name)>(platform_get_proc(#name)); \
    FailOn(gl.name == nullptr, "Couldn't load the function {}", StringView(#name));

#define EGL_NOT_REQUIRED_LOAD(name) \
    gl.name = reinterpret_cast<decltype(gl.name)>(platform_get_proc(#name));

struct GLESVTable
{
    PFNGLGETINTEGERVPROC glGetIntegerv;
    PFNGLGETSTRINGPROC glGetString;
    PFNGLGETSTRINGIPROC glGetStringi;
    PFNGLPIXELSTOREIPROC glPixelStorei;

    PFNGLENABLEPROC glEnable;
    PFNGLDISABLEPROC glDisable;
    PFNGLBLENDFUNCPROC glBlendFunc;
    PFNGLCULLFACEPROC glCullFace;
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
    PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
    PFNGLUNMAPBUFFERPROC glUnmapBuffer;

    PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
    PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
    PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
    PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
    PFNGLVERTEXATTRIBFORMATPROC glVertexAttribFormat;
    PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
    PFNGLVERTEXATTRIBBINDINGPROC glVertexAttribBinding;
    PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
    PFNGLBINDVERTEXBUFFERPROC glBindVertexBuffer;

    PFNGLGENTEXTURESPROC glGenTextures;
    PFNGLDELETETEXTURESPROC glDeleteTextures;
    PFNGLBINDTEXTUREPROC glBindTexture;
    PFNGLTEXPARAMETERIPROC glTexParameteri;
    PFNGLTEXIMAGE2DPROC glTexImage2D;
    PFNGLTEXSTORAGE2DPROC glTexStorage2D;

    PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
    PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
    PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
    PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
    PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
    PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;

    PFNGLCLEARCOLORPROC glClearColor;
    PFNGLCLEARPROC glClear;

    PFNGLACTIVETEXTUREPROC glActiveTexture;

    PFNGLDRAWARRAYSPROC glDrawArrays;
    PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;
    PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;

    PFNGLLINEWIDTHPROC glLineWidth;
};

inline GLESVTable gl{};