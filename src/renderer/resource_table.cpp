#include "renderer/resource_table.h"

#include "os/os.h"


ResourceTable ResourceTable::create(const ResourceTableCreateInfo& info)
{
    Slice set_layouts = info.allocator->array<GPU::DescriptorSetLayoutID>(info.set_layouts.len);
    Mem::copy(set_layouts, info.set_layouts);

    Mem::StackAllocator tmp_allocator = {};
    tmp_allocator.init(OS::map_memory(1024*4, OS::MapAccess::ReadWrite));
    
    return ResourceTable
    {
        .allocator = info.allocator,
        .device = info.device,
        .set_layouts = set_layouts,
        .sets = info.allocator->array<GPU::DescriptorSetID>(info.set_layouts.len),
        .tmp_allocator = tmp_allocator,
        .gpu_write_infos = Array<GPU::WriteDescriptorInfo>::with_size(info.allocator, 4),
    };
}

void ResourceTable::destroy()
{
    allocator->free(Mem::to_bytes(set_layouts));
    allocator->free(Mem::to_bytes(sets));
    OS::unmap_memory(tmp_allocator.sp);
    gpu_write_infos.destroy();
}

void ResourceTable::begin(GPU::DescriptorPoolID pool)
{
    GPU::descriptor_set_allocate(device, GPU::DescriptorSetAllocateInfo::create(pool, set_layouts), sets);
    tmp_allocator.reset();
    gpu_write_infos.clear();
}

void ResourceTable::bind_combined_texture_sampler(u32 set, u32 binding, const GPU::DescriptorTextureInfo& texture)
{
    Slice textures = tmp_allocator.array<GPU::DescriptorTextureInfo>(1);
    textures[0] = texture;
    (void)gpu_write_infos.add(
        {
            .descriptor_set = sets[set],
            .binding = binding,
            .array_element = 0,
            .type = GPU::DescriptorType::CombinedTextureSampler,
            .buffers = {},
            .textures = textures,
        }
    );
}

void ResourceTable::end(GPU::DescriptorPoolID)
{
    GPU::descriptor_set_update_descriptors(device, {.write_infos = gpu_write_infos.slice()});
}
