#include "renderer/framed_pool.h"


void FramedPool::init(const FramedPoolCreateInfo& info)
{
    data.allocator = info.allocator;
    data.device = info.device;

    data.pools = Array<GPU::DescriptorPoolID>::with_size(data.allocator, info.frame_count);
    
    for(usize i = 0; i < info.frame_count; i++)
    {
        (void)data.pools.add(GPU::descriptor_pool_create(data.device,
            GPU::DescriptorPoolCreateInfo(info.max_sets, info.sizes))
        );
    }
}

void FramedPool::destroy()
{
    (void)data.pools.iter().for_each([](GPU::DescriptorPoolID pool)
    {
        GPU::descriptor_pool_reset(pool);
        GPU::descriptor_pool_destroy(pool);
    });
    data.pools.destroy();
}

void FramedPool::reset_pool(usize frame_index)
{
    GPU::descriptor_pool_reset(data.pools.get(frame_index));
}

GPU::DescriptorPoolID FramedPool::get_pool(usize frame_index)
{
    return data.pools.get(frame_index);
}
