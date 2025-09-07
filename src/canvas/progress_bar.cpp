#include "canvas/progress_bar.h"

#include "graphics/graphics.h"
#include "resource/resource_manager.h"


void ProgressBar::init(const CreateInfo&)
{
    mark(MARK_RENDER);
    mark(MARK_INTERNAL_UPDATE);
}

void ProgressBar::internal_update(f32 dt)
{
    data.value = math::move_to(data.value, data.current_value, update_speed * dt);
}

void ProgressBar::render()
{
    const Vector2 size = get_size();
    // value = max_value -> draw_size = size
    // value = min_value -> draw_size = 0

    // max_value -> size
    // value -> x

    f32 draw_size_x = (size.x * data.value) / data.max_value;
    Vector2 draw_size = Vector2(draw_size_x, size.y);

    // Fill
    if (data.fill_texture == nullptr)
        return;

    Vector2 fill_texture_extent = Vector2(data.fill_texture->get_size());
    Rect2D fill_src_rect = Rect2D(Vector2(0, 0), fill_texture_extent);
    Graphics::draw_canvas_element(
        get_global_transform(),
        draw_size,
        fill_src_rect, data.fill_texture->texture_id,
        fill_color,
        RenderCommand::FLAG_BATCH_NONE
    );

    // Background
    if (data.bg_texture == nullptr)
        return;

    Vector2 bg_texture_extent = Vector2(data.bg_texture->get_size());
    Rect2D bg_src_rect = Rect2D(Vector2(0, 0), bg_texture_extent);
    Graphics::draw_canvas_element(
        get_global_transform(),
        size,
        bg_src_rect, data.bg_texture->texture_id,
        bg_color,
        RenderCommand::FLAG_BATCH_NONE
    );
}

void ProgressBar::set_background_texture(Texture2D* new_bg_texture)
{
    data.bg_texture = new_bg_texture;
}

void ProgressBar::set_fill_texture(Texture2D* new_fill_texture)
{
    data.fill_texture = new_fill_texture;
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
        data.current_value = data.min_value;
    }
    else if (new_value > data.max_value)
    {
        data.current_value = data.max_value;
    }
    else
    {
        data.current_value = new_value;
    }

}

void ProgressBar::set_value_immediate(f32 new_value)
{
    data.value = new_value;
    data.current_value = new_value;
}
