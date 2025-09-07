#pragma once
#include "core/header.h"
#include "mem/allocator.h"
#include "graphics/structs.h"
#include "graphics/graphics_types.h"
#include "math/vec2.h"


struct Image;
struct Viewport;


struct Graphics
{
    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static void recreate();
    static void destroy();

    static void render(Viewport* viewport);
    static void present(Viewport* viewport);

    static TextureID create_texture(const TextureCreateInfo& create_info);
    static void destroy_texture(TextureID tex_id);
    static RenderTargetID create_render_target(const RenderTargetCreateInfo& create_info);
    static void destroy_render_target(RenderTargetID rt_id);

    static RenderTargetID get_main_render_target();

    static void texture_set_image(TextureID tex_id, Image* img);
    static Vector2I texture_get_size(TextureID tex_id);

    static void render_target_set_size(RenderTargetID rt_id, const Vector2I& new_size);
    static Vector2I render_target_get_size(RenderTargetID rt_id);
};

