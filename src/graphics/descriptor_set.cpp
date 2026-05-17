#include "graphics/descriptor_set.h"



namespace Graphics
{

void DescriptorSet::init(mem::Allocator* _allocator, const DescriptorSetInfo& info)
{
    allocator = _allocator;
    
    deferred_buffers = 0;
    deferred_textures = 0;
    deferred_writes = Array<DeferredWrite>::with_size(allocator, 4);
    use_deferred = false;
    descriptor_set = GPU::descriptor_set_allocate(
        {
            .device = info.gpu_device,
            .pool = info.gpu_pool,
            .set_layout = info.gpu_set_layout
        }
    );
    set_layout = info.gpu_set_layout;
}

void DescriptorSet::destroy()
{
    deferred_writes.destroy();
    GPU::descriptor_set_free(descriptor_set);
}

void DescriptorSet::set_uniform_buffer(u32 binding, const Buffer* buffer, usize offset, usize range)
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
                        .buffer = buffer->gpu_buffer,
                        .offset = offset,
                        .range = range,
                    }
                },
                .write_array = {},
            }
        );
    }
    else
    {
        GPU::DescriptorBufferInfo buffers[] =
        {
            { .buffer = buffer->gpu_buffer, .offset = offset, .range = range, },
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

void DescriptorSet::set_combined_texture_sampler(u32 binding, GPU::TextureID texture, GPU::TextureLayout layout, Sampler* sampler)
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
                        .sampler = sampler->gpu_sampler,
                    }
                },
                .write_array = {},
            }
        );
    }
    else
    {
        GPU::DescriptorTextureInfo textures[] =
        {
            { .texture = texture, .layout = layout, .sampler = sampler->gpu_sampler, },
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

void DescriptorSet::set_combined_texture_sampler_array(u32 binding, Slice<GPU::TextureID> textures, GPU::TextureLayout layout, Slice<Graphics::Sampler*> samplers)
{
    if(use_deferred)
    {
        Slice<GPU::DescriptorTextureInfo> write_textures = allocator->array<GPU::DescriptorTextureInfo>(textures.len);
        for(usize i = 0; i < textures.len; i++)
        {
            write_textures[i].texture = textures[i];
            write_textures[i].layout = layout;
            write_textures[i].sampler = samplers[i]->gpu_sampler;
        }

        deferred_textures++;
        (void)deferred_writes.add(
            {
                .binding = binding,
                .type = GPU::DescriptorType::CombinedTextureSampler,
                .write = {},
                .write_array =
                {
                    .buffers = {},
                    .textures = write_textures,
                },
            }
        );
    }
    else
    {
        GPU::DescriptorTextureInfo* texture_infos = allocator->array<GPU::DescriptorTextureInfo>(textures.len).ptr();
        for(usize i = 0; i < textures.len; i++)
        {
            texture_infos[i] = { .texture = textures[i], .layout = layout, .sampler = samplers[i]->gpu_sampler };
        }
        
        GPU::WriteDescriptorInfo write_info[] =
        {
            { .binding = binding, .array_element = 0, .count = u32(textures.len), .type = GPU::DescriptorType::CombinedTextureSampler, .textures = Slice(texture_infos, textures.len), .buffers = {}, },
        };
        
        GPU::descriptor_set_update_descriptors(descriptor_set,
            {
                .write_infos = write_info,
            }
        );
        
        allocator->free(mem::to_bytes(Slice(texture_infos, textures.len)));
    }
}

void DescriptorSet::sync_writes()
{
    if(use_deferred && deferred_writes.count != 0)
    {
        Slice<GPU::WriteDescriptorInfo> writes = allocator->array<GPU::WriteDescriptorInfo>(deferred_writes.count);
        Slice<GPU::DescriptorBufferInfo> buffers = allocator->array<GPU::DescriptorBufferInfo>(deferred_buffers);
        Slice<GPU::DescriptorTextureInfo> textures = allocator->array<GPU::DescriptorTextureInfo>(deferred_textures);

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
                if(deferred_write.write_array.buffers.len != 0)
                {
                    writes[i].count = deferred_write.write_array.buffers.len;
                    writes[i].buffers = deferred_write.write_array.buffers;
                }
                else
                {
                    buffers[buffer_index] = deferred_write.write.buffer;
                    writes[i].buffers = Slice(&buffers[buffer_index], 1);
                }
                buffer_index++;
            }
            else if(IsAnyEqual(deferred_write.type, GPU::DescriptorType::CombinedTextureSampler))
            {
                if(deferred_write.write_array.textures.len != 0)
                {
                    writes[i].count = deferred_write.write_array.textures.len;
                    writes[i].textures = deferred_write.write_array.textures;
                }
                else {
                    textures[texture_index] = deferred_write.write.texture;
                    writes[i].textures = Slice(&textures[texture_index], 1);
                }
                texture_index++;
            }
        }

        GPU::descriptor_set_update_descriptors(descriptor_set,
            {
                .write_infos = writes,
            }
        );
        allocator->free(mem::to_bytes(writes));
        allocator->free(mem::to_bytes(buffers));
        allocator->free(mem::to_bytes(textures));

        for(const DeferredWrite& deferred_write : deferred_writes.iter())
        {
            if(deferred_write.write_array.buffers.len != 0)
            {
                allocator->free(mem::to_bytes(deferred_write.write_array.buffers));
            }
            else if(deferred_write.write_array.textures.len != 0)
            {
                allocator->free(mem::to_bytes(deferred_write.write_array.textures));
            }
        }
        deferred_writes.clear();

        deferred_buffers = 0;
        deferred_textures = 0;
    }
}

}
