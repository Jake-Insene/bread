#include "graphics/descriptor_pool.h"

#include "engine/engine.h"
#include "render/render_device.h"


namespace Graphics
{

void DescriptorPool::init(const mem::Allocator& _allocator, const GPU::DescriptorPoolCreateInfo& info)
{
    allocator = _allocator;
    
    device = info.device;
    descriptor_pool = GPU::descriptor_pool_create(info);

    descriptor_sets = FreeList<DescriptorSet, DescriptorSetRef>::with_size(allocator, 4);
    available_sets = Array<DescriptorSetRef>::with_size(allocator, 4);
    allocated_sets = Array<DescriptorSetRef>::with_size(allocator, 4);
}

void DescriptorPool::destroy()
{
    for(DescriptorSetRef set_ref : allocated_sets.iter())
    {
        DescriptorSet& descriptor_set = set(set_ref);
        descriptor_set.destroy();
    }

    descriptor_sets.destroy();
    available_sets.destroy();
    allocated_sets.destroy();
    GPU::descriptor_pool_destroy(descriptor_pool);
}

DescriptorSetRef DescriptorPool::allocate(GPU::DescriptorSetLayoutID set_layout)
{
    for (usize i = 0; i < available_sets.count; ++i)
    {
        DescriptorSetRef avail_set = available_sets.get(i);
        DescriptorSet& descriptor_set = set(avail_set);
        if (descriptor_set.set_layout == set_layout)
        {
            available_sets.remove_at(i);
            return avail_set;
        }
    }

    DescriptorSet descriptor_set = {};
    descriptor_set.init(allocator,
        {
            .device = device,
            .pool = descriptor_pool,
            .set_layout = set_layout
        }
    );

    DescriptorSetRef new_set = descriptor_sets.add(descriptor_set);
    (void)allocated_sets.add(new_set);

    return new_set;
}

void DescriptorPool::free(DescriptorSetRef set_ref)
{

    bool is_allocated = false;
    for (DescriptorSetRef ref : allocated_sets.iter())
    {
        if (ref == set_ref)
        {
            is_allocated = true;
            break;
        }
    }
    if (!is_allocated)
        return;
    
    (void)available_sets.add(set_ref);
}

}
