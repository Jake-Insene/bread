#include "render/core/gpu_resource_manager.h"

#include "engine/engine.h"
#include "render/core/gpu_memory_allocator.h"
#include "render/render_device.h"



void GPUResourceManager::initialize(const mem::Allocator& _allocator)
{
    allocator = _allocator;

    render_device = Engine::get_system_manager().get_system<RenderDevice>();
    memory_allocator = &render_device->get_memory_allocator();

    textures = FreeList<GPUTextureResource, GPUTextureID>::with_size(allocator, 4);
}

void GPUResourceManager::shutdown()
{
    textures.destroy();
}

GPUTextureID GPUResourceManager::create_texture(const GPUTextureResourceCreateInfo& ci)
{
    GPU::DeviceID device = render_device->get_graphics_device();

    GPUMemoryAllocationID allocation = memory_allocator->allocate(
        GPUMemoryAllocator::AllocationTag::Texture, ci.pixels.len
    );

    GPU::TextureID gpu_texture = GPU::texture_create(
        {
            .device = device,
            .type = ci.type,
            .format = ci.format,
            .extent = ci.extent,
            .mip_levels = 1,
            .array_levels = 1,
            .sample_count = GPU::SampleCount::Sample1,
            .tiling = GPU::TextureTiling::Optimal,
            .usage = GPU::TextureUsage::TransferDestination | GPU::TextureUsage::Sampled,
            .initial_layout = GPU::TextureLayout::Unknown,
            .memory_heap = memory_allocator->allocation_get_heap(allocation),
            .heap_offset = memory_allocator->allocation_get_offset(allocation),
        }
    );

    // Setting up the texture data
    {
        GPU::BufferID buffer = memory_allocator->begin_staging(ci.pixels.len);
        Slice<u8> mapped_buffer = memory_allocator->map_staging();
        mem::copy(mapped_buffer, ci.pixels);
        memory_allocator->unmap_staging(mapped_buffer);

        render_device->submit_and_wait(
            render_device->copy_queue, [&](GPU::CommandBufferID cmd)
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
                        .source_buffer = buffer,
                        .source_offset = 0,
                        .row_length = 0,
                        .image_height = 0,
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
                        .extent = ci.extent,
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

        memory_allocator->end_staging(buffer);
    }

    GPUTextureID texture_ref = textures.add(GPUTextureResource());
    GPUTextureResource& texture = textures.get(texture_ref);
    texture.gpu_texture = gpu_texture;
    texture.allocation = allocation;

    return texture_ref;
}

void GPUResourceManager::destroy_texture(GPUTextureID texture_ref)
{
    GPUTextureResource& texture = textures.get(texture_ref);
    memory_allocator->free(texture.allocation);
    GPU::texture_destroy(texture.gpu_texture);
}

