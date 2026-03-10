#pragma once
#include "graphics/graphics.h"



namespace InternalGraphics
{

struct Adapter
{
	VTFunc(void, initialize, const mem::Allocator&);
	VTFunc(void, shutdown);

	VTFunc(Slice<Graphics::PhysicalDeviceID>, physical_devices_enumerate);
	VTFunc(Graphics::PhysicalDeviceInfo, physical_device_get_info, Graphics::PhysicalDeviceID physical_device);
	
	VTFunc(Graphics::SurfaceID, surface_create, const Graphics::SurfaceCreateInfo& ci);
	VTFunc(void, surface_destroy, Graphics::SurfaceID surface);

	VTFunc(Graphics::DeviceID, device_create, const Graphics::DeviceCreateInfo& ci);
	VTFunc(void, device_destroy, Graphics::DeviceID device);

	VTFunc(Graphics::SwapChainID, swap_chain_create, const Graphics::SwapChainCreateInfo& ci);
	VTFunc(void, swap_chain_destroy, Graphics::SwapChainID swap_chain);
	VTFunc(Graphics::TextureID, swap_chain_get_texture, Graphics::SwapChainID swap_chain, u32 image_index);
	VTFunc(void, swap_chain_acquire_next_image, Graphics::SwapChainID swap_chain, const Graphics::AcquireInfo& acquire_info, u32* image_index);

	VTFunc(Graphics::FenceID, fence_create, const Graphics::FenceCreateInfo& ci);
	VTFunc(void, fence_destroy, Graphics::FenceID fence);
	VTFunc(void, fence_reset, Slice<Graphics::FenceID> fences);
	VTFunc(void, fence_wait_for, Slice<Graphics::FenceID> fences, bool wait_for_all, u64 timeout);

	VTFunc(Graphics::SemaphoreID, semaphore_create, const Graphics::SemaphoreCreateInfo& ci);
	VTFunc(void, semaphore_destroy, Graphics::SemaphoreID semaphore);

	VTFunc(Graphics::QueueID, queue_create, const Graphics::QueueCreateInfo& ci);
	VTFunc(void, queue_destroy, Graphics::QueueID queue);
	VTFunc(void, queue_execute_command_buffer, Graphics::QueueID queue, const Graphics::QueueExecuteInfo& execute_info);
	VTFunc(void, queue_present, Graphics::QueueID queue, const Graphics::QueuePresentInfo& present_info);
	VTFunc(void, queue_wait_idle, Graphics::QueueID queue);

	VTFunc(Graphics::MemoryHeapID, memory_heap_create, const Graphics::MemoryHeapCreateInfo& ci);
	VTFunc(void, memory_heap_destroy, Graphics::MemoryHeapID memory_heap);

	VTFunc(Graphics::BufferID, buffer_create, const Graphics::BufferCreateInfo& ci);
	VTFunc(void, buffer_destroy, Graphics::BufferID buffer);
	VTFunc(Slice<u8>, buffer_map_memory, Graphics::BufferID buffer, usize offset, usize len);
	VTFunc(void, buffer_unmap_memory, Graphics::BufferID buffer, const Slice<u8>& memory);

	VTFunc(Graphics::TextureID, texture_create, const Graphics::TextureCreateInfo& ci);
	VTFunc(void, texture_destroy, Graphics::TextureID texture);
	VTFunc(Vector2I, texture_get_size, Graphics::TextureID texture);

	VTFunc(Graphics::RenderTargetID, render_target_create, const Graphics::RenderTargetCreateInfo& ci);
	VTFunc(void, render_target_destroy, Graphics::RenderTargetID render_target);
	VTFunc(Graphics::TextureID, render_target_get_texture, Graphics::RenderTargetID render_target);

	VTFunc(Graphics::DescriptorSetID, descriptor_set_create, const Graphics::DescriptorSetCreateInfo& ci);
	VTFunc(void, descriptor_set_destroy, Graphics::DescriptorSetID descriptor_set);
	VTFunc(void, descriptor_set_update_descriptors, Graphics::DescriptorSetID descriptor_set, const Graphics::UpdateDescriptorInfo& update_info);

	VTFunc(Graphics::PipelineID, pipeline_create, const Graphics::PipelineCreateInfo& ci);
	VTFunc(void, pipeline_destroy, Graphics::PipelineID pipeline);

	VTFunc(Graphics::CommandPoolID, command_pool_create, const Graphics::CommandPoolCreateInfo& ci);
	VTFunc(void, command_pool_destroy, Graphics::CommandPoolID command_pool);
	
	VTFunc(Graphics::CommandBufferID, command_buffer_allocate, const Graphics::CommandBufferAllocateInfo& ci);
	VTFunc(void, command_buffer_free, Graphics::CommandBufferID command_buffer);

	VTFunc(void, command_buffer_begin, Graphics::CommandBufferID command_buffer);
	VTFunc(void, command_buffer_end, Graphics::CommandBufferID command_buffer);
	VTFunc(void, command_buffer_begin_renderpass, Graphics::CommandBufferID command_buffer, const Graphics::RenderPassBeginInfo& begin_info);
	VTFunc(void, command_buffer_end_renderpass, Graphics::CommandBufferID command_buffer, const Graphics::RenderPassEndInfo& end_info);

	VTFunc(void, command_buffer_memory_barrier, Graphics::CommandBufferID command_buffer, const Graphics::PipelineMemoryBarrier& memory_barrier);
	VTFunc(void, command_buffer_buffer_barrier, Graphics::CommandBufferID command_buffer, const Graphics::PipelineBufferBarrier& buffer_barrier);
	VTFunc(void, command_buffer_texture_barrier, Graphics::CommandBufferID command_buffer, const Graphics::PipelineTextureBarrier& texture_barrier);

	VTFunc(void, command_buffer_copy_buffer, Graphics::CommandBufferID command_buffer, const Graphics::BufferCopyInfo& copy_info);

	VTFunc(void, command_buffer_bind_pipeline, Graphics::CommandBufferID command_buffer, Graphics::PipelineBindPoint bind_point, Graphics::PipelineID pipeline);
	VTFunc(void, command_buffer_bind_descriptor_sets, Graphics::CommandBufferID command_buffer, Graphics::PipelineBindPoint bind_point, u32 base_set, const Slice<Graphics::DescriptorSetID>& descriptor_sets);
	VTFunc(void, command_buffer_bind_vertex_buffers, Graphics::CommandBufferID command_buffer, u32 base_binding, const Slice<Graphics::BufferID>& buffers, const Slice<usize>& offsets);
	VTFunc(void, command_buffer_constant_block, Graphics::CommandBufferID command_buffer, Graphics::PipelineID pipeline, Graphics::ShaderStage stages, u32 offset, u32 size, MemoryAddress block_address);
	
	VTFunc(void, command_buffer_set_viewports, Graphics::CommandBufferID command_buffer, u32 base_viewport, const Slice<Graphics::Viewport>& viewports);
	VTFunc(void, command_buffer_set_scissors, Graphics::CommandBufferID command_buffer, u32 base_scissor, const Slice<Graphics::Scissor>& scissors);

	VTFunc(void, command_buffer_draw, Graphics::CommandBufferID command_buffer, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance);
};

}
