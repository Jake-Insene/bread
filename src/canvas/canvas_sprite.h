#pragma once
#include "canvas/canvas_object.h"


struct Texture2D;


struct CanvasSprite : CanvasObject
{
    OBJECT(CanvasSprite, CanvasObject);

    struct InternalData
    {
        Texture2D* texture;
    } data;

    Color color{ 255, 255, 255, 255 };
    Rect2D src_rect{};
    bool flip_h = false;
    bool flip_v = false;

    void init(const CreateInfo&);

    void render();

    void set_texture(Texture2D* new_texture);
    [[nodiscard]] Texture2D* get_texture() const { return data.texture; }
};