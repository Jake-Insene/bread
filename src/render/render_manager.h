#pragma once
#include "collections/free_list.h"
#include "collections/array.h"
#include "math/color.h"
#include "math/transform_2d.h"
#include "math/rect_2d.h"
#include "render/material_manager.h"


struct Viewport;
using RenderItemID = ID<u32, struct _RenderItemTag>;


struct RenderManager
{
    static constexpr usize DefaultCommandBufferSize = 1024;
    static constexpr usize DefaultChildSize = 4;

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
        RENDER_LAYER_DEFAULT_UI = RENDER_LAYER_0,
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

    struct InternalData
    {
        mem::Allocator allocator;
        FreeList<RenderItem, RenderItemID> items;

        OT order_table;
    };

    static inline InternalData data{};

    [[nodiscard]] static mem::Allocator get_allocator() { return data.allocator; }
    [[nodiscard]] static OT& get_order_table() { return data.order_table; }

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static void recreate_window();
    
    static void update_viewport_transform(const Vector2I& viewport_size);

    static void render_scene(Viewport* main_viewport);
    static void present_scene();
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