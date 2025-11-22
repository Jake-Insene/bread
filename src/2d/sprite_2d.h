#pragma once
#include "2d/object_2d.h"
#include "math/color.h"
#include "math/rect_2d.h"
#include "resource/texture.h"


struct Sprite2D : Object2D
{
    OBJECT(Sprite2D, Object2D);
    MARKS(MARK_RENDER);

    struct InternalData
    {
        Texture2D* texture;
    } data;

    /*
    * Sprite modulate color.
    */
    Color color{255, 255, 255, 255};
    /*
    * Sprite source rect, modified by set_texture(...)
    */
    Rect2D src_rect{};
    /*
    * If true the sprite is drawed with its center at the transform position, 
    * otherwise the sprite top left will be at the transform position.
    */
    bool centered = true;
    /*
    * Flip the entire sprite horizontally
    */
    bool flip_h = false;
    /*
    * Flip the entire sprite vertically
    */
    bool flip_v = false;
    
    void render();

    /*
    * Set the sprite texture.
    * 
    * @param new_texture The new sprite texture.
    */
    void set_texture(Texture2D* new_texture) Function(FunctionNormal);

    /*
    * @return The sprite current texture
    */
    [[nodiscard]] Texture2D* get_texture() const Function(FunctionNormal) 
    {
        return data.texture;
    }
};
