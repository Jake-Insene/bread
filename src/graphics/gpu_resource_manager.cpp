#include "graphics/gpu_resource_manager.h"

#include "graphics/gpu_memory_allocator.h"


namespace Graphics
{

void GPUResourceManager::init(const GPUResourceManagerCreateInfo& info)
{
    data.allocator = info.allocator;
    data.device = info.device;
    data.graphics_queue = info.graphics_queue;
    data.copy_queue = info.copy_queue;
    data.gpu_memory_allocator = info.gpu_memory_allocator;

    data.textures = FreeList<TextureData, GPUTextureID>::with_size(data.allocator, 4);
}

void GPUResourceManager::destroy()
{
    data.textures.destroy();
}

GPUTextureID GPUResourceManager::create_texture(const TextureAllocateInfo& alloc_info)
{
    GPU::TextureID texture = GPU::texture_create(
        data.device,
        {
            .type = alloc_info.type,
            .format = alloc_info.format,
            .extent = alloc_info.extent,
            .mip_levels = 1,
            .array_levels = 1,
            .sample_count = GPU::SampleCount::Sample1,
            .tiling = GPU::TextureTiling::Optimal,
            .usage = GPU::TextureUsage::TransferDestination | GPU::TextureUsage::Sampled,
            .initial_layout = GPU::TextureLayout::Unknown,
            .subresource_range =
            {
                .aspect = GPU::TextureAspect::Color,
                .base_mip_level = 0,
                .level_count = 1,
                .base_array_layer = 0,
                .layer_count = 1,
            },
        }
    );

    GPUMemoryAllocationID allocation = data.gpu_memory_allocator->allocate(
        GPUMemoryAllocator::AllocationTag::Texture, GPU::texture_get_memory_requirements(texture)
    );

    GPU::texture_bind_memory_heap(texture,
        GPU::BindMemoryInfo::create(
            data.gpu_memory_allocator->allocation_get_heap(allocation),
            data.gpu_memory_allocator->allocation_get_offset(allocation)
        )
    );

    GPU::TextureViewID texture_view = GPU::texture_view_create(data.device,
        GPU::TextureViewCreateInfo(
        {
            // TODO: Assumming type
            .type = GPU::TextureViewType::Texture2D,
            .format = alloc_info.format,
            .texture = texture,
            .components = HasValue(alloc_info.flags & TextureAllocateFlags::ViewR8One) ?
                GPU::ComponentMapping(GPU::ComponentSwizzle::Red,
                    GPU::ComponentSwizzle::One, GPU::ComponentSwizzle::One, GPU::ComponentSwizzle::One)
                : GPU::ComponentMapping::identity(),
            .subresource_range = GPU::TextureSubresourceRange::color(0, 1, 0, 1),
        })
    );

    // Setting up the texture data
    {
        GPU::BufferID buffer = data.gpu_memory_allocator->begin_staging(alloc_info.pixels.len);
        Slice mapped_buffer = data.gpu_memory_allocator->map_staging(buffer);
        Mem::copy(mapped_buffer, alloc_info.pixels);
        data.gpu_memory_allocator->unmap_staging(buffer, mapped_buffer);

        submit_and_wait(
            data.graphics_queue, GPU::QueueUsage::Graphics, [&](GPU::CommandBufferID cmd)
            {
                const GPU::PipelineTextureBarrier begin_barrier =
                {
                    .src_masks = GPU::AccessMasks(),
                    .dest_masks = GPU::AccessMasks::TransferWrite,
                    .src_layout = GPU::TextureLayout::Unknown,
                    .dest_layout = GPU::TextureLayout::TransferDestination,
                    .texture = texture,
                    .subresource_range = GPU::TextureSubresourceRange::color(0, 1, 0, 1),
                };
                GPU::command_buffer_pipeline_barrier(cmd,
                    GPU::PipelineBarrier::texture_barrier(
                        GPU::PipelineStages::Begin, GPU::PipelineStages::Transfer,
                        Slice(&begin_barrier, 1)
                    )
                );

                GPU::BufferTextureCopyRegion region = GPU::BufferTextureCopyRegion::region(
                    0, GPU::TextureSubresourceLayers::color(0, 0, 1),
                    Vector3I(), alloc_info.extent
                );
                GPU::command_buffer_copy_buffer_to_texture(cmd,
                    GPU::CopyBufferToTextureInfo::create(
                        buffer, texture,
                        GPU::TextureLayout::TransferDestination,
                        Slice(&region, 1)
                    )
                );

                const GPU::PipelineTextureBarrier end_barrier =
                {
                    .src_masks = GPU::AccessMasks::TransferWrite,
                    .dest_masks = GPU::AccessMasks::ShaderRead,
                    .src_layout = GPU::TextureLayout::TransferDestination,
                    .dest_layout = GPU::TextureLayout::ShaderReadOnly,
                    .texture = texture,
                    .subresource_range = GPU::TextureSubresourceRange::color(0, 1, 0, 1),
                };
                GPU::command_buffer_pipeline_barrier(cmd,
                    GPU::PipelineBarrier::texture_barrier(
                        GPU::PipelineStages::Transfer, GPU::PipelineStages::FragmentShader,
                        Slice(&end_barrier, 1)
                    )
                );
            }
        );

        data.gpu_memory_allocator->end_staging(buffer);
    }

    GPUTextureID texture_ref = data.textures.add(TextureData());
    TextureData& texture_data = data.textures.get(texture_ref);
    texture_data.texture = texture;
    texture_data.texture_view = texture_view;
    texture_data.allocation = allocation;

    return texture_ref;
}

void GPUResourceManager::destroy_texture(GPUTextureID texture_ref)
{
    TextureData& texture_data = data.textures.get(texture_ref);
    data.gpu_memory_allocator->free(texture_data.allocation);

    GPU::texture_view_destroy(texture_data.texture_view);
    GPU::texture_destroy(texture_data.texture);
    data.textures.remove(texture_ref);
}

GPU::TextureID GPUResourceManager::texture_get_texture(GPUTextureID texture_ref)
{
    return data.textures.get(texture_ref).texture;
}

GPU::TextureViewID GPUResourceManager::texture_get_texture_view(GPUTextureID texture_ref)
{
    return data.textures.get(texture_ref).texture_view;
}

void GPUResourceManager::_submit_and_wait(GPU::QueueID queue, GPU::QueueUsage usage, void* arg, SubmitFn recorder)
{
    GPU::CommandPoolID pool = GPU::command_pool_create(
        data.device,
        {
            .usage = usage,
        }
    );

    GPU::CommandBufferID cmd = GPU::command_buffer_allocate(
        data.device,
        {
            .pool = pool,
        }
    );

    GPU::command_buffer_begin(cmd);
    recorder(arg, cmd);
    GPU::command_buffer_end(cmd);

    GPU::queue_execute_command_buffer(
        queue,
        {
            .wait_semaphores = {},
            .wait_stages = {},
            .command_buffers = Slice(&cmd, 1),
            .signal_semaphores = {},
            .fence = GPU::FenceID::invalid(),
        }
    );

    GPU::queue_wait_idle(queue);

    GPU::command_buffer_free(cmd);
    GPU::command_pool_destroy(pool);
}

}
