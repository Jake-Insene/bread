#pragma once
#include "graphics/gles/gles_memory_allocator.h"
#include "graphics/render_manager.h"
#include "math/vec4.h"
#include "math/mat4.h"



struct Viewport;

struct GLESRenderer
{
    // Limits get from
    // https://registry.khronos.org/OpenGL-Refpages/es3/html/glGet.xhtml
    static constexpr i32 MaxInstanceAttributeCount = 16;

#define CheckInstanceSize(type) \
    static_assert(\
        sizeof(type) <= (MaxInstanceAttributeCount * sizeof(Vector4)),\
        #type " is greater than 16 32-bit floating point vec4!"\
    );

    static constexpr u32 MaxInstancesPerBatch = 128;
    static constexpr u32 MaxPrimitivePointsPerBatch = 128 * 8;
    static constexpr u32 MaxCirclesPerBatch = 128 * 4;

    enum BatchFlag
    {
        FLAG_FLIP_H = 0x1U,
        FLAG_FLIP_V = 0x2U,
        FLAG_FONT_CHAR = 0x4U,
    };

    enum BatchType
    {
        BATCH_SPRITE,
        BATCH_SPRITE_UI,
        BATCH_QUAD,
        BATCH_LINES,
        BATCH_CIRCLE,
    };
    
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
        Rect2D rect;
        // attrib 3
        Rect2D src_rect;
        // attrib 4
        Color color;
        u32 padding[3];
    };
    static constexpr usize SpriteInstanceAttribCount = 5;
    CheckInstanceSize(SpriteInstance);

    struct SpriteUIInstance
    {
        // attrib 0
        Vector2 transform_0;
        Vector2 transform_1;
        // attrib 1
        Vector2 transform_2;
        GLID unit;
        u32 flags;
        // attrib 2
        Rect2D rect;
        // attrib 3
        Rect2D src_rect;
        // attrib 4
        Color color;
        u32 padding[3];
    };
    static constexpr usize CanvasElementInstanceAttribCount = 5;
    CheckInstanceSize(SpriteUIInstance);

    struct QuadInstance
    {
        // attrib 0
        Vector2 transform_0;
        Vector2 transform_1;
        // attrib 1
        Vector2 transform_2;
        u32 padding[2];
        // attrib 2
        Rect2D rect;
        // attrib 3
        Color color;
    };
    static constexpr usize QuadInstanceAttribCount = 4;
    CheckInstanceSize(QuadInstance);

    struct PrimitivePoint
    {
        // attrib 0
        Vector2 point;
        Color color;
        u32 flags;
    };
    static constexpr usize PrimitivePointAttribCount = 1;
    CheckInstanceSize(PrimitivePoint);

    struct PrimitiveCircle
    {
        // attrib 0
        Vector2 point;
        Color color;
        f32 radius;
    };
    static constexpr usize PrimitiveCircleAttribCount = 1;
    CheckInstanceSize(PrimitiveCircle);

    struct SceneUniform
    {
        Mat4 viewport_transform;
        Mat4 scene_transform;
        f32 time;
    };
    
    struct SpriteBatch
    {
        GLID vao;
        GLID instance_buffer_object;
        
        u32 count;
        i32 texture_index;
        
        GLID texture_units[MaxInstancesPerBatch];
        
        Slice<SpriteInstance> instances;
    };

    struct SpriteUIBatch
    {
        GLID vao;
        GLID instance_buffer_object;

        u32 count;
        i32 texture_index;

        GLID texture_units[MaxInstancesPerBatch];

        Slice<SpriteUIInstance> instances;
    };
    
    struct QuadBatch
    {
        GLID vao;
        GLID instance_buffer_object;
        
        u32 count;
        
        Slice<QuadInstance> instances;
    };

    struct PrimitivePointBatch
    {
        GLID vao;
        GLID instance_buffer_object;

        u32 count;

        Slice<PrimitivePoint> primitives;
    };

    struct PrimitiveCircleBatch
    {
        GLID vao;
        GLID instance_buffer_object;

        u32 count;

        Slice<PrimitiveCircle> primitives;
    };
    
    struct ExecutionState
    {
        i32 frame_index;
    };
    
    struct InternalData
    {
        mem::Allocator allocator;
        
        GLID global_quad_ibo;
        i32 usable_texture_units;
        
        MaterialID render_material;
        MaterialID item_material;
        GLID current_sprite_program;
        GLID current_sprite_ui_program;
        GLID current_quad_program;
        GLID current_lines_program;
        GLID current_circles_program;
        
        SpriteBatch sprite_batch;
        SpriteUIBatch sprite_ui_batch;
        QuadBatch quad_batch;
        PrimitivePointBatch primitive_batch;
        PrimitiveCircleBatch primitive_circle_batch;

        GLID scene_data_ubo;
        bool scene_data_ubo_update;
    
        SceneUniform scene_data;
    
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

    static void update_viewport_transform(const Vector2I& viewport_size);
    static void render(Viewport* viewport);

    static void _bind_program(GLID program);
    static void _bind_scene_buffer();
    static void _update_scene_uniform();

    static void _end_sprite_batch();
    static void _end_sprite_ui_batch();
    static void _end_quad_batch();
    static void _end_lines_batch();
    static void _end_circles_batch();

    static void _bind_material();

    static void _render_item_draw(RenderManager::RenderItem& item);

};
