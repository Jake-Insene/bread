#pragma once
#include "collections/free_list.h"
#include "graphics/render_target.h"
#include "math/color.h"
#include "math/transform_2d.h"
#include "math/rect_2d.h"


/*
	Contains a world to render to.
*/
struct [[nodiscard]] Viewport
{
    static constexpr usize DefaultCommandBufferSize = 1024 * 16;

    enum ViewportLayerMask
    {
        VIEWPORT_LAYER_0 = Bit(0),
        VIEWPORT_LAYER_1 = Bit(1),
        VIEWPORT_LAYER_2 = Bit(2),
        VIEWPORT_LAYER_3 = Bit(3),
        VIEWPORT_LAYER_4 = Bit(4),
        VIEWPORT_LAYER_5 = Bit(5),
        VIEWPORT_LAYER_6 = Bit(6),
        VIEWPORT_LAYER_7 = Bit(7),  

        VIEWPORT_LAYER_DEFAULT = VIEWPORT_LAYER_0,
        VIEWPORT_LAYER_COUNT = 8,
    };

    enum RenderFlags
    {
        RENDER_FLAG_NONE = 0,
        RENDER_FLAG_FLIP_H = Bit(0),
        RENDER_FLAG_FLIP_V = Bit(1),
        RENDER_FLAG_FONT_CHAR = Bit(2),
    };

    struct RenderItem
    {
        enum CommandType
        {
            CMD_RECT,
            CMD_SPRITE,
            CMD_UI_SPRITE,
        };

        struct alignas(16) Command
        {
            CommandType type;
            Command* next;
        };

        struct CommandRect : Command
        {
            Transform2D transform;
            Rect2D rect;
            Color color;
        };

        struct CommandSprite : Command
        {
            Transform2D transform;
            TextureID texture;
            Rect2D rect;
            // Source in texture coordinates
            Rect2D src_rect;
            Color mod_color;
            RenderFlags flags;
        };

        // Same as sprite but type is CMD_UI_SPRITE, 
        // the renderer should handle the special case
        struct CommandUISprite : CommandSprite
        {
        };

        RenderItemID self = RenderItemID::InvalidID;
        ViewportLayerMask layers = ViewportLayerMask(0);

        Slice<u8> command_buffer = {};
        usize offset = 0;
        Command* begin = nullptr;
        Command* end = nullptr;

        template<typename T>
        T* alloc()
        {
            T* new_cmd = reinterpret_cast<T*>(command_buffer.add(offset).ptr());
            new_cmd->next = nullptr;
            offset += sizeof(T);

            if (begin == nullptr)
            {
                begin = new_cmd;
                end = new_cmd;
            }
            else
            {
                end->next = new_cmd;
                end = new_cmd;
            }

            return new_cmd;
        }

        void reset()
        {
            begin = nullptr;
            end = nullptr;
            offset = 0;
        }
    };

    struct ViewportLayer
    {
        Array<RenderItemID> items;
    };

    struct OT
    {
        ViewportLayer layers[VIEWPORT_LAYER_COUNT];
        ViewportLayerMask active_layers;
    };

    mem::Allocator allocator;
    FreeList<RenderItem, RenderItemID> items;

    RenderTarget rt;
    OT order_table;
    Transform2D scene_transform;

    Vector2I viewport_size;
    Color clear_color;

    bool must_sync;

    static Viewport create_from_render_target(const mem::Allocator& allocator, RenderTarget rt_id);
    void destroy();

    void reset_commands();
    [[nodiscard]] OT& get_order_table() { return order_table; }

    void set_scene_transform(const Transform2D& transform);
    Transform2D get_scene_transform() const { return scene_transform; }

    void set_size(const Vector2I& new_size);
    Vector2I get_size() const;

    RenderItemID create_item(ViewportLayerMask layers);
    void destroy_item(RenderItemID render_item_id);

    void item_set_layers(RenderItemID render_item_id, ViewportLayerMask layers);
    ViewportLayerMask item_get_layers(RenderItemID render_item_id);

    void render_item_draw_sprite(RenderItemID render_item_id, const Transform2D& transform, TextureID texture, 
        const Rect2D& rect, const Rect2D& src_rect, Color mod_color, RenderFlags flags);

    void render_item_draw_ui_sprite(RenderItemID render_item_id, const Transform2D& transform, TextureID texture,
        const Rect2D& rect, const Rect2D& src_rect, Color mod_color, RenderFlags flags);

    void render_item_draw_rect(RenderItemID render_item_id, const Transform2D& transform, 
        const Rect2D& dest_rect, Color color);
};