#pragma once
#include "object/object.h"
#include "render/render_manager.h"


struct Texture2D;

struct RenderObject : Object
{
    struct InternalData
    {
        RenderItemID render_item;
    } data;

    void init(const CreateInfo& info);
    void deinit();

    RenderItemID get_render_item() const { return data.render_item; }

    void draw_sprite(const Transform2D& transform, Texture2D* texture, const Rect2D& rect, const Rect2D& src_rect, Color color, u32 flags);
};

