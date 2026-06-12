#include "renderer/framed_pool.h"


void FramedPool::init(const FramedPoolCreateInfo& info)
{
    allocator = info.allocator;
    device = info.device;

    pools = Array<GPU::DescriptorPoolID>::with_size(allocator, info.frame_count);
    
    for(usize i = 0; i < info.frame_count; i++)
    {
        (void)pools.add(GPU::descriptor_pool_create(device,
            GPU::DescriptorPoolCreateInfo(info.max_sets, info.sizes))
        );
    }
}

void FramedPool::destroy()
{
    (void)pools.iter().for_each([](GPU::DescriptorPoolID pool)
    {
        GPU::descriptor_pool_reset(pool);
        GPU::descriptor_pool_destroy(pool);
    });
    pools.destroy();
}

void FramedPool::reset_pool(usize frame_index)
{
    GPU::descriptor_pool_reset(pools.get(frame_index));
}

GPU::DescriptorPoolID FramedPool::get_pool(usize frame_index)
{
    return pools.get(frame_index);
}
