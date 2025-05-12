#pragma once
#include "platform/platform_header.h"
#include "engine/engine.h"

struct Win32Engine : Engine
{
	struct InternalData
	{
	};

	static InternalData data;

	static Engine::VTable get_vtable();

	static Vector2I get_window_pos();
	static void set_window_size(const Vector2I& new_size);
};
