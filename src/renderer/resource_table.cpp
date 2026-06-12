#include "renderer/resource_table.h"


ResourceTable ResourceTable::create(const ResourceTableCreateInfo& info)
{
    return ResourceTable
    {
        .allocator = info.allocator,
        .device = info.device,
        .descriptor_pool = GPU::descriptor_pool_create(info.device,
            {
                .max_sets = info.max_sets,
                .sizes = info.sizes,
            }
        )
    };
}

void ResourceTable::destroy()
{
    reset();

    GPU::descriptor_pool_destroy(descriptor_pool);
}

void ResourceTable::reset()
{
    GPU::descriptor_pool_reset(descriptor_pool);
}

GPU::DescriptorSetID ResourceTable::allocate(GPU::DescriptorSetLayoutID set_layout)
{
    GPU::DescriptorSetID out_descriptor_sets;
    GPU::descriptor_set_allocate(device,
        {.pool = descriptor_pool, .set_layouts = Slice(&set_layout, 1)},
        Slice(&out_descriptor_sets, 1)
    );

    return out_descriptor_sets;
}

