#pragma once
#include "graphics/command_interface.h"

struct GLESCommandProcessor
{
    static constexpr u32 MaxInstancesPerBatch = 128;
    static constexpr u32 MaxPrimitivePointsPerBatch = 1024;
    
    struct SpriteInstance
    {
        // attrib 0
        Vector2 transform_0;
        Vector2 transform_1;
        // attrib 1
        Vector2 transform_2;
        u32 unit;
        u32 flags;
        // attrib 2
        Vector2 texture_extent;
        Color color;
        u32 padding;
        // attrib 3
        Rect2D src_rect;
    };
    static constexpr usize SpriteInstanceAttribCount = 4;
    
    static_assert(
        sizeof(SpriteInstance) <= (16*sizeof(Vector4)),
        "SpriteInstance is greater than 16 32-bit floating vec4!"
    );
    
    struct QuadInstance
    {
        // attrib 0
        Vector2 transform_0;
        Vector2 transform_1;
        // attrib 1
        Vector2 transform_2;
        Vector2 size;
        // attrib 2
        Color color;
    };
    static constexpr usize QuadInstanceAttribCount = 3;

    struct PrimitivePoint
    {
        // attrib 0
        Vector2 point;
        Color color;
        u32 flags;
    };
    static constexpr usize PrimitivePointAttribCount = 1;
    
    struct SceneUniform
    {
        Mat4 screen_transform;
        Mat4 scene_transform;
    };
    
    struct SpriteBatch
    {
        u32 vao;
        u32 instancebo;
        u32 program;
        
        u32 count;
        u32 texture_index;
        
        u32 texture_units[MaxInstancesPerBatch];
        
        Slice<u8> instances;
    };
    
    struct QuadBatch
    {
        u32 vao;
        u32 instancebo;
        u32 program;
        
        u32 count;
        
        Slice<u8> instances;
    };

    struct PrimitiveBatch
    {
        u32 vao;
        u32 instancebo;
        u32 program;

        u32 count;

        Slice<u8> primitives;
    };
    
    struct ExecutionState
    {
        u32 last_fbo;
        u32 current_fbo;
        ResourceID current_fb;
    };
    
    struct InternalData
    {
        mem::Allocator allocator;
        
        u32 global_quad_ibo;
        
        SpriteBatch sprite_batch;
        QuadBatch quad_batch;
        PrimitiveBatch primitive_batch;
        u32 blit_program;

        u32 scene_data_ubo;
        bool scene_data_ubo_update;
    
        SceneUniform scene_data;
    
        Array<RenderCommand> commands;
    
        // Execution state
        ExecutionState state;
    };
    
    static inline InternalData data;
    
    static void initialize(mem::Allocator allocator);
    static void shutdown();
    
    static void end_sprite_batch();
    static void end_quad_batch();
    static void end_primitive_batch();

    static void update_scene_uniform();

    static void render();
    
    [[nodiscard]] static u32 get_current_fbo() { return data.state.current_fbo; }
    static void set_current_fbo(u32 new_object) { data.state.current_fbo = new_object; }
    [[nodiscard]] static ResourceID get_current_fb() { return data.state.current_fb; }
    static void set_current_fb(ResourceID new_object) { data.state.current_fb = new_object; }
};
