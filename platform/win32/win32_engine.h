#pragma once
#include "platform/platform_header.h"
#include "engine/engine.h"

struct Win32Engine : Engine
{
	static inline HWND window = nullptr;
};
