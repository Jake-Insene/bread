#pragma once
#include "graphics/command_interface.h"
#include "graphics/gles/gles_memory_allocator.h"

struct GLESCommandProcessor
{
    static constexpr u32 MaxInstancesPerBatch = 128;
    static constexpr u32 MaxPrimitivePointsPerBatch = 128 * 8;
    
    struct SpriteInstance
    {
        // attrib 0
        Vector2 transform_0;
        Vector2 transform_1;
        // attrib 1
        Vector2 transform_2;
        GLID unit;
        u32 flags;
        // attrib 2
        Vector2 texture_extent;
        Vector2 dest_extent;
        // attrib 3
        Rect2D src_rect;
        // attrib 4
        Color color;
        u32 padding[3];
    };
    static constexpr usize SpriteInstanceAttribCount = 5;
    
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

    struct CanvasElementInstance
    {
        // attrib 0
        Vector2 transform_0;
        Vector2 transform_1;
        // attrib 1
        Vector2 transform_2;
        GLID unit;
        u32 flags;
        // attrib 2
        Vector2 texture_extent;
        Vector2 dest_extent;
        // attrib 3
        Rect2D src_rect;
        // attrib 4
        Color color;
        u32 padding[3];
    };
    static constexpr usize CanvasElementInstanceAttribCount = 5;

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
        GLID vao;
        GLID instance_buffer_object;
        GLID program;
        
        u32 count;
        i32 texture_index;
        
        GLID texture_units[MaxInstancesPerBatch];
        
        Slice<SpriteInstance> instances;
    };

    struct CanvasElementBatch
    {
        GLID vao;
        GLID instance_buffer_object;
        GLID program;

        u32 count;
        i32 texture_index;

        GLID texture_units[MaxInstancesPerBatch];

        Slice<CanvasElementInstance> instances;
    };
    
    struct QuadBatch
    {
        GLID vao;
        GLID instance_buffer_object;
        GLID program;
        
        u32 count;
        
        Slice<QuadInstance> instances;
    };

    struct PrimitiveBatch
    {
        GLID vao;
        GLID instance_buffer_object;
        GLID program;

        u32 count;

        Slice<PrimitivePoint> primitives;
    };
    
    struct ExecutionState
    {
        GLID last_fbo;
        GLID current_fbo;
        Graphics::RenderTargetID current_fb;
    };
    
    struct InternalData
    {
        mem::Allocator allocator;
        
        GLID global_quad_ibo;
        i32 usable_texture_units;
        
        SpriteBatch sprite_batch;
        CanvasElementBatch canvas_element_batch;
        QuadBatch quad_batch;
        PrimitiveBatch primitive_batch;

        GLID scene_data_ubo;
        bool scene_data_ubo_update;
    
        SceneUniform scene_data;
    
        Array<RenderCommand> commands;
    
        // Execution state
        ExecutionState state;

        struct
        {
            u64 draw_call_count;
        } debug;
    };
    
    static inline InternalData data;
    
    static void initialize(mem::Allocator allocator);
    static void shutdown();

    static void recreate_window_transform(Vector2I window_size);

    static void bind_program(GLID program);
    static void bind_scene_buffer();
    static void update_scene_uniform();

    static void end_sprite_batch();
    static void end_canvas_element_batch();
    static void end_quad_batch();
    static void end_primitive_batch();

    static void render();
    
    [[nodiscard]] static GLID get_current_fbo() { return data.state.current_fbo; }
    static void set_current_fbo(GLID new_object) { data.state.current_fbo = new_object; }
    [[nodiscard]] static Graphics::RenderTargetID get_current_fb() { return data.state.current_fb; }
    static void set_current_fb(Graphics::RenderTargetID new_object) { data.state.current_fb = new_object; }
};
