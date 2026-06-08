#include "graphics/descriptor_pool.h"

#include "graphics/device.h"


namespace Graphics
{

void DescriptorPool::init(Mem::Allocator* _allocator, Device* _parent, const GPU::DescriptorPoolCreateInfo& info)
{
    DeviceObject::init(_allocator, _parent);
    gpu_device = info.device;
    gpu_descriptor_pool = GPU::descriptor_pool_create(info);

    allocated_sets = Array<DescriptorSet*>::with_size(allocator, 4);
}

void DescriptorPool::destroy()
{
    for(DescriptorSet* descriptor_set : allocated_sets.iter())
    {
        descriptor_set->destroy();
    }

    allocated_sets.destroy();
    GPU::descriptor_pool_destroy(gpu_descriptor_pool);

    DeviceObject::destroy();
}

DescriptorSet* DescriptorPool::allocate(GPU::DescriptorSetLayoutID gpu_descriptor_set_layout)
{
    DescriptorSet* descriptor_set = parent->_allocate_object<DescriptorSet>();
    descriptor_set->init(allocator, parent,
        {
            .gpu_device = gpu_device,
            .gpu_descriptor_pool = gpu_descriptor_pool,
            .gpu_descriptor_set_layout = gpu_descriptor_set_layout
        }
    );

    (void)allocated_sets.add(descriptor_set);
    return descriptor_set;
}

void DescriptorPool::free(DescriptorSet* descriptor_set)
{
    descriptor_set->destroy();
    allocated_sets.remove(descriptor_set);
}

}
