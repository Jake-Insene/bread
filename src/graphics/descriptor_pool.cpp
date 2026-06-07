#include "graphics/descriptor_pool.h"



namespace Graphics
{

void DescriptorPool::init(Mem::Allocator* _allocator, Device* _parent, const GPU::DescriptorPoolCreateInfo& info)
{
    DeviceObject::init(_allocator, _parent);
    gpu_device = info.device;
    gpu_descriptor_pool = GPU::descriptor_pool_create(info);

    descriptor_sets = FreeList<DescriptorSet, DescriptorSetRef>::with_size(allocator, 4);
    allocated_sets = Array<DescriptorSetRef>::with_size(allocator, 4);
}

void DescriptorPool::destroy()
{
    for(DescriptorSetRef set_ref : allocated_sets.iter())
    {
        DescriptorSet* descriptor_set = set(set_ref);
        descriptor_set->destroy();
    }

    descriptor_sets.destroy();
    allocated_sets.destroy();
    GPU::descriptor_pool_destroy(gpu_descriptor_pool);

    DeviceObject::destroy();
}

DescriptorSetRef DescriptorPool::allocate(GPU::DescriptorSetLayoutID gpu_set_layout)
{
    DescriptorSet descriptor_set = {};
    descriptor_set.init(allocator,
        {
            .gpu_device = gpu_device,
            .gpu_pool = gpu_descriptor_pool,
            .gpu_set_layout = gpu_set_layout
        }
    );

    DescriptorSetRef new_set = descriptor_sets.add(descriptor_set);
    (void)allocated_sets.add(new_set);

    return new_set;
}

void DescriptorPool::free(DescriptorSetRef set_ref)
{
    set(set_ref)->destroy();

    descriptor_sets.remove(set_ref);
}

}
