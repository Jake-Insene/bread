#pragma once
#include "collections/array.h"
#include "graphics/graphics.h"
#include "platform/platform_header.h"
#include "resource/image.h"

#define GLESDebugInfo(...) DebugInfo("GLESDriver: " __VA_ARGS__)


struct GLESDriver
{   
    struct GLLimits
    {
        i32 max_texture_units;
    };
    
    struct InternalData
    {
        mem::Allocator allocator;
        
        GLLimits limits;
    };
    
    static inline InternalData data;

    static Graphics::VTable get_vtable();

    [[nodiscard]] static mem::Allocator& get_allocator() { return data.allocator; }

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static void recreate();
    static void destroy();
    
    static void render();
    static void present();
    
    static void add_cmd(const RenderCommand& cmd);

    static void _init_context();
};

