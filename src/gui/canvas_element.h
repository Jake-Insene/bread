#pragma once
#include "collections/string.h"
#include "math/transform_2d.h"
#include "render/render_manager.h"



struct Canvas;
struct Font;


struct [[nodiscard]] CanvasElement
{
    struct InternalData
    {
        Font* font;
        i32 font_size;
    } data;

    Canvas* owner;
    String text;
    
    Transform2D transform;
    Rect2D element_rect;
    Color element_color;

    usize visible_characters;

    void init(const mem::Allocator& allocator);
    void deinit();

    void set_font(Font* new_font);
    [[nodiscard]] Font* get_font() const { return data.font; }

    void set_font_size(i32 new_font_size);
    [[nodiscard]] i32 get_font_size() const { return data.font_size; }

    void draw(RenderItemID render_item);
};
