#include "gpu/gpu.h"

#include "gpu/gpu_adapter.h"
#include "gpu/vk/vk_driver.h"


static inline InternalGPU::GPUAdapter current_adapter;

void GPU::initialize(const mem::Allocator& allocator)
{
	current_adapter = VulkanDriver::get_adapter();

	current_adapter.initialize(allocator);
}

void GPU::shutdown()
{
	current_adapter.shutdown();
}

Slice<GPU::PhysicalDeviceID> GPU::physical_devices_enumerate()
{
	return current_adapter.physical_devices_enumerate();
}

GPU::PhysicalDeviceInfo GPU::physical_device_get_info(PhysicalDeviceID physical_device)
{
	return current_adapter.physical_device_get_info(physical_device);
}

GPU::SurfaceID GPU::surface_create(const GPU::SurfaceCreateInfo& ci)
{
	return current_adapter.surface_create(ci);
}

void GPU::surface_destroy(GPU::SurfaceID surface)
{
	current_adapter.surface_destroy(surface);
}

GPU::DeviceID GPU::device_create(const DeviceCreateInfo &ci)
{
	return current_adapter.device_create(ci);
}

void GPU::device_destroy(DeviceID device)
{
	current_adapter.device_destroy(device);
}

GPU::SwapChainID GPU::swap_chain_create(const SwapChainCreateInfo& ci)
{
	return current_adapter.swap_chain_create(ci);
}

void GPU::swap_chain_destroy(SwapChainID swap_chain)
{
	current_adapter.swap_chain_destroy(swap_chain);
}

u32 GPU::swap_chain_get_image_count(SwapChainID swap_chain)
{
	return current_adapter.swap_chain_get_image_count(swap_chain);
}

GPU::TextureID GPU::swap_chain_get_texture(SwapChainID swap_chain, u32 image_index)
{
	return current_adapter.swap_chain_get_texture(swap_chain, image_index);
}

void GPU::swap_chain_acquire_next_image(SwapChainID swap_chain, const AcquireInfo& acquire_info, u32* image_index)
{
	current_adapter.swap_chain_acquire_next_image(swap_chain, acquire_info, image_index);
}

GPU::FenceID GPU::fence_create(const FenceCreateInfo &ci)
{
	return current_adapter.fence_create(ci);
}

void GPU::fence_destroy(FenceID fence)
{
	current_adapter.fence_destroy(fence);
}

void GPU::fence_reset(Slice<FenceID> fences)
{
	current_adapter.fence_reset(fences);
}

void GPU::fence_wait_for(Slice<FenceID> fences, bool wait_for_all, u64 timeout)
{
	current_adapter.fence_wait_for(fences, wait_for_all, timeout);
}

GPU::SemaphoreID GPU::semaphore_create(const SemaphoreCreateInfo &ci)
{
	return current_adapter.semaphore_create(ci);
}

void GPU::semaphore_destroy(SemaphoreID semaphore)
{
	current_adapter.semaphore_destroy(semaphore);
}

GPU::QueueID GPU::queue_create(const QueueCreateInfo& ci)
{
	return current_adapter.queue_create(ci);
}

void GPU::queue_destroy(QueueID queue)
{
	current_adapter.queue_destroy(queue);
}

void GPU::queue_execute_command_buffer(QueueID queue, const QueueExecuteInfo& execute_info)
{
	current_adapter.queue_execute_command_buffer(queue, execute_info);
}

void GPU::queue_present(QueueID queue, const QueuePresentInfo& present_info)
{
	current_adapter.queue_present(queue, present_info);
}

void GPU::queue_wait_idle(QueueID queue)
{
	current_adapter.queue_wait_idle(queue);
}

GPU::MemoryHeapID GPU::memory_heap_create(const MemoryHeapCreateInfo& ci)
{
	return current_adapter.memory_heap_create(ci);
}

void GPU::memory_heap_destroy(MemoryHeapID memory_heap)
{
	current_adapter.memory_heap_destroy(memory_heap);
}

GPU::BufferID GPU::buffer_create(const BufferCreateInfo& ci)
{
	return current_adapter.buffer_create(ci);
}

void GPU::buffer_destroy(BufferID buffer)
{
	current_adapter.buffer_destroy(buffer);
}

Slice<u8> GPU::buffer_map_memory(GPU::BufferID buffer, usize offset, usize len)
{
	return current_adapter.buffer_map_memory(buffer, offset, len);
}

void GPU::buffer_unmap_memory(GPU::BufferID buffer, const Slice<u8>& memory)
{
	current_adapter.buffer_unmap_memory(buffer, memory);
}

GPU::SamplerID GPU::sampler_create(const SamplerCreateInfo& ci)
{
	return current_adapter.sampler_create(ci);
}

void GPU::sampler_destroy(SamplerID sampler)
{
	current_adapter.sampler_destroy(sampler);
}

GPU::TextureID GPU::texture_create(const TextureCreateInfo& ci)
{
	return current_adapter.texture_create(ci);
}

void GPU::texture_destroy(TextureID texture)
{
	current_adapter.texture_destroy(texture);
}

Vector2I GPU::texture_get_size(TextureID texture)
{
	return current_adapter.texture_get_size(texture);
}

GPU::RenderTargetID GPU::render_target_create(const RenderTargetCreateInfo& ci)
{
	return current_adapter.render_target_create(ci);
}

void GPU::render_target_destroy(RenderTargetID render_target)
{
	current_adapter.render_target_destroy(render_target);
}

GPU::TextureID GPU::render_target_get_texture(RenderTargetID render_target)
{
	return current_adapter.render_target_get_texture(render_target);
}

GPU::DescriptorSetLayoutID GPU::descriptor_set_layout_create(const DescriptorSetLayoutCreateInfo& ci)
{
	return current_adapter.descriptor_set_layout_create(ci);
}

void GPU::descriptor_set_layout_destroy(DescriptorSetLayoutID descriptor_set_layout)
{
	current_adapter.descriptor_set_layout_destroy(descriptor_set_layout);
}

GPU::DescriptorSetID GPU::descriptor_set_create(const GPU::DescriptorSetCreateInfo& ci)
{
	return current_adapter.descriptor_set_create(ci);
}

void GPU::descriptor_set_destroy(GPU::DescriptorSetID descriptor_set)
{
	current_adapter.descriptor_set_destroy(descriptor_set);
}

void GPU::descriptor_set_update_descriptors(DescriptorSetID descriptor_set, const UpdateDescriptorInfo& update_info)
{
	current_adapter.descriptor_set_update_descriptors(descriptor_set, update_info);
}

GPU::PipelineID GPU::pipeline_create(const GPU::PipelineCreateInfo& ci)
{
	return current_adapter.pipeline_create(ci);
}

void GPU::pipeline_destroy(PipelineID pipeline)
{
	current_adapter.pipeline_destroy(pipeline);
}

GPU::CommandPoolID GPU::command_pool_create(const CommandPoolCreateInfo &ci)
{
	return current_adapter.command_pool_create(ci);
}

void GPU::command_pool_destroy(CommandPoolID command_pool)
{
	return current_adapter.command_pool_destroy(command_pool);
}

GPU::CommandBufferID GPU::command_buffer_allocate(const CommandBufferAllocateInfo& ci)
{
	return current_adapter.command_buffer_allocate(ci);
}

void GPU::command_buffer_free(CommandBufferID cmd)
{
	current_adapter.command_buffer_free(cmd);
}

void GPU::command_buffer_begin(CommandBufferID cmd)
{
	current_adapter.command_buffer_begin(cmd);
}

void GPU::command_buffer_end(CommandBufferID cmd)
{
	current_adapter.command_buffer_end(cmd);
}

void GPU::command_buffer_begin_renderpass(CommandBufferID cmd, const RenderPassBeginInfo& begin_info)
{
	current_adapter.command_buffer_begin_renderpass(cmd, begin_info);
}

void GPU::command_buffer_end_renderpass(CommandBufferID cmd, const RenderPassEndInfo& end_info)
{
	current_adapter.command_buffer_end_renderpass(cmd, end_info);
}

void GPU::command_buffer_memory_barrier(CommandBufferID command_buffer, const PipelineMemoryBarrier& memory_barrier)
{
	current_adapter.command_buffer_memory_barrier(command_buffer, memory_barrier);
}

void GPU::command_buffer_buffer_barrier(CommandBufferID command_buffer, const PipelineBufferBarrier& buffer_barrier)
{
	current_adapter.command_buffer_buffer_barrier(command_buffer, buffer_barrier);
}

void GPU::command_buffer_texture_barrier(CommandBufferID command_buffer, const PipelineTextureBarrier& texture_barrier)
{
	current_adapter.command_buffer_texture_barrier(command_buffer, texture_barrier);
}

void GPU::command_buffer_copy_buffer_to_texture(CommandBufferID command_buffer, const CopyBufferToTextureInfo& copy_info)
{
	current_adapter.command_buffer_copy_buffer_to_texture(command_buffer, copy_info);
}

void GPU::command_buffer_copy_buffer(CommandBufferID command_buffer, const BufferCopyInfo& copy_info)
{
	current_adapter.command_buffer_copy_buffer(command_buffer, copy_info);
}

void GPU::command_buffer_bind_pipeline(CommandBufferID command_buffer, PipelineBindPoint bind_point, PipelineID pipeline)
{
	current_adapter.command_buffer_bind_pipeline(command_buffer, bind_point, pipeline);
}

void GPU::command_buffer_bind_descriptor_sets(CommandBufferID command_buffer, PipelineBindPoint bind_point, u32 base_set, const Slice<DescriptorSetID>& descriptor_sets)
{
	current_adapter.command_buffer_bind_descriptor_sets(command_buffer, bind_point, base_set, descriptor_sets);
}


void GPU::command_buffer_bind_vertex_buffers(CommandBufferID command_buffer, u32 base_binding, const Slice<BufferID>& buffers, const Slice<usize>& offsets)
{
	current_adapter.command_buffer_bind_vertex_buffers(command_buffer, base_binding, buffers, offsets);
}

void GPU::command_buffer_constant_block(GPU::CommandBufferID command_buffer, GPU::PipelineID pipeline, GPU::ShaderStage stage, u32 offset, u32 size, MemoryAddress block_address)
{
	current_adapter.command_buffer_constant_block(command_buffer, pipeline, stage, offset, size, block_address);
}

void GPU::command_buffer_set_viewports(CommandBufferID command_buffer, u32 base_viewport, const Slice<Viewport>& viewports)
{
	current_adapter.command_buffer_set_viewports(command_buffer, base_viewport, viewports);
}

void GPU::command_buffer_set_scissors(CommandBufferID command_buffer, u32 base_scissor, const Slice<Scissor>& scissors)
{
	current_adapter.command_buffer_set_scissors(command_buffer, base_scissor, scissors);
}

void GPU::command_buffer_draw(GPU::CommandBufferID cmd, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance)
{
	current_adapter.command_buffer_draw(cmd, vertex_count, instance_count, base_vertex, base_instance);
}

