#pragma once
#include "platform/platform_header.h"
#include "engine/engine.h"

struct Win32Engine : Engine
{
	struct InternalData
	{
	};

	static InternalData data;
};
