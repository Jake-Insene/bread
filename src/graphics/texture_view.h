#pragma once
#include "graphics/device_object.h"
#include "graphics/structures.h"


namespace Graphics
{

struct TextureView : DeviceObject
{
    GPU::TextureViewID gpu_texture_view;

    void init(Mem::Allocator* _allocator, Device* _parent, const TextureViewInfo& info);
    void destroy();
};

}