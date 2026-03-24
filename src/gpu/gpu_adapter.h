#pragma once
#include "gpu/gpu.h"


namespace InternalGPU
{

struct GPUAdapter
{
	VTFunc(void, initialize, const mem::Allocator&);
	VTFunc(void, shutdown);

	VTFunc(Slice<GPU::PhysicalDeviceID>, physical_devices_enumerate);
	VTFunc(GPU::PhysicalDeviceInfo, physical_device_get_info, GPU::PhysicalDeviceID physical_device);
	
	VTFunc(GPU::SurfaceID, surface_create, const GPU::SurfaceCreateInfo& ci);
	VTFunc(void, surface_destroy, GPU::SurfaceID surface);

	VTFunc(GPU::DeviceID, device_create, const GPU::DeviceCreateInfo& ci);
	VTFunc(void, device_destroy, GPU::DeviceID device);

	VTFunc(GPU::SwapChainID, swap_chain_create, const GPU::SwapChainCreateInfo& ci);
	VTFunc(void, swap_chain_destroy, GPU::SwapChainID swap_chain);
	VTFunc(u32, swap_chain_get_image_count, GPU::SwapChainID swap_chain);
	VTFunc(GPU::TextureID, swap_chain_get_texture, GPU::SwapChainID swap_chain, u32 image_index);
	VTFunc(GPU::AcquireResult, swap_chain_acquire_next_image, GPU::SwapChainID swap_chain, const GPU::AcquireInfo& acquire_info, u32* image_index);

	VTFunc(GPU::FenceID, fence_create, const GPU::FenceCreateInfo& ci);
	VTFunc(void, fence_destroy, GPU::FenceID fence);
	VTFunc(void, fence_reset, Slice<GPU::FenceID> fences);
	VTFunc(void, fence_wait_for, Slice<GPU::FenceID> fences, bool wait_for_all, u64 timeout);

	VTFunc(GPU::SemaphoreID, semaphore_create, const GPU::SemaphoreCreateInfo& ci);
	VTFunc(void, semaphore_destroy, GPU::SemaphoreID semaphore);

	VTFunc(GPU::QueueID, queue_create, const GPU::QueueCreateInfo& ci);
	VTFunc(void, queue_destroy, GPU::QueueID queue);
	VTFunc(void, queue_execute_command_buffer, GPU::QueueID queue, const GPU::QueueExecuteInfo& execute_info);
	VTFunc(void, queue_present, GPU::QueueID queue, const GPU::QueuePresentInfo& present_info);
	VTFunc(void, queue_wait_idle, GPU::QueueID queue);

	VTFunc(GPU::MemoryHeapID, memory_heap_create, const GPU::MemoryHeapCreateInfo& ci);
	VTFunc(void, memory_heap_destroy, GPU::MemoryHeapID memory_heap);

	VTFunc(GPU::BufferID, buffer_create, const GPU::BufferCreateInfo& ci);
	VTFunc(void, buffer_destroy, GPU::BufferID buffer);
	VTFunc(Slice<u8>, buffer_map_memory, GPU::BufferID buffer, usize offset, usize len);
	VTFunc(void, buffer_unmap_memory, GPU::BufferID buffer, const Slice<u8>& memory);

	VTFunc(GPU::SamplerID, sampler_create, const GPU::SamplerCreateInfo& ci);
	VTFunc(void, sampler_destroy, GPU::SamplerID sampler);

	VTFunc(GPU::TextureID, texture_create, const GPU::TextureCreateInfo& ci);
	VTFunc(void, texture_destroy, GPU::TextureID texture);
	VTFunc(Vector2I, texture_get_size, GPU::TextureID texture);

	VTFunc(GPU::RenderTargetID, render_target_create, const GPU::RenderTargetCreateInfo& ci);
	VTFunc(void, render_target_destroy, GPU::RenderTargetID render_target);
	VTFunc(GPU::TextureID, render_target_get_texture, GPU::RenderTargetID render_target);

	VTFunc(GPU::DescriptorSetLayoutID, descriptor_set_layout_create, const GPU::DescriptorSetLayoutCreateInfo& ci);
	VTFunc(void, descriptor_set_layout_destroy, GPU::DescriptorSetLayoutID descriptor_set_layout);

	VTFunc(GPU::DescriptorPoolID, descriptor_pool_create, const GPU::DescriptorPoolCreateInfo& ci);
	VTFunc(void, descriptor_pool_destroy, GPU::DescriptorPoolID descriptor_pool);

	VTFunc(GPU::DescriptorSetID, descriptor_set_allocate, const GPU::DescriptorSetAllocateInfo& ci);
	VTFunc(void, descriptor_set_free, GPU::DescriptorSetID descriptor_set);
	VTFunc(void, descriptor_set_update_descriptors, GPU::DescriptorSetID descriptor_set, const GPU::UpdateDescriptorInfo& update_info);

	VTFunc(GPU::PipelineID, pipeline_create, const GPU::PipelineCreateInfo& ci);
	VTFunc(void, pipeline_destroy, GPU::PipelineID pipeline);

	VTFunc(GPU::CommandPoolID, command_pool_create, const GPU::CommandPoolCreateInfo& ci);
	VTFunc(void, command_pool_destroy, GPU::CommandPoolID command_pool);
	
	VTFunc(GPU::CommandBufferID, command_buffer_allocate, const GPU::CommandBufferAllocateInfo& ci);
	VTFunc(void, command_buffer_free, GPU::CommandBufferID command_buffer);

	VTFunc(void, command_buffer_begin, GPU::CommandBufferID command_buffer);
	VTFunc(void, command_buffer_end, GPU::CommandBufferID command_buffer);
	VTFunc(void, command_buffer_begin_renderpass, GPU::CommandBufferID command_buffer, const GPU::RenderPassBeginInfo& begin_info);
	VTFunc(void, command_buffer_end_renderpass, GPU::CommandBufferID command_buffer, const GPU::RenderPassEndInfo& end_info);

	VTFunc(void, command_buffer_memory_barrier, GPU::CommandBufferID command_buffer, const GPU::PipelineMemoryBarrier& memory_barrier);
	VTFunc(void, command_buffer_buffer_barrier, GPU::CommandBufferID command_buffer, const GPU::PipelineBufferBarrier& buffer_barrier);
	VTFunc(void, command_buffer_texture_barrier, GPU::CommandBufferID command_buffer, const GPU::PipelineTextureBarrier& texture_barrier);

	VTFunc(void, command_buffer_copy_buffer_to_texture, GPU::CommandBufferID command_buffer, const GPU::CopyBufferToTextureInfo& copy_info);
	VTFunc(void, command_buffer_copy_buffer, GPU::CommandBufferID command_buffer, const GPU::BufferCopyInfo& copy_info);

	VTFunc(void, command_buffer_bind_pipeline, GPU::CommandBufferID command_buffer, GPU::PipelineBindPoint bind_point, GPU::PipelineID pipeline);
	VTFunc(void, command_buffer_bind_descriptor_sets, GPU::CommandBufferID command_buffer, GPU::PipelineBindPoint bind_point, u32 base_set, const Slice<GPU::DescriptorSetID>& descriptor_sets);
	VTFunc(void, command_buffer_bind_vertex_buffers, GPU::CommandBufferID command_buffer, u32 base_binding, const Slice<GPU::BufferID>& buffers, const Slice<usize>& offsets);
	VTFunc(void, command_buffer_constant_block, GPU::CommandBufferID command_buffer, GPU::PipelineID pipeline, GPU::ShaderStage stages, u32 offset, u32 size, MemoryAddress block_address);
	
	VTFunc(void, command_buffer_set_viewports, GPU::CommandBufferID command_buffer, u32 base_viewport, const Slice<GPU::Viewport>& viewports);
	VTFunc(void, command_buffer_set_scissors, GPU::CommandBufferID command_buffer, u32 base_scissor, const Slice<GPU::Scissor>& scissors);

	VTFunc(void, command_buffer_draw, GPU::CommandBufferID command_buffer, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance);
};

}
