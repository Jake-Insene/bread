#pragma once
#include "2d/object_2d.h"
#include "io/texture.h"
#include "math/color.h"
#include "math/rect_2d.h"


struct Sprite2D : Object2D
{
    OBJECT(Sprite2D, Object2D);

    // As everything in a struct is public we need to hide data
    // that should not be modified/access directly, this also
    // resolve some namespace problems.
    struct InternalData
    {
        Texture2D* texture;
    } data;

    Color color{255, 255, 255, 255};
    Rect2D src_rect{};
    
    void init(const CreateInfo& info);
    void render();

    void set_texture(Texture2D* new_texture);
    [[nodiscard]] Texture2D* get_texture() const { return data.texture; }
};
