#include "graphics/texture_view.h"

#include "graphics/device.h"
#include "graphics/texture.h"


namespace Graphics
{

void TextureView::init(Mem::Allocator* _allocator, Device* _parent, const TextureViewInfo& info)
{
    DeviceObject::init(_allocator, _parent);

    gpu_texture_view = GPU::texture_view_create(
        {
            .device = parent->gpu_device,
            .type = info.type,
            .format = info.format,
            .texture =info.texture->gpu_texture,
            .subresource_range = info.subresource_range
        }
    );
}

void TextureView::destroy()
{
    GPU::texture_view_destroy(gpu_texture_view);

    DeviceObject::destroy();
}

}
