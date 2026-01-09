#pragma once
#include "math/color.h"
#include "math/mat4.h"
#include "math/rect_2d.h"
#include "math/vec4.h"
#include "mem/allocator.h"
#include "render/material_manager.h"
#include "render/render_manager.h"


struct Viewport;


struct SceneRenderer
{
    // Minimum for GLES
    // https://registry.khronos.org/OpenGL-Refpages/es3/html/glGet.xhtml
    static constexpr i32 MaxInstanceAttributeCount = 16;

#define CheckInstanceSize(type) \
    static_assert(\
        sizeof(type) <= (MaxInstanceAttributeCount * sizeof(Vector4)),\
        #type " is greater than 16 32-bit floating point vec4!"\
    );

    static constexpr u32 MaxInstancesPerBatch = 128 * 16;
    static constexpr u32 MaxPrimitivePointsPerBatch = 512 * 32;
    static constexpr u32 MaxCirclesPerBatch = 128 * 32;

    enum BatchFlag
    {
        FLAG_FLIP_H = 0x1U,
        FLAG_FLIP_V = 0x2U,
        FLAG_FONT_CHAR = 0x4U,
    };

    struct alignas(16) SpriteInstance
    {
        // attrib 0
        Vector2 transform_0;
        Vector2 transform_1;
        // attrib 1
        Vector2 transform_2;
        u32 unit;
        u32 flags;
        // attrib 2
        Rect2D rect;
        // attrib 3
        Rect2D src_rect;
        // attrib 4
        Color color;
        u32 padding[3];
    };
    CheckInstanceSize(SpriteInstance)

    struct alignas(16) SpriteUIInstance
    {
        // attrib 0
        Vector2 transform_0;
        Vector2 transform_1;
        // attrib 1
        Vector2 transform_2;
        u32 unit;
        u32 flags;
        // attrib 2
        Rect2D rect;
        // attrib 3
        Rect2D src_rect;
        // attrib 4
        Color color;
        u32 padding[3];
    };
    CheckInstanceSize(SpriteUIInstance)

    struct alignas(16) QuadInstance
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
        u32 padding1[3];
    };
    CheckInstanceSize(QuadInstance)

    struct alignas(16) PrimitivePoint
    {
        // attrib 0
        Vector2 point;
        Color color;
        u32 flags;
    };
    CheckInstanceSize(PrimitivePoint)

    struct alignas(16) PrimitiveCircle
    {
        // attrib 0
        Vector2 point;
        Color color;
        f32 radius;
    };
    CheckInstanceSize(PrimitiveCircle)

    struct alignas(16) ScreenInstance
    {
        u32 unit;
        u32 padding[3];
    };
    CheckInstanceSize(ScreenInstance)

    struct SceneUniform
    {
        Mat4 viewport_transform;
        Mat4 scene_transform;
        f32 time;
        u32 padding[3];
    };

    struct BatchBase
    {
        Graphics::PipelineID current_pipeline;
        Graphics::BufferID vb;
        u32 instance_count;
        u32 current_instance_count;
        u32 buffer_offset;
        u32 padding;
    };

    struct SpriteBatchBase : BatchBase
    {
        usize texture_index;
        Graphics::TextureID texture_units[16];
    };

    template<typename BatchInstance>
    struct Batch : BatchBase
    {
        Slice<BatchInstance> instances;
    };

    template<typename BatchInstance>
    struct SpriteBatch : SpriteBatchBase
    {
        Slice<BatchInstance> instances;
    };

    struct InternalData
    {
        mem::Allocator allocator;

        Graphics::BufferID global_ib;
        u32 usable_texture_units;

        MaterialManager::MaterialID item_material;
        Graphics::RenderTargetID render_targets[2];
        usize present_index;
        usize draw_index;

        Graphics::SwapChainID sc;
        Graphics::CommandBufferID command_buffer;
        Graphics::QueueID queue;

        SpriteBatch<SpriteInstance> sprite_batch;
        SpriteBatch<SpriteUIInstance> sprite_ui_batch;
        Batch<QuadInstance> quads_batch;
        Batch<PrimitivePoint> lines_batch;
        Batch<PrimitiveCircle> circles_batch;

        Vector2I render_target_size;

        SceneUniform scene_data;
        Graphics::BufferID scene_uniform;
        bool scene_data_ubo_update;
    };

    static inline InternalData data{};

    [[nodiscard]] static mem::Allocator get_allocator() { return data.allocator; }

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static void recreate_window();

    static void render_scene(Viewport* main_viewport);
    static void present_scene();

    static void _bind_material();
    static void _render_item(RenderManager::RenderItem& item);

    static void _update_viewport_transform(const Vector2I& viewport_size);
    static void _update_scene_uniform();

};