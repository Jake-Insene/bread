#pragma once

#include "external/glcore.h"

#undef WGL_WGLEXT_PROTOTYPES
#include "external/wgl.h"

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <windowsx.h>
#include <xaudio2.h>
#include <dbghelp.h>
#include <WaaSApi.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <d3d12.h>

#undef far
#undef near
#undef max
#undef min

