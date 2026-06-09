#include "render_device/core/gpu_resource_manager.h"

#include "graphics/queue.h"
#include "graphics/texture.h"
#include "graphics/texture_view.h"
#include "render_device/core/gpu_memory_allocator.h"


void GPUResourceManager::init(const GPUResourceManagerCreateInfo& info)
{
    allocator = info.allocator;
    graphics_device = info.graphics_device;
    gpu_memory_allocator = info.gpu_memory_allocator;

    textures = FreeList<TextureData, GPUTextureID>::with_size(allocator, 4);
}

void GPUResourceManager::destroy()
{
    textures.destroy();
}

GPUTextureID GPUResourceManager::create_texture(const TextureAllocateInfo& alloc_info)
{
    Graphics::Texture* texture = graphics_device->create_texture(
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

    GPUMemoryAllocationID allocation = gpu_memory_allocator->allocate(
        GPUMemoryAllocator::AllocationTag::Texture, GPU::texture_get_memory_requirements(texture->gpu_texture)
    );

    texture->bind_memory(gpu_memory_allocator->allocation_get_heap(allocation),
        gpu_memory_allocator->allocation_get_offset(allocation));

    Graphics::TextureView* texture_view = graphics_device->create_texture_view(
        {
            // TODO: Assumming type
            .type = GPU::TextureViewType::Texture2D,
            .format = alloc_info.format,
            .texture = texture,
            .subresource_range = GPU::TextureSubresourceRange::color(0, 1, 0, 1),
        }
    );

    // Setting up the texture data
    {
        Graphics::Buffer* buffer = gpu_memory_allocator->begin_staging(alloc_info.pixels.len);
        Slice<u8> mapped_buffer = gpu_memory_allocator->map_staging();
        Mem::copy(mapped_buffer, alloc_info.pixels);
        gpu_memory_allocator->unmap_staging(mapped_buffer);

        submit_and_wait(
            graphics_device->get_copy_queue(), [&](GPU::CommandBufferID cmd)
            {
                const GPU::PipelineTextureBarrier begin_barrier =
                {
                    .src_masks = GPU::AccessMasks(),
                    .dest_masks = GPU::AccessMasks::TransferWrite,
                    .src_layout = GPU::TextureLayout::Unknown,
                    .dest_layout = GPU::TextureLayout::TransferDestination,
                    .texture = texture->gpu_texture,
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
                    GPU::CopyBufferToTextureInfo::copy(
                        buffer->gpu_buffer, texture->gpu_texture,
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
                    .texture = texture->gpu_texture,
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

        gpu_memory_allocator->end_staging(buffer);
    }

    GPUTextureID texture_ref = textures.add(TextureData());
    TextureData& texture_data = textures.get(texture_ref);
    texture_data.texture = texture;
    texture_data.texture_view = texture_view;
    texture_data.allocation = allocation;

    return texture_ref;
}

void GPUResourceManager::destroy_texture(GPUTextureID texture_ref)
{
    TextureData& texture_data = textures.get(texture_ref);
    gpu_memory_allocator->free(texture_data.allocation);
    texture_data.texture_view->destroy();
    texture_data.texture->destroy();
    textures.remove(texture_ref);
}

Graphics::Texture* GPUResourceManager::texture_get_texture(GPUTextureID texture_ref)
{
    return textures.get(texture_ref).texture;
}

Graphics::TextureView* GPUResourceManager::texture_get_texture_view(GPUTextureID texture_ref)
{
    return textures.get(texture_ref).texture_view;
}

void GPUResourceManager::_submit_and_wait(Graphics::Queue* queue, void* arg, SubmitFn recorder)
{
    GPU::CommandPoolID pool = GPU::command_pool_create(
        {
            .device = graphics_device->gpu_device,
            .usage = queue->gpu_queue_usage,
        }
    );

    GPU::CommandBufferID cmd = GPU::command_buffer_allocate(
        {
            .pool = pool,
        }
    );

    GPU::command_buffer_begin(cmd);
    recorder(arg, cmd);
    GPU::command_buffer_end(cmd);

    GPU::queue_execute_command_buffer(
        queue->gpu_queue,
        {
            .wait_semaphores = {},
            .wait_stages = {},
            .command_buffers = Slice(&cmd, 1),
            .signal_semaphores = {},
            .fence = GPU::FenceID::invalid(),
        }
    );

    queue->wait_idle();

    GPU::command_buffer_free(cmd);
    GPU::command_pool_destroy(pool);
}
