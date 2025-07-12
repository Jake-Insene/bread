#pragma once
#include "canvas/canvas_object.h"


struct Texture2D;


struct CanvasSprite : CanvasObject
{
    OBJECT(CanvasSprite, CanvasObject);

    // As everything in a struct is public we need to hide data
    // that should not be modified/access directly, this also
    // resolve some namespace problems.
    struct InternalData
    {
        Texture2D* texture;
    } data;

    Color color{ 255, 255, 255, 255 };
    Rect2D src_rect{};
    bool flip_v = false;
    bool flip_h = false;

    void enter();
    void render();

    void set_texture(Texture2D* new_texture);
    [[nodiscard]] Texture2D* get_texture() const { return data.texture; }

    u32 _get_render_flags();
};