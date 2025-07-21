#pragma once
#include "canvas/canvas_object.h"


struct Texture2D;


struct ProgressBar : CanvasObject
{
	OBJECT(ProgressBar, CanvasObject);

    static constexpr f32 DefaultMin = 0.f;
    static constexpr f32 DefaultMax = 100.f;

    // As everything in a struct is public we need to hide data
    // that should not be modified/access directly, this also
    // resolve some namespace problems.
    struct InternalData
    {
        Texture2D* bg_texture;
        Texture2D* fill_texture;

        f32 min_value = DefaultMin;
        f32 max_value = DefaultMax;
        f32 value;
    } data;

    Color color{ 255, 255, 255, 255 };

    void init(const CreateInfo&);

    void render();

    void set_background_texture(Texture2D* new_bg_texture);
    [[nodiscard]] Texture2D* get_background_texture() const { return data.bg_texture; }

    void set_fill_texture(Texture2D* new_fill_texture);
    [[nodiscard]] Texture2D* get_fill_texture() const { return data.fill_texture; }

    void set_min(f32 new_min);
    [[nodiscard]] f32 get_min() const { return data.min_value; }

    void set_max(f32 new_max);
    [[nodiscard]] f32 get_max() const { return data.max_value; }

    void set_value(f32 new_value);
    [[nodiscard]] f32 get_value() const { return data.value; }
};