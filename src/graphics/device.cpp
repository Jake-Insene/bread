#include "graphics/device.h"

#include "debug/fail.h"
#include "graphics/buffer.h"
#include "graphics/command_pool.h"
#include "graphics/descriptor_pool.h"
#include "graphics/fence.h"
#include "graphics/memory_heap.h"
#include "graphics/pipeline_layout.h"
#include "graphics/pipeline.h"
#include "graphics/sampler.h"
#include "graphics/semaphore.h"
#include "graphics/swap_chain.h"
#include "graphics/texture.h"
#include "graphics/texture_view.h"
#include "log/log.h"


namespace Graphics
{

void Device::init(Mem::Allocator* _allocator, GPU::PhysicalDeviceID _gpu_physical_device)
{
    allocator = _allocator;
    ConstructObject(tmp_allocator);
    tmp_allocator.init(OS::map_memory(DefaultStackSize, OS::MapAccess::ReadWrite));

    mutex = Mutex::create();
    mutex.lock();

    gpu_physical_device = _gpu_physical_device;

    gpu_device = GPU::device_create(
        {
            .physical_device = gpu_physical_device,
        }
    );

    // Garanted
    graphics_queue.init(
        allocator,
        gpu_device,
        GPU::QueueUsage::Graphics,
        0
    );

    if(GPU::queue_get_count({.device = gpu_device, .usage = GPU::QueueUsage::Compute}) > 0)
    {
        compute_queue.init(
            allocator,
            gpu_device,
            GPU::QueueUsage::Graphics,
            0
        );
    }
    else
    {
        compute_queue = graphics_queue;
    }

    if(GPU::queue_get_count({.device = gpu_device, .usage = GPU::QueueUsage::Copy}) > 0)
    {
        copy_queue.init(
            allocator,
            gpu_device,
            GPU::QueueUsage::Copy,
            0
        );
    }
    else
    {
        copy_queue = compute_queue;
    }

    if(GPU::queue_get_count({.device = gpu_device, .usage = GPU::QueueUsage::Present}) > 0)
    {
        present_queue.init(
            allocator,
            gpu_device,
            GPU::QueueUsage::Present,
            0
        );
    }
    else
    {
        present_queue = graphics_queue;
    }

    allocated_objects = Array<DeviceObject*>::with_size(allocator, 4);

    mutex.unlock();
}

void Device::destroy()
{
    OS::unmap_memory(tmp_allocator.sp);

    mutex.lock();

    graphics_queue.wait_idle();
    compute_queue.wait_idle();
    copy_queue.wait_idle();
    present_queue.wait_idle();

    for(DeviceObject*& allocated_object : allocated_objects.iter())
    {
        allocator->free(Slice(reinterpret_cast<u8*>(allocated_object), 1));
    }
    allocated_objects.destroy();

    graphics_queue.destroy();
    compute_queue.destroy();
    copy_queue.destroy();
    present_queue.destroy();

    mutex.unlock();

    mutex.destroy();

    GPU::device_destroy(gpu_device);
}

SwapChain* Device::create_swap_chain(Window* window, GPU::TextureFormat surface_format)
{
    mutex.lock();
    SwapChain* sc = _allocate_object<SwapChain>();
    sc->init(allocator, this,
        {
            .gpu_device = gpu_device,
            .present_queue = &present_queue,
            .window = window->window_id,
            .surface_format = surface_format,
        }
    );
    mutex.unlock();
    return sc;
}

Fence* Device::create_fence(bool signaled)
{
    mutex.lock();
    Fence* fence = _allocate_object<Fence>();
    fence->init(allocator, this, gpu_device, signaled);
    mutex.unlock();
    return fence;
}

Semaphore* Device::create_semaphore()
{
    mutex.lock();
    Semaphore* semaphore = _allocate_object<Semaphore>();
    semaphore->init(allocator, this, gpu_device);
    mutex.unlock();
    return semaphore;
}

MemoryHeap* Device::create_memory_heap(GPU::HeapUsage usage, usize size)
{
    mutex.lock();
    MemoryHeap* heap = _allocate_object<MemoryHeap>();
    heap->init(allocator, this,
        {
            .device = gpu_device,
            .heap_usage = usage,
            .heap_size = size,
        }
    );
    mutex.unlock();
    return heap;
}

Buffer* Device::create_buffer(GPU::BufferUsage usage, usize size)
{
    mutex.lock();
    Buffer* buffer = _allocate_object<Buffer>();
    buffer->init(allocator, this,
        {
            .gpu_device = gpu_device,
            .usage = usage,
            .size = size,
        }
    );
    mutex.unlock();
    return buffer;
}

Sampler* Device::create_sampler(const SamplerInfo& sampler_info)
{
    mutex.lock();
    Sampler* sampler = _allocate_object<Sampler>();
    sampler->init(allocator, this, gpu_device, sampler_info);
    mutex.unlock();
    return sampler;
}

Texture* Device::create_texture(const TextureInfo& texture_info)
{
    mutex.lock();
    Texture* texture = _allocate_object<Texture>();
    texture->init(allocator, this, texture_info);
    mutex.unlock();
    return texture;
}

TextureView* Device::create_texture_view(const TextureViewInfo& texture_view_info)
{
    mutex.lock();
    TextureView* texture_view = _allocate_object<TextureView>();
    texture_view->init(allocator, this, texture_view_info);
    mutex.unlock();
    return texture_view;
}

DescriptorPool* Device::create_descriptor_pool(u32 max_sets, Slice<const GPU::DescriptorPoolSize> sizes)
{
    mutex.lock();
    DescriptorPool* descriptor_pool = _allocate_object<DescriptorPool>();
    descriptor_pool->init(allocator, this,
        {
            .device = gpu_device,
            .max_sets = max_sets,
            .sizes = sizes,
        }
    );
    mutex.unlock();
    return descriptor_pool;
}

PipelineLayout* Device::create_pipeline_layout(const PipelineLayoutInfo& pipeline_layout_info)
{
    mutex.lock();
    PipelineLayout* pipe_layout = _allocate_object<PipelineLayout>();
    pipe_layout->init(allocator, this, gpu_device, pipeline_layout_info);
    mutex.unlock();
    return pipe_layout;
}

Pipeline* Device::create_pipeline(const PipelineInfo& pipeline_info)
{
    mutex.lock();
    Pipeline* pipe = _allocate_object<Pipeline>();
    pipe->init(allocator, this, gpu_device, pipeline_info);
    mutex.unlock();
    return pipe;
}

CommandPool* Device::create_command_pool(GPU::QueueUsage gpu_queue_usage)
{
    mutex.lock();
    CommandPool* command_pool = _allocate_object<CommandPool>();
    command_pool->init(allocator, this,
        {
            .gpu_device = gpu_device,
            .gpu_queue_usage = gpu_queue_usage,
        }
    );
    mutex.unlock();
    return command_pool;
}

void Device::update_descriptor_sets(const Slice<const DescriptorWrite>& descriptor_writes)
{
    tmp_allocator.reset();

    Slice<GPU::WriteDescriptorInfo> gpu_write_infos = tmp_allocator.array<GPU::WriteDescriptorInfo>(descriptor_writes.len);
    for(usize i = 0; i < gpu_write_infos.len; i++)
    {
        gpu_write_infos[i] =
        {
            .descriptor_set = descriptor_writes[i].set->gpu_descriptor_set,
            .binding = descriptor_writes[i].binding,
            .array_element = descriptor_writes[i].array_element,
            .type = descriptor_writes[i].type,
            .buffers = {},
            .textures = {},
        };

        switch(descriptor_writes[i].type)
        {
        case GPU::DescriptorType::UniformBuffer:
        case GPU::DescriptorType::StorageBuffer:
        {
            Slice<GPU::DescriptorBufferInfo> buffers = tmp_allocator.array<GPU::DescriptorBufferInfo>(descriptor_writes[i].buffers.len);
            for(usize buffer_i = 0; buffer_i < gpu_write_infos[i].buffers.len; buffer_i++)
            {
                buffers[buffer_i] =
                {
                    .buffer = descriptor_writes[i].buffers[buffer_i].buffer->gpu_buffer,
                    .offset = descriptor_writes[i].buffers[buffer_i].offset,
                    .range = descriptor_writes[i].buffers[buffer_i].range,
                };
            }

            gpu_write_infos[i].buffers = buffers;
        }
            break;
        case GPU::DescriptorType::CombinedTextureSampler:
        {
            Slice<GPU::DescriptorTextureInfo> textures = tmp_allocator.array<GPU::DescriptorTextureInfo>(descriptor_writes[i].textures.len);
            for(usize texture_i = 0; texture_i < gpu_write_infos[i].textures.len; texture_i++)
            {
                textures[texture_i] =
                {
                    .texture_view = descriptor_writes[i].textures[texture_i].texture_view->gpu_texture_view,
                    .layout = descriptor_writes[i].textures[texture_i].layout,
                    .sampler = descriptor_writes[i].textures[texture_i].sampler->gpu_sampler,
                };
            }
            gpu_write_infos[i].textures = textures;
        }
            break;
        default:
            FailOn(true, "invalid descriptor type");
            break;
        }
    }

    GPU::descriptor_set_update_descriptors(
        {
            .device = gpu_device,
            .write_infos = gpu_write_infos,
        }
    );
}

void Device::release_object(DeviceObject* child)
{
    mutex.lock();
    Log::debug("[Graphics::Device({})]: Releasing child object({})", this, child);
    DebugAssert(
        allocated_objects.iter().find(child) != allocated_objects.iter().end(),
        "the allocated object it's not owned by this device"
    );
    allocated_objects.remove(child);
    allocator->free(Slice(reinterpret_cast<u8*>(child), 1));
    mutex.unlock();
}

void Device::_log_child_alloc(DeviceObject* child)
{
    Log::debug("[Graphics::Device({})]: Allocating child object({})", this, child);
}

}