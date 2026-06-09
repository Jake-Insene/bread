#pragma once
#include "gpu/gpu.h"


namespace InternalGPU
{

struct GPUAdapter
{
	virtual void initialize(Mem::Allocator* allocator) = 0;
	virtual void shutdown() = 0;

	virtual Slice<GPU::PhysicalDeviceID> physical_devices_enumerate() = 0;
	virtual GPU::PhysicalDeviceInfo physical_device_get_info(GPU::PhysicalDeviceID physical_device) = 0;

	virtual GPU::SurfaceID surface_create(const GPU::SurfaceCreateInfo& ci) = 0;
	virtual void surface_destroy(GPU::SurfaceID surface) = 0;

	virtual GPU::DeviceID device_create(const GPU::DeviceCreateInfo& ci) = 0;
	virtual void device_destroy(GPU::DeviceID device) = 0;

	virtual GPU::SwapChainID swap_chain_create(const GPU::SwapChainCreateInfo& ci) = 0;
	virtual void swap_chain_destroy(GPU::SwapChainID swap_chain) = 0;
	virtual u32 swap_chain_get_image_count(GPU::SwapChainID swap_chain) = 0;
	virtual GPU::TextureID swap_chain_get_image(GPU::SwapChainID swap_chain, u32 image_index) = 0;
	virtual GPU::TextureViewID swap_chain_get_image_view(GPU::SwapChainID swap_chain, u32 image_index) = 0;
	virtual GPU::AcquireResult swap_chain_acquire_next_image(GPU::SwapChainID swap_chain, const GPU::AcquireInfo& acquire_info, u32* image_index) = 0;

	virtual GPU::FenceID fence_create(const GPU::FenceCreateInfo& ci) = 0;
	virtual void fence_destroy(GPU::FenceID fence) = 0;
	virtual bool fence_get_state(GPU::FenceID fence) = 0;
	virtual void fence_reset(Slice<GPU::FenceID> fences) = 0;
	virtual void fence_wait_for(Slice<GPU::FenceID> fences, bool wait_for_all, u64 timeout) = 0;

	virtual GPU::SemaphoreID semaphore_create(const GPU::SemaphoreCreateInfo& ci) = 0;
	virtual void semaphore_destroy(GPU::SemaphoreID semaphore) = 0;

	virtual u32 queue_get_count(const GPU::QueueGetCountInfo& gci) = 0;
	virtual GPU::QueueID queue_get(const GPU::QueueGetInfo& gi) = 0;
	virtual void queue_execute_command_buffer(GPU::QueueID queue, const GPU::QueueExecuteInfo& execute_info) = 0;
	virtual GPU::AcquireResult queue_present(GPU::QueueID queue, const GPU::QueuePresentInfo& present_info) = 0;
	virtual void queue_wait_idle(GPU::QueueID queue) = 0;

	virtual GPU::MemoryHeapID memory_heap_create(const GPU::MemoryHeapCreateInfo& ci) = 0;
	virtual void memory_heap_destroy(GPU::MemoryHeapID memory_heap) = 0;
	virtual Slice<u8> memory_heap_map(GPU::MemoryHeapID memory_heap, usize offset, usize len) = 0;
	virtual void memory_heap_unmap(GPU::MemoryHeapID memory_heap, const Slice<u8>& memory) = 0;

	virtual GPU::BufferID buffer_create(const GPU::BufferCreateInfo& ci) = 0;
	virtual void buffer_destroy(GPU::BufferID buffer) = 0;
	virtual GPU::MemoryRequirements buffer_get_memory_requirements(GPU::BufferID buffer) = 0;
	virtual void buffer_bind_memory_heap(GPU::BufferID buffer, const GPU::BindMemoryInfo& bind_info) = 0;

	virtual GPU::SamplerID sampler_create(const GPU::SamplerCreateInfo& ci) = 0;
	virtual void sampler_destroy(GPU::SamplerID sampler) = 0;

	virtual GPU::TextureID texture_create(const GPU::TextureCreateInfo& ci) = 0;
	virtual void texture_destroy(GPU::TextureID texture) = 0;
	virtual GPU::MemoryRequirements texture_get_memory_requirements(GPU::TextureID texture) = 0;
	virtual void texture_bind_memory_heap(GPU::TextureID texture, const GPU::BindMemoryInfo& bind_info) = 0;

	virtual GPU::TextureViewID texture_view_create(const GPU::TextureViewCreateInfo& ci) = 0;
	virtual void texture_view_destroy(GPU::TextureViewID texture_view) = 0;

	virtual GPU::DescriptorSetLayoutID descriptor_set_layout_create(const GPU::DescriptorSetLayoutCreateInfo& ci) = 0;
	virtual void descriptor_set_layout_destroy(GPU::DescriptorSetLayoutID descriptor_set_layout) = 0;

	virtual GPU::DescriptorPoolID descriptor_pool_create(const GPU::DescriptorPoolCreateInfo& ci) = 0;
	virtual void descriptor_pool_destroy(GPU::DescriptorPoolID descriptor_pool) = 0;

	virtual void descriptor_set_allocate(const GPU::DescriptorSetAllocateInfo& ci, Slice<GPU::DescriptorSetID> out_descriptor_sets) = 0;
	virtual void descriptor_set_free(GPU::DescriptorPoolID descriptor_pool, const Slice<const GPU::DescriptorSetID>& descriptor_sets) = 0;
	virtual void descriptor_set_update_descriptors(const GPU::UpdateDescriptorInfo& update_info) = 0;

	virtual GPU::PipelineLayoutID pipeline_layout_create(const GPU::PipelineLayoutCreateInfo& ci) = 0;
	virtual void pipeline_layout_destroy(GPU::PipelineLayoutID pipeline_layout) = 0;

	virtual GPU::PipelineID pipeline_create(const GPU::PipelineCreateInfo& ci) = 0;
	virtual void pipeline_destroy(GPU::PipelineID pipeline) = 0;

	virtual GPU::CommandPoolID command_pool_create(const GPU::CommandPoolCreateInfo& ci) = 0;
	virtual void command_pool_destroy(GPU::CommandPoolID command_pool) = 0;
		
	virtual GPU::CommandBufferID command_buffer_allocate(const GPU::CommandBufferAllocateInfo& ci) = 0;
	virtual void command_buffer_free(GPU::CommandBufferID command_buffer) = 0;

	virtual void command_buffer_begin(GPU::CommandBufferID command_buffer) = 0;
	virtual void command_buffer_end(GPU::CommandBufferID command_buffer) = 0;
	
	virtual void command_buffer_begin_renderpass(GPU::CommandBufferID command_buffer, const GPU::RenderPassBeginInfo& begin_info) = 0;
	virtual void command_buffer_end_renderpass(GPU::CommandBufferID command_buffer, const GPU::RenderPassEndInfo& end_info) = 0;

	virtual void command_buffer_pipeline_barrier(GPU::CommandBufferID command_buffer, const GPU::PipelineBarrier& pipeline_barrier) = 0;

	virtual void command_buffer_copy_buffer_to_texture(GPU::CommandBufferID command_buffer, const GPU::CopyBufferToTextureInfo& copy_info) = 0;
	virtual void command_buffer_copy_buffer(GPU::CommandBufferID command_buffer, const GPU::CopyBufferInfo& copy_info) = 0;

	virtual void command_buffer_bind_pipeline(GPU::CommandBufferID command_buffer, GPU::PipelineBindPoint bind_point, GPU::PipelineID pipeline) = 0;
	virtual void command_buffer_bind_descriptor_sets(GPU::CommandBufferID command_buffer, GPU::PipelineBindPoint bind_point, GPU::PipelineLayoutID pipeline_layout, u32 base_set, const Slice<GPU::DescriptorSetID>& descriptor_sets) = 0;
	virtual void command_buffer_bind_vertex_buffers(GPU::CommandBufferID command_buffer, u32 base_binding, const Slice<GPU::BufferID>& buffers, const Slice<usize>& offsets) = 0;
	virtual void command_buffer_constant_block(GPU::CommandBufferID command_buffer, GPU::PipelineLayoutID pipeline_layout, GPU::ShaderStage stages, u32 offset, u32 size, MemoryAddress block_address) = 0;
		
	virtual void command_buffer_set_viewports(GPU::CommandBufferID command_buffer, u32 base_viewport, const Slice<const GPU::Viewport>& viewports) = 0;
	virtual void command_buffer_set_scissors(GPU::CommandBufferID command_buffer, u32 base_scissor, const Slice<const GPU::Scissor>& scissors) = 0;

	virtual void command_buffer_draw(GPU::CommandBufferID command_buffer, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance) = 0;
};

}
