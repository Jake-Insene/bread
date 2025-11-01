#pragma once
#include "core/header.h"
#include "mem/allocator.h"
#include "math/vec2.h"

struct Display
{
    struct __WindowTag {};

    static constexpr i32 DefaultWidth = 600;
    static constexpr i32 DefaultHeight = 800;

    static constexpr const char* DefaultTitle = "Bread!!!";

    using WindowID = ID<u32, __WindowTag>;

    struct VTable
    {
        VTFunc(void, initialize, const mem::Allocator&);
        VTFunc(void, shutdown);

        VTFunc(WindowID, window_create);
        VTFunc(Vector2I, window_get_size, WindowID);
        VTFunc(void, window_set_size, WindowID, const Vector2I&);
        VTFunc(Opaque, window_get_native_handle, WindowID);
    };

    struct InternalData
    {
        mem::Allocator allocator;
    };

    static inline VTable vtable;
    static inline InternalData data;

    static mem::Allocator get_allocator() { return data.allocator; }

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static WindowID window_create();
    static Vector2I window_get_size(WindowID window_id);
    static void window_set_size(WindowID window_id, const Vector2I& new_size);
    static MemoryAddress window_get_native_handle(WindowID window_id);
};
