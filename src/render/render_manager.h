#pragma once
#include "collections/free_list.h"
#include "collections/array.h"
#include "math/color.h"
#include "math/transform_2d.h"
#include "math/mat4.h"
#include "math/rect_2d.h"
#include "math/vec4.h"
#include "render/material_manager.h"


struct Viewport;
using RenderItemID = ID<u32, struct _RenderItemTag>;


struct RenderManager
{
    static constexpr usize DefaultCommandBufferSize = 1024;
    static constexpr usize DefaultChildSize = 4;

    // Minimum for GLES
    // https://registry.khronos.org/OpenGL-Refpages/es3/html/glGet.xhtml
    static constexpr i32 MaxInstanceAttributeCount = 16;

#define CheckInstanceSize(type) \
    static_assert(\
        sizeof(type) <= (MaxInstanceAttributeCount * sizeof(Vector4)),\
        #type " is greater than 16 32-bit floating point vec4!"\
    );

    static constexpr u32 MaxInstancesPerBatch = 128 * 16;
    static constexpr u32 MaxPrimitivePointsPerBatch = 128 * 32;
    static constexpr u32 MaxCirclesPerBatch = 128 * 32;

    enum BatchFlag
    {
        FLAG_FLIP_H = 0x1U,
        FLAG_FLIP_V = 0x2U,
        FLAG_FONT_CHAR = 0x4U,
    };

    enum RenderLayerMask
    {
        RENDER_LAYER_0 = Bit(0),
        RENDER_LAYER_1 = Bit(1),
        RENDER_LAYER_2 = Bit(2),
        RENDER_LAYER_3 = Bit(3),
        RENDER_LAYER_4 = Bit(4),
        RENDER_LAYER_5 = Bit(5),
        RENDER_LAYER_6 = Bit(6),
        RENDER_LAYER_7 = Bit(7),

        RENDER_LAYER_DEFAULT = RENDER_LAYER_0,
        RENDER_LAYER_COUNT = 8,
    };

    enum RenderFlags
    {
        RENDER_FLAG_NONE = 0,
        RENDER_FLAG_FLIP_H = Bit(0),
        RENDER_FLAG_FLIP_V = Bit(1),
        RENDER_FLAG_FONT_CHAR = Bit(2),
    };

    struct [[nodiscard]] RenderItem
    {
        static constexpr usize DefaultCommandBufferSize = 1024;
        static constexpr usize DefaultChildSize = 4;

        enum CommandType
        {
            CMD_RECT,
            CMD_LINE,
            CMD_CIRCLE,
            CMD_SPRITE,
            CMD_UI_SPRITE,
        };

        struct alignas(16) Command
        {
            CommandType type;
            usize next;
        };

        struct CommandRect : Command
        {
            Transform2D transform;
            Rect2D rect;
            Color color;
        };

        struct CommandLine : Command
        {
            Vector2 point1;
            Vector2 point2;
            Color color;
        };

        struct CommandCircle : Command
        {
            Vector2 center;
            f32 radius;
            Color color;
        };

        struct CommandSprite : Command
        {
            Transform2D transform;
            Graphics::TextureID texture;
            Rect2D rect;
            // Source in texture coordinates
            Rect2D src_rect;
            Color mod_color;
            RenderFlags flags;
        };

        // Same as sprite but type is CMD_UI_SPRITE, 
        // the renderer should handle the special case
        struct CommandUISprite : CommandSprite
        {};

        RenderItemID self = RenderItemID();
        mem::Allocator allocator;
        
        RenderItemID parent = RenderItemID();
        Array<RenderItemID> childs;
        
        RenderLayerMask layers = RenderLayerMask(0);
        MaterialManager::MaterialID material;

        // For ordering
        RenderItemID prev;
        RenderItemID next;
        i32 z_index;

        Slice<u8> command_buffer = {};
        usize offset = 0;
        usize last_element = usize(-1);

        void destroy();

        [[nodiscard]] Command* begin() { return reinterpret_cast<Command*>(command_buffer.ptr()); }
        [[nodiscard]] Command* end() { return reinterpret_cast<Command*>(command_buffer.ptr() + offset); }

        [[nodiscard]] Command* get_command_at(usize cmd_offset)
        {
            return reinterpret_cast<Command*>(command_buffer.ptr() + cmd_offset);
        }

        Command* alloc_base(usize command_size);

        template<typename T>
        T* alloc()
        {
            return reinterpret_cast<T*>(alloc_base(sizeof(T)));
        }

        void reset();
    };

    struct RenderLayer
    {
        Array<RenderItemID> items;
    };

    struct OT
    {
        RenderLayer layers[RENDER_LAYER_COUNT];
        RenderLayerMask active_layers;
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
    static constexpr usize SpriteInstanceAttribCount = 5;
    CheckInstanceSize(SpriteInstance);

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
    static constexpr usize CanvasElementInstanceAttribCount = 5;
    CheckInstanceSize(SpriteUIInstance);

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
    static constexpr usize QuadInstanceAttribCount = 4;
    CheckInstanceSize(QuadInstance);

    struct alignas(16) PrimitivePoint
    {
        // attrib 0
        Vector2 point;
        Color color;
        u32 flags;
    };
    static constexpr usize PrimitivePointAttribCount = 1;
    CheckInstanceSize(PrimitivePoint);

    struct alignas(16) PrimitiveCircle
    {
        // attrib 0
        Vector2 point;
        Color color;
        f32 radius;
    };
    static constexpr usize PrimitiveCircleAttribCount = 1;
    CheckInstanceSize(PrimitiveCircle);

    struct alignas(16) SceneUniform
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
    };

    struct SpriteBatchBase : BatchBase
    {
        u32 texture_index;
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
        FreeList<RenderItem, RenderItemID> items;

        OT order_table;

        Graphics::BufferID global_ib;
        u32 usable_texture_units;

        SceneUniform scene_data;
        Graphics::BufferID scene_uniform;
        bool scene_data_ubo_update;

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
    };

    static inline InternalData data{};

    [[nodiscard]] static mem::Allocator get_allocator() { return data.allocator; }
    [[nodiscard]] static OT& get_order_table() { return data.order_table; }

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static void recreate_window();
    
    static void update_viewport_transform(const Vector2I& viewport_size);

    static void render_scene(Viewport* main_viewport);
    static void reset_commands();

    static RenderItemID create_item();
    static void destroy_item(RenderItemID render_item_id);

    static void item_set_parent(RenderItemID render_item_id, RenderItemID parent_id);
    static void item_set_material(RenderItemID render_item_id, MaterialManager::MaterialID material);
    static MaterialManager::MaterialID item_get_material(RenderItemID render_item_id);

    static void item_set_layers(RenderItemID render_item_id, RenderLayerMask layers);
    static RenderLayerMask item_get_layers(RenderItemID render_item_id);

    static void render_item_draw_rect(RenderItemID render_item_id, const Transform2D& transform,
        const Rect2D& dest_rect, Color color);

    static void render_item_draw_line(RenderItemID render_item_id,
        const Vector2& point1, const Vector2& point2, Color color);

    static void render_item_draw_circle(RenderItemID render_item_id,
        const Vector2& center, f32 radius, Color color);

    static void render_item_draw_sprite(RenderItemID render_item_id, const Transform2D& transform, Graphics::TextureID texture,
        const Rect2D& rect, const Rect2D& src_rect, Color mod_color, RenderFlags flags);

    static void render_item_draw_ui_sprite(RenderItemID render_item_id, const Transform2D& transform, Graphics::TextureID texture,
        const Rect2D& rect, const Rect2D& src_rect, Color mod_color, RenderFlags flags);

    static void _bind_material();

    static void _render_item(RenderItem& item);

    static RenderItem& _get_item(RenderItemID render_item_id);

    static void _update_scene_uniform();
};