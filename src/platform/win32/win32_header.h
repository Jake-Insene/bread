#pragma once

#include "external/glcore.h"

#undef WGL_WGLEXT_PROTOTYPES
#include "external/wgl.h"

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <windowsx.h>
#include <xaudio2.h>

#undef far
#undef near
#undef max
#undef min

