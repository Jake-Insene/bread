#pragma once
#include "Platform/platform_header.h"
#include "display/display.h"


struct AndroidDisplay
{
    struct InternalData
    {
        Mem::Allocator& allocator;

        GPU::SurfaceID window_surface;
        Vector2I window_size;
    };

    static inline InternalData data;

    static void update_native_size(const Vector2I& new_size);
};