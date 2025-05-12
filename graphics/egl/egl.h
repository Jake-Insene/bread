#pragma once
#include "core/header.h"
#include "math/vec2.h"

struct EGL
{
    struct VTable
    {
        VTFunc(void, initialize, const mem::Allocator&);
        VTFunc(void, shutdown);
        
        VTFunc(void, recreate_window_surface);
        VTFunc(void, destroy_window_surface);
        VTFunc(void, present);
        VTFunc(void, set_vsync, bool);
    };

    struct InternalData
    {
        mem::Allocator allocator;
    };
    
    static inline VTable vtable;
    static inline InternalData data;
    
    static void initialize(const mem::Allocator& allocator);
    
    VTFuncDefS(shutdown);

    VTFuncDefS(recreate_window_surface);
    VTFuncDefS(destroy_window_surface);
    VTFuncDefS(present);

    VTFuncDefArg1S(set_vsync, bool);
};
