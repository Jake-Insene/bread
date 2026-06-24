#include "resource/texture.h"

#include "engine/engine.h"
#include "graphics/render_device.h"


void Texture::init(const ResourceCreateInfo& info)
{
    Resource::init(info);
    texture_ref = GPUTextureID::invalid();
    size = Vector2I();
}

void Texture::destroy()
{
    Engine::get_render_device()->get_gpu_resource_manager()->destroy_texture(texture_ref);
    Resource::destroy();
}

Vector2I Texture::get_size() const
{
    return size;
}


