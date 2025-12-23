#pragma once
#include "platform/platform_header.h"
#include "display/display.h"


struct AndroidDisplay
{
    struct InternalData
    {
        mem::Allocator allocator;

        // GLES use this for the viewport
        Vector2I window_size;
    };

    static inline InternalData data;

    static void update_native_size(Vector2I new_size);
};