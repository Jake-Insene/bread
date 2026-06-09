#include "gpu/gpu.h"

#include "gpu/gpu_adapter.h"
#include "gpu/gpu_debug_layer.h"
#include "gpu/vk/vk_adapter.h"


#define GPUValidationCheck(cond, ...) DebugAssert(!(cond), __VA_ARGS__)

static inline InternalGPU::GPUAdapter* current_adapter;
static inline Mem::Allocator* adapter_allocator_owner = nullptr;

void GPU::initialize(Mem::Allocator* allocator)
{
	adapter_allocator_owner = allocator;
	current_adapter = allocator->object<VulkanAdapter>();

	current_adapter->initialize(allocator);

#if defined(BREAD_ENABLE_GPU_DEBUG_LAYER)
	gpu_debug_layer.init(allocator);
#endif
}

void GPU::initialize_from_adapter(InternalGPU::GPUAdapter* adapter)
{
	current_adapter = adapter;
}

void GPU::shutdown()
{
	current_adapter->shutdown();

#if defined(BREAD_ENABLE_GPU_DEBUG_LAYER)
	gpu_debug_layer.destroy();
#endif

	// Only the process who calls GPU::initialize owns the memory of current_adapter.
	if(adapter_allocator_owner != nullptr)
	{
		adapter_allocator_owner->free(Slice(reinterpret_cast<u8*>(current_adapter), 1));
	}
}

InternalGPU::GPUAdapter* GPU::get_adapter()
{
	return current_adapter;
}

Slice<GPU::PhysicalDeviceID> GPU::physical_devices_enumerate()
{
	return current_adapter->physical_devices_enumerate();
}

GPU::PhysicalDeviceInfo GPU::physical_device_get_info(PhysicalDeviceID physical_device)
{
    GPUValidationCheck(physical_device.is_valid() == false, "invalid physical device");
	return current_adapter->physical_device_get_info(physical_device);
}

GPU::SurfaceID GPU::surface_create(const GPU::SurfaceCreateInfo& ci)
{
    GPUValidationCheck(ci.window_native_handle == 0, "invalid window native handle");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter->surface_create(ci));
}

void GPU::surface_destroy(GPU::SurfaceID surface)
{
    GPUValidationCheck(surface.is_valid() == false, "invalid surface");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(surface, current_adapter->surface_destroy(surface));
}

GPU::DeviceID GPU::device_create(const DeviceCreateInfo &ci)
{
    GPUValidationCheck(ci.physical_device.is_valid() == false, "invalid physical device");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter->device_create(ci);)
}

void GPU::device_destroy(DeviceID device)
{
    GPUValidationCheck(device.is_valid() == false, "invalid device");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(device, current_adapter->device_destroy(device));
}

GPU::SwapChainID GPU::swap_chain_create(const SwapChainCreateInfo& ci)
{
    GPUValidationCheck(ci.device.is_valid() == false, "invalid device");
	GPUValidationCheck(ci.surface.is_valid() == false, "invalid surface");
    GPUValidationCheck(ci.present_mode == GPU::PresentMode::Unknown, "invalid present mode");
    GPUValidationCheck(ci.format == GPU::TextureFormat::Unknown, "invalid surface format");
    GPUValidationCheck(ci.min_image_count == 0, "invalid min image count");

	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter->swap_chain_create(ci));
}

void GPU::swap_chain_destroy(SwapChainID swap_chain)
{
    GPUValidationCheck(swap_chain.is_valid() == false, "invalid swap chain");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(swap_chain, current_adapter->swap_chain_destroy(swap_chain));
}

u32 GPU::swap_chain_get_image_count(SwapChainID swap_chain)
{
    GPUValidationCheck(swap_chain.is_valid() == false, "invalid swap chain");
	return current_adapter->swap_chain_get_image_count(swap_chain);
}

GPU::TextureID GPU::swap_chain_get_image(SwapChainID swap_chain, u32 image_index)
{
	GPUValidationCheck(swap_chain.is_valid() == false, "invalid swapchain");
    GPUValidationCheck(image_index >= swap_chain_get_image_count(swap_chain), "invalid image index");
	return current_adapter->swap_chain_get_image(swap_chain, image_index);
}

GPU::TextureViewID GPU::swap_chain_get_image_view(SwapChainID swap_chain, u32 image_index)
{
	GPUValidationCheck(swap_chain.is_valid() == false, "invalid swapchain");
    GPUValidationCheck(image_index >= swap_chain_get_image_count(swap_chain), "invalid image index");
	return current_adapter->swap_chain_get_image_view(swap_chain, image_index);
}

GPU::AcquireResult GPU::swap_chain_acquire_next_image(SwapChainID swap_chain, const AcquireInfo& acquire_info, u32* image_index)
{
	GPUValidationCheck(swap_chain.is_valid() == false, "invalid swapchain");
	GPUValidationCheck(
        acquire_info.semaphore.is_valid() == false && acquire_info.fence.is_valid() == false, 
        "semaphore and fence can't be null, at least one is require"
    );
	GPUValidationCheck(image_index == nullptr, "image index must be a valid pointer to a u32");
	return current_adapter->swap_chain_acquire_next_image(swap_chain, acquire_info, image_index);
}

GPU::FenceID GPU::fence_create(const FenceCreateInfo &ci)
{
	GPUValidationCheck(ci.device.is_valid() == false, "invalid device");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter->fence_create(ci));
}

void GPU::fence_destroy(FenceID fence)
{
	GPUValidationCheck(fence.is_valid() == false, "invalid fence");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(fence, current_adapter->fence_destroy(fence));
}

bool GPU::fence_get_state(FenceID fence)
{
	GPUValidationCheck(fence.is_valid() == false, "invalid fence");
	return current_adapter->fence_get_state(fence);
}

void GPU::fence_reset(Slice<FenceID> fences)
{
    GPUValidationCheck(fences.len == 0, "at least one fence is expected");
	for(FenceID fence : fences)
	{
		GPUValidationCheck(fence.is_valid() == false, "invalid fence");
	}
	current_adapter->fence_reset(fences);
}

void GPU::fence_wait_for(Slice<FenceID> fences, bool wait_for_all, u64 timeout)
{
    GPUValidationCheck(fences.len == 0, "at least one fence is expected");
	for(FenceID fence : fences)
	{
		GPUValidationCheck(fence.is_valid() == false, "invalid fence");
	}
	current_adapter->fence_wait_for(fences, wait_for_all, timeout);
}

GPU::SemaphoreID GPU::semaphore_create(const SemaphoreCreateInfo &ci)
{
    GPUValidationCheck(ci.device.is_valid() == false, "invalid device");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter->semaphore_create(ci));
}

void GPU::semaphore_destroy(SemaphoreID semaphore)
{
    GPUValidationCheck(semaphore.is_valid() == false, "invalid semaphore");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(semaphore, current_adapter->semaphore_destroy(semaphore));
}

u32 GPU::queue_get_count(const QueueGetCountInfo& gci)
{
    GPUValidationCheck(gci.device.is_valid() == false, "invalid device");
    GPUValidationCheck(gci.usage == GPU::QueueUsage::Unknown, "invalid queue usage");

	return current_adapter->queue_get_count(gci);
}

GPU::QueueID GPU::queue_get(const QueueGetInfo& gi)
{
    GPUValidationCheck(gi.device.is_valid() == false, "invalid device");
    GPUValidationCheck(gi.usage == QueueUsage::Unknown, "invalid queue usage");
    GPUValidationCheck(gi.index >= queue_get_count({.device = gi.device, .usage = gi.usage}), "invalid queue index");

	return current_adapter->queue_get(gi);
}

void GPU::queue_execute_command_buffer(QueueID queue, const QueueExecuteInfo& execute_info)
{
    GPUValidationCheck(queue.is_valid() == false, "invalid queue");
	GPUValidationCheck(
        execute_info.wait_stages.len != 0 && execute_info.wait_semaphores.len != execute_info.wait_stages.len,
        "wait stages must be equal in len to wait semaphores or empty"
    );
	current_adapter->queue_execute_command_buffer(queue, execute_info);
}

GPU::AcquireResult GPU::queue_present(QueueID queue, const QueuePresentInfo& present_info)
{
    GPUValidationCheck(queue.is_valid() == false, "invalid queue");
	GPUValidationCheck(
        present_info.wait_semaphores.len != 0 && present_info.swapchains.len != present_info.wait_semaphores.len,
        "wait semaphores must be equal in len to swapchains or empty"
    );
	GPUValidationCheck(present_info.swapchains.len != present_info.image_indices.len,
		"image indices must be equal in len to swapchains"
	);
	return current_adapter->queue_present(queue, present_info);
}

void GPU::queue_wait_idle(QueueID queue)
{
    GPUValidationCheck(queue.is_valid() == false, "invalid queue");
	current_adapter->queue_wait_idle(queue);
}

GPU::MemoryHeapID GPU::memory_heap_create(const MemoryHeapCreateInfo& ci)
{
	GPUValidationCheck(ci.device.is_valid() == false, "invalid device");
    GPUValidationCheck(ci.heap_usage == GPU::HeapUsage::Unknown, "invalid heap usage");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter->memory_heap_create(ci));
}

void GPU::memory_heap_destroy(MemoryHeapID memory_heap)
{
    GPUValidationCheck(memory_heap.is_valid() == false, "invalid memory heap");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(memory_heap, current_adapter->memory_heap_destroy(memory_heap));
}

Slice<u8> GPU::memory_heap_map(GPU::MemoryHeapID memory_heap, usize offset, usize len)
{
	GPUValidationCheck(memory_heap.is_valid() == false, "invalid memory heap");
    GPUValidationCheck(len == 0, "invalid len");
	return current_adapter->memory_heap_map(memory_heap, offset, len);
}

void GPU::memory_heap_unmap(GPU::MemoryHeapID memory_heap, const Slice<u8>& memory)
{
	GPUValidationCheck(memory_heap.is_valid() == false, "invalid memory heap");
    GPUValidationCheck(memory.ptr() == nullptr, "invalid memory address");
	current_adapter->memory_heap_unmap(memory_heap, memory);
}

GPU::BufferID GPU::buffer_create(const BufferCreateInfo& ci)
{
	GPUValidationCheck(ci.device.is_valid() == false, "invalid device");
    GPUValidationCheck(ci.usage == GPU::BufferUsage(0), "invalid buffer usage");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter->buffer_create(ci));
}

void GPU::buffer_destroy(BufferID buffer)
{
    GPUValidationCheck(buffer.is_valid() == false, "invalid buffer");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(buffer, current_adapter->buffer_destroy(buffer));
}

GPU::MemoryRequirements GPU::buffer_get_memory_requirements(BufferID buffer)
{
	GPUValidationCheck(buffer.is_valid() == false, "invalid buffer");
	return current_adapter->buffer_get_memory_requirements(buffer);
}

void GPU::buffer_bind_memory_heap(BufferID buffer, const BindMemoryInfo& bind_info)
{
	GPUValidationCheck(buffer.is_valid() == false, "invalid buffer");
    GPUValidationCheck(bind_info.memory_heap.is_valid() == false, "invalid memory heap");
	current_adapter->buffer_bind_memory_heap(buffer, bind_info);
}

GPU::SamplerID GPU::sampler_create(const SamplerCreateInfo& ci)
{
    GPUValidationCheck(ci.device.is_valid() == false, "invalid device");
    GPUValidationCheck(ci.min_filter == GPU::Filter::Unknown, "invalid min filter");
    GPUValidationCheck(ci.mag_filter == GPU::Filter::Unknown, "invalid mag filter");
    GPUValidationCheck(ci.mipmap_mode == GPU::SamplerMipMapMode::Unknown, "invalid mipmap mode");
    GPUValidationCheck(ci.address_mode_u == GPU::SamplerAddressMode::Unknown, "invalid address mode u");
    GPUValidationCheck(ci.address_mode_v == GPU::SamplerAddressMode::Unknown, "invalid address mode v");
    GPUValidationCheck(ci.address_mode_w == GPU::SamplerAddressMode::Unknown, "invalid address mode w");
    GPUValidationCheck(ci.compare_op == GPU::CompareOp::Unknown, "invalid compare op");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter->sampler_create(ci));
}

void GPU::sampler_destroy(SamplerID sampler)
{
    GPUValidationCheck(sampler.is_valid() == false, "invalid sampler");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(sampler, current_adapter->sampler_destroy(sampler));
}

GPU::TextureID GPU::texture_create(const TextureCreateInfo& ci)
{
	GPUValidationCheck(ci.device.is_valid() == false, "invalid device");
    GPUValidationCheck(ci.type == GPU::TextureType::Unknown, "invalid texture type");
    GPUValidationCheck(ci.format == GPU::TextureFormat::Unknown, "invalid texture format");
    GPUValidationCheck(ci.extent.x == 0 || ci.extent.y == 0 || ci.extent.z == 0, "invalid texture size");
    GPUValidationCheck(ci.mip_levels == 0, "invalid texture mip levels");
    GPUValidationCheck(ci.array_levels == 0, "invalid texture array levels");
    GPUValidationCheck(ci.sample_count == GPU::SampleCount::Unknown, "invalid texture sample count");
    GPUValidationCheck(ci.tiling == GPU::TextureTiling::Unknown, "invalid texture tiling");
    GPUValidationCheck(ci.usage == GPU::TextureUsage(0), "invalid texture usage");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter->texture_create(ci));
}

void GPU::texture_destroy(TextureID texture)
{
    GPUValidationCheck(texture.is_valid() == false, "invalid texture");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(texture, current_adapter->texture_destroy(texture));
}

GPU::MemoryRequirements GPU::texture_get_memory_requirements(TextureID texture)
{
	GPUValidationCheck(texture.is_valid() == false, "invalid texture");
	return current_adapter->texture_get_memory_requirements(texture);
}

void GPU::texture_bind_memory_heap(TextureID texture, const BindMemoryInfo& bind_info)
{
	GPUValidationCheck(texture.is_valid() == false, "invalid texture");
    GPUValidationCheck(bind_info.memory_heap.is_valid() == false, "invalid memory heap");
	current_adapter->texture_bind_memory_heap(texture, bind_info);
}

GPU::TextureViewID GPU::texture_view_create(const TextureViewCreateInfo& ci)
{
	GPUValidationCheck(ci.device.is_valid() == false, "invalid device");
    GPUValidationCheck(ci.type == GPU::TextureViewType::Unknown, "invalid texture view type");
    GPUValidationCheck(ci.texture.is_valid() == false, "invalid texture");
    GPUValidationCheck(ci.format == GPU::TextureFormat::Unknown, "invalid texture format");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter->texture_view_create(ci));
}

void GPU::texture_view_destroy(TextureViewID texture_view)
{
	GPUValidationCheck(texture_view.is_valid() == false, "invalid texture view");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(texture_view, current_adapter->texture_view_destroy(texture_view));
}

GPU::DescriptorSetLayoutID GPU::descriptor_set_layout_create(const DescriptorSetLayoutCreateInfo& ci)
{
	GPUValidationCheck(ci.device.is_valid() == false, "invalid device");
    GPUValidationCheck(ci.bindings.len == 0, "invalid descriptor binding count");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter->descriptor_set_layout_create(ci));
}

void GPU::descriptor_set_layout_destroy(DescriptorSetLayoutID descriptor_set_layout)
{
    GPUValidationCheck(descriptor_set_layout.is_valid() == false, "invalid descriptor set layout");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(descriptor_set_layout, current_adapter->descriptor_set_layout_destroy(descriptor_set_layout));
}

GPU::DescriptorPoolID GPU::descriptor_pool_create(const DescriptorPoolCreateInfo &ci)
{
    GPUValidationCheck(ci.device.is_valid() == false, "invalid device");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter->descriptor_pool_create(ci));
}

void GPU::descriptor_pool_destroy(DescriptorPoolID descriptor_pool)
{
    GPUValidationCheck(descriptor_pool.is_valid() == false, "invalid descriptor pool");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(descriptor_pool, current_adapter->descriptor_pool_destroy(descriptor_pool));
}

GPU::DescriptorSetID GPU::descriptor_set_allocate(const GPU::DescriptorSetAllocateInfo& ci)
{
	GPUValidationCheck(ci.device.is_valid() == false, "invalid device");
    GPUValidationCheck(ci.pool.is_valid() == false, "invalid descriptor pool");
    GPUValidationCheck(ci.set_layout.is_valid() == false, "invalid set layout");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter->descriptor_set_allocate(ci));
}

void GPU::descriptor_set_free(GPU::DescriptorSetID descriptor_set)
{
    GPUValidationCheck(descriptor_set.is_valid() == false, "invalid descriptor set");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(descriptor_set, current_adapter->descriptor_set_free(descriptor_set));
}

void GPU::descriptor_set_update_descriptors(DescriptorSetID descriptor_set, const UpdateDescriptorInfo& update_info)
{
    GPUValidationCheck(descriptor_set.is_valid() == false, "invalid descriptor set");
	for(usize write_index = 0; write_index < update_info.write_infos.len; write_index++)
	{
		const WriteDescriptorInfo& write_info = update_info.write_infos[write_index];
		GPUValidationCheck(write_info.type == GPU::DescriptorType::Unknown, "invalid descriptor type")
		GPUValidationCheck(
			IsAnyEqual(write_info.type, GPU::DescriptorType::UniformBuffer, GPU::DescriptorType::StorageBuffer)
			&& write_info.count != write_info.buffers.len,
			"invalid buffers len, WriteDescriptorInfo::count buffers were expected"
		);
		GPUValidationCheck(
			IsAnyEqual(write_info.type, GPU::DescriptorType::CombinedTextureSampler)
			&& write_info.count != write_info.textures.len,
			"invalid textures len, WriteDescriptorInfo::count textures were expected"
		);
	}
	current_adapter->descriptor_set_update_descriptors(descriptor_set, update_info);
}

GPU::PipelineLayoutID GPU::pipeline_layout_create(const PipelineLayoutCreateInfo &ci)
{
	GPUValidationCheck(ci.device.is_valid() == false, "invalid device");

	for(GPU::ConstantBlock cb : ci.constant_blocks)
	{
		GPUValidationCheck(cb.size > GPU::MaxConstantBlockSize, "a constant block size must be less than or equal to 128 bytes");
    	GPUValidationCheck(
    	    Mem::align_up(cb.size, GPU::ConstantBlockAlignment) != cb.size,
    	    "a constant block size must be GPU::ConstantBlockAlignment bytes aligned"
    	);
	}

	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter->pipeline_layout_create(ci));
}

void GPU::pipeline_layout_destroy(PipelineLayoutID pipeline_layout)
{
    GPUValidationCheck(pipeline_layout.is_valid() == false, "invalid pipeline layout");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(pipeline_layout, current_adapter->pipeline_layout_destroy(pipeline_layout));
}

GPU::PipelineID GPU::pipeline_create(const GPU::PipelineCreateInfo& ci)
{
	GPUValidationCheck(ci.device.is_valid() == false, "invalid device");
    GPUValidationCheck(ci.bind_point == GPU::PipelineBindPoint::Unknown, "invalid pipeline bind point");
    GPUValidationCheck(ci.input_assembly.topology == GPU::PrimitiveTopology::Unknown, "invalid topology");
    GPUValidationCheck(ci.shader_stages.len == 0, "at least one shader stage was expected");
    GPUValidationCheck(ci.pipeline_layout.is_valid() == false, "invalid pipeline layout");
	
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter->pipeline_create(ci));
}

void GPU::pipeline_destroy(PipelineID pipeline)
{
    GPUValidationCheck(pipeline.is_valid() == false, "invalid pipeline");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(pipeline, current_adapter->pipeline_destroy(pipeline));
}

GPU::CommandPoolID GPU::command_pool_create(const CommandPoolCreateInfo &ci)
{
	GPUValidationCheck(ci.device.is_valid() == false, "invalid device");
    GPUValidationCheck(ci.queue.is_valid() == false, "invalid queue");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter->command_pool_create(ci));
}

void GPU::command_pool_destroy(CommandPoolID command_pool)
{
    GPUValidationCheck(command_pool.is_valid() == false, "invalid command pool");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(command_pool, current_adapter->command_pool_destroy(command_pool));
}

GPU::CommandBufferID GPU::command_buffer_allocate(const CommandBufferAllocateInfo& ci)
{
    GPUValidationCheck(ci.pool.is_valid() == false, "invalid command pool");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter->command_buffer_allocate(ci));
}

void GPU::command_buffer_free(CommandBufferID command_buffer)
{
    GPUValidationCheck(command_buffer.is_valid() == false, "invalid command buffer");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(command_buffer, current_adapter->command_buffer_free(command_buffer));
}

void GPU::command_buffer_begin(CommandBufferID command_buffer)
{
    GPUValidationCheck(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter->command_buffer_begin(command_buffer);
}

void GPU::command_buffer_end(CommandBufferID command_buffer)
{
    GPUValidationCheck(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter->command_buffer_end(command_buffer);
}

void GPU::command_buffer_begin_renderpass(CommandBufferID command_buffer, const RenderPassBeginInfo& begin_info)
{
	GPUValidationCheck(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter->command_buffer_begin_renderpass(command_buffer, begin_info);
}

void GPU::command_buffer_end_renderpass(CommandBufferID command_buffer, const RenderPassEndInfo& end_info)
{
    GPUValidationCheck(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter->command_buffer_end_renderpass(command_buffer, end_info);
}

void GPU::command_buffer_pipeline_barrier(CommandBufferID command_buffer, const PipelineBarrier& pipeline_barrier)
{
    GPUValidationCheck(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter->command_buffer_pipeline_barrier(command_buffer, pipeline_barrier);
}

void GPU::command_buffer_copy_buffer_to_texture(CommandBufferID command_buffer, const CopyBufferToTextureInfo& copy_info)
{
	GPUValidationCheck(command_buffer.is_valid() == false, "invalid command buffer");
    GPUValidationCheck(copy_info.src_buffer.is_valid() == false, "invalid source buffer");
    GPUValidationCheck(copy_info.dest_texture.is_valid() == false, "invalid destination texture");
    GPUValidationCheck(copy_info.dest_layout == GPU::TextureLayout::Unknown, "invalid destination texture layout");
	current_adapter->command_buffer_copy_buffer_to_texture(command_buffer, copy_info);
}

void GPU::command_buffer_copy_buffer(CommandBufferID command_buffer, const BufferCopyInfo& copy_info)
{
	GPUValidationCheck(command_buffer.is_valid() == false, "invalid command buffer");
	GPUValidationCheck(copy_info.src_buffer.is_valid() == false, "invalid source buffer");
    GPUValidationCheck(copy_info.dest_buffer.is_valid() == false, "invalid destination buffer");
	current_adapter->command_buffer_copy_buffer(command_buffer, copy_info);
}

void GPU::command_buffer_bind_pipeline(CommandBufferID command_buffer, PipelineBindPoint bind_point, PipelineID pipeline)
{
	GPUValidationCheck(command_buffer.is_valid() == false, "invalid command buffer");
	GPUValidationCheck(bind_point == GPU::PipelineBindPoint::Unknown, "invalid bind point");
    GPUValidationCheck(pipeline.is_valid() == false, "invalid pipeline");
	current_adapter->command_buffer_bind_pipeline(command_buffer, bind_point, pipeline);
}

void GPU::command_buffer_bind_descriptor_sets(CommandBufferID command_buffer, PipelineBindPoint bind_point, PipelineLayoutID pipeline_layout, u32 base_set, const Slice<DescriptorSetID>& descriptor_sets)
{
    GPUValidationCheck(command_buffer.is_valid() == false, "invalid command buffer");
	GPUValidationCheck(bind_point == GPU::PipelineBindPoint::Unknown, "invalid bind point");
    GPUValidationCheck(pipeline_layout.is_valid() == false, "invalid pipeline layout");
    GPUValidationCheck(descriptor_sets.len == 0, "invalid descriptor set count");
	current_adapter->command_buffer_bind_descriptor_sets(command_buffer, bind_point, pipeline_layout, base_set, descriptor_sets);
}


void GPU::command_buffer_bind_vertex_buffers(CommandBufferID command_buffer, u32 base_binding, const Slice<BufferID>& buffers, const Slice<usize>& offsets)
{
    GPUValidationCheck(command_buffer.is_valid() == false, "invalid command buffer");
    GPUValidationCheck(buffers.len != offsets.len, "inconsistent buffers and offsets count");
	current_adapter->command_buffer_bind_vertex_buffers(command_buffer, base_binding, buffers, offsets);
}

void GPU::command_buffer_constant_block(GPU::CommandBufferID command_buffer, PipelineLayoutID pipeline_layout, GPU::ShaderStage stages, u32 offset, u32 size, MemoryAddress block_address)
{
	GPUValidationCheck(command_buffer.is_valid() == false, "invalid command buffer");
	GPUValidationCheck(pipeline_layout.is_valid() == false, "invalid pipeline layout");
	GPUValidationCheck(stages == GPU::ShaderStage(0), "invalid shader stages");
    GPUValidationCheck(size == 0, "invalid constant block size");
    GPUValidationCheck(Mem::align_up(size, ConstantBlockAlignment) != size,
    	"a constant block size must be GPU::ConstantBlockAlignment bytes aligned"
	);
	GPUValidationCheck(Mem::align_up(offset, ConstantBlockAlignment) != offset,
    	"a constant block offset must be GPU::ConstantBlockAlignment bytes aligned"
	);
	current_adapter->command_buffer_constant_block(command_buffer, pipeline_layout, stages, offset, size, block_address);
}

void GPU::command_buffer_set_viewports(CommandBufferID command_buffer, u32 base_viewport, const Slice<const Viewport>& viewports)
{
    GPUValidationCheck(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter->command_buffer_set_viewports(command_buffer, base_viewport, viewports);
}

void GPU::command_buffer_set_scissors(CommandBufferID command_buffer, u32 base_scissor, const Slice<const Scissor>& scissors)
{
    GPUValidationCheck(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter->command_buffer_set_scissors(command_buffer, base_scissor, scissors);
}

void GPU::command_buffer_draw(GPU::CommandBufferID command_buffer, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance)
{
    GPUValidationCheck(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter->command_buffer_draw(command_buffer, vertex_count, instance_count, base_vertex, base_instance);
}

