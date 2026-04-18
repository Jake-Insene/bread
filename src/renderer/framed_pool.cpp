#include "renderer/framed_pool.h"


void FramedPool::init(const FramedPoolCreateInfo& info)
{
    allocator = info.allocator;
    graphics_device = info.graphics_device;

    descriptor_pool = graphics_device->create_descriptor_pool(info.max_sets, info.sizes);
    framed_sets = Array<Graphics::DescriptorSetRef>::with_size(allocator, info.frame_count);

    for(usize i = 0; i < info.frame_count; i++)
    {
        (void)framed_sets.add(descriptor_pool->allocate(info.gpu_set_layout));
    }
}

void FramedPool::destroy()
{
    descriptor_pool->destroy();
    framed_sets.destroy();
}

Graphics::DescriptorSet* FramedPool::get_set(usize frame_index)
{
    return descriptor_pool->set(framed_sets.get(frame_index));
}
