#pragma once
#include "core/header.h"
#include "math/vec2.h"

struct Display
{
    static constexpr i32 DefaultWidth = 600;
    static constexpr i32 DefaultHeight = 800;

    static constexpr const char* DefaultTitle = "Bread!!!";

    using WindowID = ID<u32>;

    struct VTable
    {
        VTFunc(void, initialize, const mem::Allocator&);
        VTFunc(void, shutdown);

        VTFunc(WindowID, window_create);
        VTFunc(Vector2I, window_get_size, WindowID);
        VTFunc(void, window_set_size, WindowID, const Vector2I&);
        VTFunc(void*, window_get_native_handle, WindowID);
    };

    struct InternalData
    {
        mem::Allocator allocator;
    };

    static inline VTable vtable;
    static inline InternalData data;

    static mem::Allocator get_allocator() { return data.allocator; }

    static void initialize(const mem::Allocator& allocator);
    
    VTFuncDefS(shutdown);
    VTFuncDefRetS(WindowID, window_create);
    VTFuncDefArg1RetS(Vector2I, window_get_size, WindowID);
    VTFuncDefArg2S(window_set_size, WindowID, const Vector2I&);
    VTFuncDefArg1RetS(void*, window_get_native_handle, WindowID);
};
