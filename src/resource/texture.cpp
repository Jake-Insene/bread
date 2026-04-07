#include "resource/texture.h"

#include "engine/engine.h"
#include "render_device/render_device.h"


void Texture::init(const ResourceCreateInfo& info)
{
    Resource::init(info);
    texture_ref = GPUTextureID::invalid();
    size = Vector2I();
}

void Texture::destroy()
{
    Engine::get_system_manager()->get_system<RenderDevice>()->get_resource_manager().destroy_texture(texture_ref);
    Resource::destroy();
}

Vector2I Texture::get_size() const
{
    return size;
}


