#include "render_device/core/gpu_resource_manager.h"

#include "render_device/core/gpu_memory_allocator.h"
#include "render_device/render_device.h"


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
    GPU::TextureID gpu_texture = GPU::texture_create(
        {
            .device = graphics_device->gpu_device,
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
        GPUMemoryAllocator::AllocationTag::Texture, GPU::texture_get_memory_requirements(gpu_texture)
    );

    GPU::texture_bind_memory_heap(gpu_texture,
        {
            .memory_heap = gpu_memory_allocator->allocation_get_heap(allocation)->gpu_memory_heap,
            .heap_offset = gpu_memory_allocator->allocation_get_offset(allocation),
        }
    );

    GPU::TextureViewID gpu_texture_view = GPU::texture_view_create(
        {
            .device = graphics_device->gpu_device,
            // TODO: Assumming type
            .type = GPU::TextureViewType::Texture2D,
            .format = alloc_info.format,
            .texture = gpu_texture,
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

    // Setting up the texture data
    {
        Graphics::Buffer* buffer = gpu_memory_allocator->begin_staging(alloc_info.pixels.len);
        Slice<u8> mapped_buffer = gpu_memory_allocator->map_staging();
        Mem::copy(mapped_buffer, alloc_info.pixels);
        gpu_memory_allocator->unmap_staging(mapped_buffer);

        submit_and_wait(
            graphics_device->copy_queue.gpu_queue, [&](GPU::CommandBufferID cmd)
            {
                GPU::command_buffer_texture_barrier(cmd,
                    {
                        .src_stages = GPU::PipelineStages::Begin,
                        .dest_stages = GPU::PipelineStages::Transfer,
                        .src_masks = GPU::AccessMasks(),
                        .dest_masks = GPU::AccessMasks::TransferWrite,
                        .src_layout = GPU::TextureLayout::Unknown,
                        .dest_layout = GPU::TextureLayout::TransferDestination,
                        .texture = gpu_texture,
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
                GPU::command_buffer_copy_buffer_to_texture(cmd,
                    {
                        .source_buffer = buffer->gpu_buffer,
                        .source_offset = 0,
                        .row_length = 0,
                        .texture_height = 0,
                        .destination_texture = gpu_texture,
                        .destination_layout = GPU::TextureLayout::TransferDestination,
                        .subresource_layer =
                        {
                            .aspect = GPU::TextureAspect::Color,
                            .mip_level = 0,
                            .base_array_layer = 0,
                            .layer_count = 1,
                        },
                        .offset = Vector3I(),
                        .extent = alloc_info.extent,
                    }
                );
                GPU::command_buffer_texture_barrier(cmd,
                    {
                        .src_stages = GPU::PipelineStages::Transfer,
                        .dest_stages = GPU::PipelineStages::FragmentShader,
                        .src_masks = GPU::AccessMasks::TransferWrite,
                        .dest_masks = GPU::AccessMasks::ShaderRead,
                        .src_layout = GPU::TextureLayout::TransferDestination,
                        .dest_layout = GPU::TextureLayout::ShaderReadOnly,
                        .texture = gpu_texture,
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
            }
        );

        gpu_memory_allocator->end_staging(buffer);
    }

    GPUTextureID texture_ref = textures.add(TextureData());
    TextureData& texture = textures.get(texture_ref);
    texture.gpu_texture = gpu_texture;
    texture.gpu_texture_view = gpu_texture_view;
    texture.allocation = allocation;

    return texture_ref;
}

void GPUResourceManager::destroy_texture(GPUTextureID texture_ref)
{
    TextureData& texture = textures.get(texture_ref);
    gpu_memory_allocator->free(texture.allocation);
    GPU::texture_view_destroy(texture.gpu_texture_view);
    GPU::texture_destroy(texture.gpu_texture);
}

GPU::TextureID GPUResourceManager::texture_get_gpu_texture(GPUTextureID texture_ref)
{
    return textures.get(texture_ref).gpu_texture;
}

GPU::TextureViewID GPUResourceManager::texture_get_gpu_texture_view(GPUTextureID texture_ref)
{
    return textures.get(texture_ref).gpu_texture_view;
}

void GPUResourceManager::_submit_and_wait(GPU::QueueID gpu_queue, void* arg, SubmitFn recorder)
{
    GPU::CommandPoolID pool = GPU::command_pool_create(
        {
            .device = graphics_device->gpu_device,
            .queue = gpu_queue,
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
        gpu_queue,
        {
            .wait_semaphores = {},
            .wait_stages = {},
            .command_buffers = Slice(&cmd, 1),
            .signal_semaphores = {},
            .fence = GPU::FenceID::invalid(),
        }
    );

    GPU::queue_wait_idle(gpu_queue);

    GPU::command_buffer_free(cmd);
    GPU::command_pool_destroy(pool);
}
