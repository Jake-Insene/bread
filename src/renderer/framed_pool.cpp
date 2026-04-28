#include "renderer/framed_pool.h"


void FramedPool::init(const FramedPoolCreateInfo& info)
{
    allocator = info.allocator;
    graphics_device = info.graphics_device;

    pools = Array<Graphics::DescriptorPool*>::with_size(allocator, info.frame_count);
    
    for(usize i = 0; i < info.frame_count; i++)
    {
        (void)pools.add(graphics_device->create_descriptor_pool(info.max_sets, info.sizes));
    }
}

void FramedPool::destroy()
{
    (void)pools.iter().for_each([](Graphics::DescriptorPool* pool) { pool->destroy(); });
    pools.destroy();
}

Graphics::DescriptorPool* FramedPool::get_pool(usize frame_index)
{
    return pools.get(frame_index);
}
