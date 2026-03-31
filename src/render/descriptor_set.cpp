#include "render/descriptor_set.h"



void DescriptorSet::init(const mem::Allocator& _allocator, const DescriptorSetInfo& info)
{
    allocator = _allocator;
    
    deferred_buffers = 0;
    deferred_textures = 0;
    deferred_writes = Array<DeferredWrite>::with_size(allocator, 4);
    use_deferred = false;
    descriptor_set = GPU::descriptor_set_allocate(
        {
            .device = info.device,
            .pool = info.pool,
            .set_layout = info.set_layout
        }
    );
    set_layout = info.set_layout;
}

void DescriptorSet::destroy()
{
    deferred_writes.destroy();
    GPU::descriptor_set_free(descriptor_set);
}

void DescriptorSet::set_uniform_buffer(u32 binding, const Buffer& buffer, usize offset, usize range)
{
    if(use_deferred)
    {
        deferred_buffers++;
        (void)deferred_writes.add(
            {
                .binding = binding,
                .type = GPU::DescriptorType::UniformBuffer,
                .write =
                {
                    .buffer =
                    {
                        .buffer = buffer.buffer,
                        .offset = offset,
                        .range = range,
                    }
                },
            }
        );
    }
    else
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
}

void DescriptorSet::set_combined_texture_sampler(u32 binding, GPU::TextureID texture, GPU::TextureLayout layout, Sampler& sampler)
{
    if(use_deferred)
    {
        deferred_textures++;
        (void)deferred_writes.add(
            {
                .binding = binding,
                .type = GPU::DescriptorType::CombinedTextureSampler,
                .write =
                {
                    .texture =
                    {
                        .texture = texture,
                        .layout = layout,
                        .sampler = sampler.sampler,
                    }
                },
            }
        );
    }
    else
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
}

void DescriptorSet::sync_writes()
{
    if(use_deferred && deferred_writes.count)
    {
        Slice<GPU::WriteDescriptorInfo> writes = allocator.array<GPU::WriteDescriptorInfo>(deferred_writes.count);
        Slice<GPU::DescriptorBufferInfo> buffers = allocator.array<GPU::DescriptorBufferInfo>(deferred_buffers);
        Slice<GPU::DescriptorTextureInfo> textures = allocator.array<GPU::DescriptorTextureInfo>(deferred_textures);

        usize buffer_index = 0;
        usize texture_index = 0;
        for(usize i = 0; i < writes.len; i++)
        {
            const DeferredWrite& deferred_write = deferred_writes.get(i);
            writes[i].binding = deferred_write.binding;
            writes[i].array_element = 0;
            writes[i].count = 1;
            writes[i].type = deferred_write.type;
            writes[i].buffers = {};
            writes[i].textures = {};

            if(IsAnyEqual(deferred_write.type, GPU::DescriptorType::UniformBuffer, GPU::DescriptorType::StorageBuffer))
            {
                buffers[buffer_index] =
                {
                    .buffer = deferred_write.write.buffer.buffer,
                    .offset = deferred_write.write.buffer.offset,
                    .range = deferred_write.write.buffer.range,
                };
                writes[i].buffers = Slice(&buffers[buffer_index], 1);
                buffer_index++;
            }
            else if(IsAnyEqual(deferred_write.type, GPU::DescriptorType::CombinedTextureSampler))
            {
                textures[texture_index] =
                {
                    .texture = deferred_write.write.texture.texture,
                    .layout = deferred_write.write.texture.layout,
                    .sampler = deferred_write.write.texture.sampler,
                };
                writes[i].textures = Slice(&textures[texture_index], 1);
                texture_index++;
            }
        }

        GPU::descriptor_set_update_descriptors(descriptor_set,
            {
                .write_infos = writes,
            }
        );
        allocator.free(mem::to_bytes(writes));
        allocator.free(mem::to_bytes(buffers));
        allocator.free(mem::to_bytes(textures));
        deferred_writes.clear();

        deferred_buffers = 0;
        deferred_textures = 0;
    }
}
