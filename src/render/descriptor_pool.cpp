#include "render/descriptor_pool.h"

#include "engine/engine.h"
#include "render/render_device.h"


DescriptorPool DescriptorPool::create(u32 max_sets, Slice<const GPU::DescriptorPoolSize> sizes)
{
    DescriptorPool pool = {};

    RenderDevice* render_device = Engine::get_system_manager()->get_system<RenderDevice>();

    pool.init(render_device->allocator,
        {
            .device = render_device->get_graphics_device(),
            .max_sets = max_sets,
            .sizes = sizes,
        }
    );
    return pool;
}

void DescriptorPool::init(const mem::Allocator& _allocator, const GPU::DescriptorPoolCreateInfo& info)
{
    allocator = _allocator;
    
    device = info.device;
    descriptor_pool = GPU::descriptor_pool_create(info);

    allocated_sets = Array<DescriptorSet>::with_size(allocator, 4);
}

void DescriptorPool::destroy()
{
    for(DescriptorSet& set : allocated_sets.iter())
    {
        set.destroy();
    }

    allocated_sets.destroy();
    GPU::descriptor_pool_destroy(descriptor_pool);
}

DescriptorSet DescriptorPool::allocate(GPU::DescriptorSetLayoutID set_layout)
{
    DescriptorSet descriptor_set = {};
    descriptor_set.init(allocator,
        {
            .device = device,
            .pool = descriptor_pool,
            .set_layout = set_layout
        }
    );

    (void)allocated_sets.add(descriptor_set);
    return descriptor_set;
}

