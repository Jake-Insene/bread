#include "render/descriptor_set.h"



void DescriptorSet::init(const mem::Allocator& _allocator, const DescriptorSetInfo& info)
{
    allocator = _allocator;
    
    descriptor_set = GPU::descriptor_set_allocate(
        {
            .device = info.device,
            .pool = info.pool,
            .set_layout = info.set_layout
        }
    );
}

void DescriptorSet::destroy()
{
    GPU::descriptor_set_free(descriptor_set);
}

void DescriptorSet::set_uniform_buffer(u32 binding, const Buffer& buffer, usize offset, usize range)
{
    GPU::DescriptorBufferInfo buffers[] =
    {
        { .buffer = buffer.buffer, .offset = offset, .range = range, },
    };

    GPU::WriteDescriptorInfo write_info[] =
    {
        { .binding = binding, .array_element = 0, .count = 1, .type = GPU::DescriptorType::UniformBuffer, .textures = {}, .buffers = buffers, },
    };

    GPU::descriptor_set_update_descriptors(descriptor_set,
        {
            .write_infos = write_info,
        }
    );
}

void DescriptorSet::set_combined_texture_sampler(u32 binding, GPU::TextureID texture, GPU::TextureLayout layout, Sampler& sampler)
{
    GPU::DescriptorTextureInfo textures[] =
    {
        { .texture = texture, .layout = layout, .sampler = sampler.sampler, },
    };

    GPU::WriteDescriptorInfo write_info[] =
    {
        { .binding = binding, .array_element = 0, .count = 1, .type = GPU::DescriptorType::CombinedTextureSampler, .textures = textures, .buffers = {}, },
    };

    GPU::descriptor_set_update_descriptors(descriptor_set,
        {
            .write_infos = write_info,
        }
    );   
}
