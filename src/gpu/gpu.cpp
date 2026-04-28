#include "gpu/gpu.h"

#include "gpu/gpu_adapter.h"
#include "gpu/gpu_debug_layer.h"
#include "gpu/vk/vk_driver.h"


#define GPUFailOn FailOn


static inline InternalGPU::GPUAdapter current_adapter;

void GPU::initialize(const mem::Allocator& allocator)
{
	current_adapter = VulkanDriver::get_adapter();

	current_adapter.initialize(allocator);

#if defined(BREAD_ENABLE_GPU_DEBUG_LAYER)
	gpu_debug_layer.init(allocator);
#endif
}

void GPU::initialize_from_adapter(InternalGPU::GPUAdapter *adapter)
{
	current_adapter = *adapter;
}

void GPU::shutdown()
{
	current_adapter.shutdown();

#if defined(BREAD_ENABLE_GPU_DEBUG_LAYER)
	gpu_debug_layer.destroy();
#endif
}

InternalGPU::GPUAdapter* GPU::get_adapter()
{
	return &current_adapter;
}

Slice<GPU::PhysicalDeviceID> GPU::physical_devices_enumerate()
{
	return current_adapter.physical_devices_enumerate();
}

GPU::PhysicalDeviceInfo GPU::physical_device_get_info(PhysicalDeviceID physical_device)
{
    GPUFailOn(physical_device.is_valid() == false, "invalid physical device");
	return current_adapter.physical_device_get_info(physical_device);
}

GPU::SurfaceID GPU::surface_create(const GPU::SurfaceCreateInfo& ci)
{
    GPUFailOn(ci.window_native_handle == 0, "invalid window native handle");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter.surface_create(ci));
}

void GPU::surface_destroy(GPU::SurfaceID surface)
{
    GPUFailOn(surface.is_valid() == false, "invalid surface");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(surface, current_adapter.surface_destroy(surface));
}

GPU::DeviceID GPU::device_create(const DeviceCreateInfo &ci)
{
    GPUFailOn(ci.physical_device.is_valid() == false, "invalid physical device");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter.device_create(ci);)
}

void GPU::device_destroy(DeviceID device)
{
    GPUFailOn(device.is_valid() == false, "invalid device");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(device, current_adapter.device_destroy(device));
}

GPU::SwapChainID GPU::swap_chain_create(const SwapChainCreateInfo& ci)
{
    GPUFailOn(ci.device.is_valid() == false, "invalid device");
	GPUFailOn(ci.surface.is_valid() == false, "invalid surface");
    GPUFailOn(ci.present_mode == GPU::PresentMode::Unknown, "invalid present mode");
    GPUFailOn(ci.format == GPU::TextureFormat::Unknown, "invalid surface format");
    GPUFailOn(ci.min_image_count == 0, "invalid min image count");

	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter.swap_chain_create(ci));
}

void GPU::swap_chain_destroy(SwapChainID swap_chain)
{
    GPUFailOn(swap_chain.is_valid() == false, "invalid swap chain");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(swap_chain, current_adapter.swap_chain_destroy(swap_chain));
}

u32 GPU::swap_chain_get_image_count(SwapChainID swap_chain)
{
    GPUFailOn(swap_chain.is_valid() == false, "invalid swap chain");
	return current_adapter.swap_chain_get_image_count(swap_chain);
}

GPU::TextureID GPU::swap_chain_get_texture(SwapChainID swap_chain, u32 image_index)
{
	GPUFailOn(swap_chain.is_valid() == false, "invalid swapchain");
    GPUFailOn(image_index >= swap_chain_get_image_count(swap_chain), "invalid image index");
	return current_adapter.swap_chain_get_texture(swap_chain, image_index);
}

GPU::AcquireResult GPU::swap_chain_acquire_next_image(SwapChainID swap_chain, const AcquireInfo& acquire_info, u32* image_index)
{
	GPUFailOn(swap_chain.is_valid() == false, "invalid swapchain");
	GPUFailOn(
        acquire_info.semaphore.is_valid() == false && acquire_info.fence.is_valid() == false, 
        "semaphore and fence can't be null, at least one is require"
    );
	GPUFailOn(image_index == nullptr, "image index must be a valid pointer to a u32");
	return current_adapter.swap_chain_acquire_next_image(swap_chain, acquire_info, image_index);
}

GPU::FenceID GPU::fence_create(const FenceCreateInfo &ci)
{
	GPUFailOn(ci.device.is_valid() == false, "invalid device");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter.fence_create(ci));
}

void GPU::fence_destroy(FenceID fence)
{
	GPUFailOn(fence.is_valid() == false, "invalid fence");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(fence, current_adapter.fence_destroy(fence));
}

bool GPU::fence_get_state(FenceID fence)
{
	GPUFailOn(fence.is_valid() == false, "invalid fence");
	return current_adapter.fence_get_state(fence);
}

void GPU::fence_reset(Slice<FenceID> fences)
{
    GPUFailOn(fences.len == 0, "at least one fence is expected");
	for(FenceID fence : fences)
	{
		GPUFailOn(fence.is_valid() == false, "invalid fence");
	}
	current_adapter.fence_reset(fences);
}

void GPU::fence_wait_for(Slice<FenceID> fences, bool wait_for_all, u64 timeout)
{
    GPUFailOn(fences.len == 0, "at least one fence is expected");
	for(FenceID fence : fences)
	{
		GPUFailOn(fence.is_valid() == false, "invalid fence");
	}
	current_adapter.fence_wait_for(fences, wait_for_all, timeout);
}

GPU::SemaphoreID GPU::semaphore_create(const SemaphoreCreateInfo &ci)
{
    GPUFailOn(ci.device.is_valid() == false, "invalid device");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter.semaphore_create(ci));
}

void GPU::semaphore_destroy(SemaphoreID semaphore)
{
    GPUFailOn(semaphore.is_valid() == false, "invalid semaphore");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(semaphore, current_adapter.semaphore_destroy(semaphore));
}

GPU::QueueID GPU::queue_create(const QueueCreateInfo& ci)
{
    GPUFailOn(ci.device.is_valid() == false, "invalid device");
    GPUFailOn(ci.usage == GPU::QueueUsage::Unknown, "invalid queue usage");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter.queue_create(ci));
}

void GPU::queue_destroy(QueueID queue)
{
    GPUFailOn(queue.is_valid() == false, "invalid queue");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(queue, current_adapter.queue_destroy(queue));
}

void GPU::queue_execute_command_buffer(QueueID queue, const QueueExecuteInfo& execute_info)
{
    GPUFailOn(queue.is_valid() == false, "invalid queue");
	GPUFailOn(
        execute_info.wait_stages.len != 0 && execute_info.wait_semaphores.len != execute_info.wait_stages.len,
        "wait stages must be equal in len to wait semaphores or empty"
    );
	current_adapter.queue_execute_command_buffer(queue, execute_info);
}

GPU::AcquireResult GPU::queue_present(QueueID queue, const QueuePresentInfo& present_info)
{
    GPUFailOn(queue.is_valid() == false, "invalid queue");
	GPUFailOn(
        present_info.wait_semaphores.len != 0 && present_info.swapchains.len != present_info.wait_semaphores.len,
        "wait semaphores must be equal in len to swapchains or empty"
    );
	GPUFailOn(present_info.swapchains.len != present_info.image_indices.len,
		"image indices must be equal in len to swapchains"
	);
	return current_adapter.queue_present(queue, present_info);
}

void GPU::queue_wait_idle(QueueID queue)
{
    GPUFailOn(queue.is_valid() == false, "invalid queue");
	current_adapter.queue_wait_idle(queue);
}

GPU::MemoryHeapID GPU::memory_heap_create(const MemoryHeapCreateInfo& ci)
{
	GPUFailOn(ci.device.is_valid() == false, "invalid device");
    GPUFailOn(ci.heap_usage == GPU::HeapUsage::CPUExclusive, "invalid heap usage");
    GPUFailOn(
		mem::align_up(ci.heap_size, GPU::HeapAlignment) != ci.heap_size,
		"invalid heap alignment"
	);
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter.memory_heap_create(ci));
}

void GPU::memory_heap_destroy(MemoryHeapID memory_heap)
{
    GPUFailOn(memory_heap.is_valid() == false, "invalid memory heap");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(memory_heap, current_adapter.memory_heap_destroy(memory_heap));
}

Slice<u8> GPU::memory_heap_map(GPU::MemoryHeapID memory_heap, usize offset, usize len)
{
	GPUFailOn(memory_heap.is_valid() == false, "invalid memory heap");
    GPUFailOn(len == 0, "invalid len");
	return current_adapter.memory_heap_map(memory_heap, offset, len);
}

void GPU::memory_heap_unmap(GPU::MemoryHeapID memory_heap, const Slice<u8>& memory)
{
	GPUFailOn(memory_heap.is_valid() == false, "invalid memory heap");
    GPUFailOn(memory.ptr() == nullptr, "invalid memory address");
	current_adapter.memory_heap_unmap(memory_heap, memory);
}

GPU::BufferID GPU::buffer_create(const BufferCreateInfo& ci)
{
	GPUFailOn(ci.device.is_valid() == false, "invalid device");
    GPUFailOn(ci.usage == GPU::BufferUsage(0), "invalid buffer usage");
    GPUFailOn(
		mem::align_up(ci.size, GPU::MinHeapResourceAlignment) != ci.size,
		"invalid buffer alignment"
	);
    GPUFailOn(ci.memory_heap.is_valid() == false, "invalid memory heap");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter.buffer_create(ci));
}

void GPU::buffer_destroy(BufferID buffer)
{
    GPUFailOn(buffer.is_valid() == false, "invalid buffer");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(buffer, current_adapter.buffer_destroy(buffer));
}

GPU::SamplerID GPU::sampler_create(const SamplerCreateInfo& ci)
{
    GPUFailOn(ci.device.is_valid() == false, "invalid device");
    GPUFailOn(ci.min_filter == GPU::Filter::Unknown, "invalid min filter");
    GPUFailOn(ci.mag_filter == GPU::Filter::Unknown, "invalid mag filter");
    GPUFailOn(ci.mipmap_mode == GPU::SamplerMipMapMode::Unknown, "invalid mipmap mode");
    GPUFailOn(ci.address_mode_u == GPU::SamplerAddressMode::Unknown, "invalid address mode u");
    GPUFailOn(ci.address_mode_v == GPU::SamplerAddressMode::Unknown, "invalid address mode v");
    GPUFailOn(ci.address_mode_w == GPU::SamplerAddressMode::Unknown, "invalid address mode w");
    GPUFailOn(ci.compare_op == GPU::CompareOp::Unknown, "invalid compare op");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter.sampler_create(ci));
}

void GPU::sampler_destroy(SamplerID sampler)
{
    GPUFailOn(sampler.is_valid() == false, "invalid sampler");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(sampler, current_adapter.sampler_destroy(sampler));
}

GPU::TextureID GPU::texture_create(const TextureCreateInfo& ci)
{
	GPUFailOn(ci.device.is_valid() == false, "invalid device");
    GPUFailOn(ci.type == GPU::TextureType::Unknown, "invalid texture type");
    GPUFailOn(ci.format == GPU::TextureFormat::Unknown, "invalid texture format");
    GPUFailOn(ci.extent.x == 0 || ci.extent.y == 0 || ci.extent.z == 0, "invalid texture size");
    GPUFailOn(ci.mip_levels == 0, "invalid texture mip levels");
    GPUFailOn(ci.array_levels == 0, "invalid texture array levels");
    GPUFailOn(ci.sample_count == GPU::SampleCount::Unknown, "invalid texture sample count");
    GPUFailOn(ci.tiling == GPU::TextureTiling::Unknown, "invalid texture tiling");
    GPUFailOn(ci.usage == GPU::TextureUsage(0), "invalid texture usage");
    GPUFailOn(ci.memory_heap.is_valid() == false, "invalid texture memory heap");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter.texture_create(ci));
}

void GPU::texture_destroy(TextureID texture)
{
    GPUFailOn(texture.is_valid() == false, "invalid texture");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(texture, current_adapter.texture_destroy(texture));
}

GPU::DescriptorSetLayoutID GPU::descriptor_set_layout_create(const DescriptorSetLayoutCreateInfo& ci)
{
	GPUFailOn(ci.device.is_valid() == false, "invalid device");
    GPUFailOn(ci.bindings.len == 0, "invalid descriptor binding count");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter.descriptor_set_layout_create(ci));
}

void GPU::descriptor_set_layout_destroy(DescriptorSetLayoutID descriptor_set_layout)
{
    GPUFailOn(descriptor_set_layout.is_valid() == false, "invalid descriptor set layout");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(descriptor_set_layout, current_adapter.descriptor_set_layout_destroy(descriptor_set_layout));
}

GPU::DescriptorPoolID GPU::descriptor_pool_create(const DescriptorPoolCreateInfo &ci)
{
    GPUFailOn(ci.device.is_valid() == false, "invalid device");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter.descriptor_pool_create(ci));
}

void GPU::descriptor_pool_destroy(DescriptorPoolID descriptor_pool)
{
    GPUFailOn(descriptor_pool.is_valid() == false, "invalid descriptor pool");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(descriptor_pool, current_adapter.descriptor_pool_destroy(descriptor_pool));
}

GPU::DescriptorSetID GPU::descriptor_set_allocate(const GPU::DescriptorSetAllocateInfo& ci)
{
	GPUFailOn(ci.device.is_valid() == false, "invalid device");
    GPUFailOn(ci.pool.is_valid() == false, "invalid descriptor pool");
    GPUFailOn(ci.set_layout.is_valid() == false, "invalid set layout");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter.descriptor_set_allocate(ci));
}

void GPU::descriptor_set_free(GPU::DescriptorSetID descriptor_set)
{
    GPUFailOn(descriptor_set.is_valid() == false, "invalid descriptor set");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(descriptor_set, current_adapter.descriptor_set_free(descriptor_set));
}

void GPU::descriptor_set_update_descriptors(DescriptorSetID descriptor_set, const UpdateDescriptorInfo& update_info)
{
    GPUFailOn(descriptor_set.is_valid() == false, "invalid descriptor set");
	for(usize write_index = 0; write_index < update_info.write_infos.len; write_index++)
	{
		const WriteDescriptorInfo& write_info = update_info.write_infos[write_index];
		GPUFailOn(write_info.type == GPU::DescriptorType::Unknown, "invalid descriptor type")
		GPUFailOn(
			IsAnyEqual(write_info.type, GPU::DescriptorType::UniformBuffer, GPU::DescriptorType::StorageBuffer)
			&& write_info.count != write_info.buffers.len,
			"invalid buffers len, WriteDescriptorInfo::count buffers were expected"
		);
		GPUFailOn(
			IsAnyEqual(write_info.type, GPU::DescriptorType::CombinedTextureSampler)
			&& write_info.count != write_info.textures.len,
			"invalid textures len, WriteDescriptorInfo::count textures were expected"
		);
	}
	current_adapter.descriptor_set_update_descriptors(descriptor_set, update_info);
}

GPU::PipelineLayoutID GPU::pipeline_layout_create(const PipelineLayoutCreateInfo &ci)
{
	GPUFailOn(ci.device.is_valid() == false, "invalid device");

	for(GPU::ConstantBlock cb : ci.constant_blocks)
	{
		GPUFailOn(cb.size > GPU::MaxConstantBlockSize, "a constant block size must be less than or equal to 128 bytes");
    	GPUFailOn(
    	    mem::align_up(cb.size, GPU::ConstantBlockAlignment) != cb.size,
    	    "a constant block size must be GPU::ConstantBlockAlignment bytes aligned"
    	);
	}

	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter.pipeline_layout_create(ci));
}

void GPU::pipeline_layout_destroy(PipelineLayoutID pipeline_layout)
{
    GPUFailOn(pipeline_layout.is_valid() == false, "invalid pipeline layout");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(pipeline_layout, current_adapter.pipeline_layout_destroy(pipeline_layout));
}

GPU::PipelineID GPU::pipeline_create(const GPU::PipelineCreateInfo& ci)
{
	GPUFailOn(ci.device.is_valid() == false, "invalid device");
    GPUFailOn(ci.bind_point == GPU::PipelineBindPoint::Unknown, "invalid pipeline bind point");
    GPUFailOn(ci.input_assembly.topology == GPU::PrimitiveTopology::Unknown, "invalid topology");
    GPUFailOn(ci.shader_stages.len == 0, "at least one shader stage was expected");
    GPUFailOn(ci.pipeline_layout.is_valid() == false, "invalid pipeline layout");
	
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter.pipeline_create(ci));
}

void GPU::pipeline_destroy(PipelineID pipeline)
{
    GPUFailOn(pipeline.is_valid() == false, "invalid pipeline");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(pipeline, current_adapter.pipeline_destroy(pipeline));
}

GPU::CommandPoolID GPU::command_pool_create(const CommandPoolCreateInfo &ci)
{
	GPUFailOn(ci.device.is_valid() == false, "invalid device");
    GPUFailOn(ci.queue.is_valid() == false, "invalid queue");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter.command_pool_create(ci));
}

void GPU::command_pool_destroy(CommandPoolID command_pool)
{
    GPUFailOn(command_pool.is_valid() == false, "invalid command pool");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(command_pool, current_adapter.command_pool_destroy(command_pool));
}

GPU::CommandBufferID GPU::command_buffer_allocate(const CommandBufferAllocateInfo& ci)
{
    GPUFailOn(ci.pool.is_valid() == false, "invalid command pool");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(current_adapter.command_buffer_allocate(ci));
}

void GPU::command_buffer_free(CommandBufferID command_buffer)
{
    GPUFailOn(command_buffer.is_valid() == false, "invalid command buffer");
	GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(command_buffer, current_adapter.command_buffer_free(command_buffer));
}

void GPU::command_buffer_begin(CommandBufferID command_buffer)
{
    GPUFailOn(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter.command_buffer_begin(command_buffer);
}

void GPU::command_buffer_end(CommandBufferID command_buffer)
{
    GPUFailOn(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter.command_buffer_end(command_buffer);
}

void GPU::command_buffer_begin_renderpass(CommandBufferID command_buffer, const RenderPassBeginInfo& begin_info)
{
	GPUFailOn(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter.command_buffer_begin_renderpass(command_buffer, begin_info);
}

void GPU::command_buffer_end_renderpass(CommandBufferID command_buffer, const RenderPassEndInfo& end_info)
{
    GPUFailOn(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter.command_buffer_end_renderpass(command_buffer, end_info);
}

void GPU::command_buffer_memory_barrier(CommandBufferID command_buffer, const PipelineMemoryBarrier& memory_barrier)
{
    GPUFailOn(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter.command_buffer_memory_barrier(command_buffer, memory_barrier);
}

void GPU::command_buffer_buffer_barrier(CommandBufferID command_buffer, const PipelineBufferBarrier& buffer_barrier)
{
    GPUFailOn(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter.command_buffer_buffer_barrier(command_buffer, buffer_barrier);
}

void GPU::command_buffer_texture_barrier(CommandBufferID command_buffer, const PipelineTextureBarrier& texture_barrier)
{
    GPUFailOn(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter.command_buffer_texture_barrier(command_buffer, texture_barrier);
}

void GPU::command_buffer_copy_buffer_to_texture(CommandBufferID command_buffer, const CopyBufferToTextureInfo& copy_info)
{
	GPUFailOn(command_buffer.is_valid() == false, "invalid command buffer");
    GPUFailOn(copy_info.source_buffer.is_valid() == false, "invalid source buffer");
    GPUFailOn(copy_info.destination_texture.is_valid() == false, "invalid destination texture");
	current_adapter.command_buffer_copy_buffer_to_texture(command_buffer, copy_info);
}

void GPU::command_buffer_copy_buffer(CommandBufferID command_buffer, const BufferCopyInfo& copy_info)
{
	GPUFailOn(command_buffer.is_valid() == false, "invalid command buffer");
	GPUFailOn(copy_info.source_buffer.is_valid() == false, "invalid source buffer");
    GPUFailOn(copy_info.destination_buffer.is_valid() == false, "invalid destination buffer");
	current_adapter.command_buffer_copy_buffer(command_buffer, copy_info);
}

void GPU::command_buffer_bind_pipeline(CommandBufferID command_buffer, PipelineBindPoint bind_point, PipelineID pipeline)
{
	GPUFailOn(command_buffer.is_valid() == false, "invalid command buffer");
	GPUFailOn(bind_point == GPU::PipelineBindPoint::Unknown, "invalid bind point");
    GPUFailOn(pipeline.is_valid() == false, "invalid pipeline");
	current_adapter.command_buffer_bind_pipeline(command_buffer, bind_point, pipeline);
}

void GPU::command_buffer_bind_descriptor_sets(CommandBufferID command_buffer, PipelineBindPoint bind_point, PipelineLayoutID pipeline_layout, u32 base_set, const Slice<DescriptorSetID>& descriptor_sets)
{
    GPUFailOn(command_buffer.is_valid() == false, "invalid command buffer");
	GPUFailOn(bind_point == GPU::PipelineBindPoint::Unknown, "invalid bind point");
    GPUFailOn(pipeline_layout.is_valid() == false, "invalid pipeline layout");
    GPUFailOn(descriptor_sets.len == 0, "invalid descriptor set count");
	current_adapter.command_buffer_bind_descriptor_sets(command_buffer, bind_point, pipeline_layout, base_set, descriptor_sets);
}


void GPU::command_buffer_bind_vertex_buffers(CommandBufferID command_buffer, u32 base_binding, const Slice<BufferID>& buffers, const Slice<usize>& offsets)
{
    GPUFailOn(command_buffer.is_valid() == false, "invalid command buffer");
    GPUFailOn(buffers.len != offsets.len, "inconsistent buffers and offsets count");
	current_adapter.command_buffer_bind_vertex_buffers(command_buffer, base_binding, buffers, offsets);
}

void GPU::command_buffer_constant_block(GPU::CommandBufferID command_buffer, PipelineLayoutID pipeline_layout, GPU::ShaderStage stages, u32 offset, u32 size, MemoryAddress block_address)
{
	GPUFailOn(command_buffer.is_valid() == false, "invalid command buffer");
	GPUFailOn(pipeline_layout.is_valid() == false, "invalid pipeline layout");
	GPUFailOn(stages == GPU::ShaderStage(0), "invalid shader stages");
    GPUFailOn(size == 0, "invalid constant block size");
	current_adapter.command_buffer_constant_block(command_buffer, pipeline_layout, stages, offset, size, block_address);
}

void GPU::command_buffer_set_viewports(CommandBufferID command_buffer, u32 base_viewport, const Slice<Viewport>& viewports)
{
    GPUFailOn(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter.command_buffer_set_viewports(command_buffer, base_viewport, viewports);
}

void GPU::command_buffer_set_scissors(CommandBufferID command_buffer, u32 base_scissor, const Slice<Scissor>& scissors)
{
    GPUFailOn(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter.command_buffer_set_scissors(command_buffer, base_scissor, scissors);
}

void GPU::command_buffer_draw(GPU::CommandBufferID command_buffer, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance)
{
    GPUFailOn(command_buffer.is_valid() == false, "invalid command buffer");
	current_adapter.command_buffer_draw(command_buffer, vertex_count, instance_count, base_vertex, base_instance);
}

