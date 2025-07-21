#include "canvas/progress_bar.h"

#include "graphics/graphics.h"
#include "resource/resource_manager.h"


void ProgressBar::init(const CreateInfo&)
{
    mark(MARK_RENDER);
}

void ProgressBar::render()
{
    if (data.bg_texture == nullptr || data.fill_texture == nullptr)
        return;

    const Vector2 size = get_size();
    // value = max_value -> draw_size = size
    // value = min_value -> draw_size = 0

    // max_value -> size
    // value -> x

    f32 draw_size_x = (size.x * data.value) / data.max_value;
    Vector2 draw_size = Vector2(draw_size_x, size.y);

    // Fill
    Vector2 bg_texture_extent = Vector2(data.fill_texture->get_size());
    Rect2D bg_src_rect = Rect2D(Vector2(0, 0), bg_texture_extent);
    Graphics2D::draw_canvas_element(
        get_global_transform(),
        bg_texture_extent,
        draw_size,
        bg_src_rect, data.fill_texture->texture_id,
        color,
        RenderCommand::FLAG_CANVAS_NONE
    );

    // Background

    Vector2 fill_texture_extent = Vector2(data.bg_texture->get_size());
    Rect2D fill_src_rect = Rect2D(Vector2(0, 0), fill_texture_extent);
    Graphics2D::draw_canvas_element(
        get_global_transform(),
        fill_texture_extent,
        size,
        fill_src_rect, data.bg_texture->texture_id,
        color,
        RenderCommand::FLAG_CANVAS_NONE
    );
}

void ProgressBar::set_background_texture(Texture2D* new_bg_texture)
{
    data.bg_texture = new_bg_texture;
    if (data.bg_texture == nullptr)
        return;
}

void ProgressBar::set_fill_texture(Texture2D* new_fill_texture)
{
    data.fill_texture = new_fill_texture;
    if (data.fill_texture == nullptr)
        return;

}

void ProgressBar::set_min(f32 new_min)
{
    data.min_value = new_min;
}

void ProgressBar::set_max(f32 new_max)
{
    data.max_value = new_max;
}

void ProgressBar::set_value(f32 new_value)
{
    if (new_value < data.min_value)
    {
        data.value = data.min_value;
    }
    else if (new_value > data.max_value)
    {
        data.value = data.max_value;
    }
    else
    {
        data.value = new_value;
    }

}
