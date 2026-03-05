#include "graphics/graphics.h"

#include "graphics/adapter.h"
#include "graphics/vk/vk_driver.h"


static inline InternalGraphics::Adapter current_adapter;

void Graphics::initialize(const mem::Allocator& allocator)
{
	current_adapter = VulkanDriver::get_adapter();

	current_adapter.initialize(allocator);
}

void Graphics::shutdown()
{
	current_adapter.shutdown();
}

Slice<Graphics::PhysicalDeviceID> Graphics::physical_devices_enumerate()
{
	return current_adapter.physical_devices_enumerate();
}

Graphics::PhysicalDeviceInfo Graphics::physical_device_get_info(PhysicalDeviceID physical_device)
{
	return current_adapter.physical_device_get_info(physical_device);
}

Graphics::SurfaceID Graphics::surface_create(const Graphics::SurfaceCreateInfo& ci)
{
	return current_adapter.surface_create(ci);
}

void Graphics::surface_destroy(Graphics::SurfaceID surface)
{
	current_adapter.surface_destroy(surface);
}

Graphics::DeviceID Graphics::device_create(const DeviceCreateInfo &ci)
{
	return current_adapter.device_create(ci);
}

void Graphics::device_destroy(DeviceID device)
{
	current_adapter.device_destroy(device);
}

Graphics::SwapChainID Graphics::swap_chain_create(const SwapChainCreateInfo& ci)
{
	return current_adapter.swap_chain_create(ci);
}

void Graphics::swap_chain_destroy(SwapChainID swap_chain)
{
	current_adapter.swap_chain_destroy(swap_chain);
}

Graphics::TextureID Graphics::swap_chain_get_texture(SwapChainID swap_chain, u32 image_index)
{
	return current_adapter.swap_chain_get_texture(swap_chain, image_index);
}

void Graphics::swap_chain_acquire_next_image(SwapChainID swap_chain, const AcquireInfo& acquire_info, u32* image_index)
{
	current_adapter.swap_chain_acquire_next_image(swap_chain, acquire_info, image_index);
}

Graphics::FenceID Graphics::fence_create(const FenceCreateInfo &ci)
{
	return current_adapter.fence_create(ci);
}

void Graphics::fence_destroy(FenceID fence)
{
	current_adapter.fence_destroy(fence);
}

void Graphics::fence_reset(Slice<FenceID> fences)
{
	current_adapter.fence_reset(fences);
}

void Graphics::fence_wait_for(Slice<FenceID> fences, bool wait_for_all, u64 timeout)
{
	current_adapter.fence_wait_for(fences, wait_for_all, timeout);
}

Graphics::SemaphoreID Graphics::semaphore_create(const SemaphoreCreateInfo &ci)
{
	return current_adapter.semaphore_create(ci);
}

void Graphics::semaphore_destroy(SemaphoreID semaphore)
{
	current_adapter.semaphore_destroy(semaphore);
}

Graphics::QueueID Graphics::queue_create(const QueueCreateInfo& ci)
{
	return current_adapter.queue_create(ci);
}

void Graphics::queue_destroy(QueueID queue)
{
	current_adapter.queue_destroy(queue);
}

void Graphics::queue_execute_command_buffer(QueueID queue, const QueueExecuteInfo& execute_info)
{
	current_adapter.queue_execute_command_buffer(queue, execute_info);
}

void Graphics::queue_present(QueueID queue, const QueuePresentInfo& present_info)
{
	current_adapter.queue_present(queue, present_info);
}

void Graphics::queue_wait_idle(QueueID queue)
{
	current_adapter.queue_wait_idle(queue);
}

Graphics::BufferID Graphics::buffer_create(const BufferCreateInfo& ci)
{
	return current_adapter.buffer_create(ci);
}

void Graphics::buffer_destroy(BufferID buffer)
{
	current_adapter.buffer_destroy(buffer);
}

Slice<u8> Graphics::buffer_map_memory(Graphics::BufferID buffer, usize offset, usize len)
{
	return current_adapter.buffer_map_memory(buffer, offset, len);
}

void Graphics::buffer_unmap_memory(Graphics::BufferID buffer, const Slice<u8>& memory)
{
	current_adapter.buffer_unmap_memory(buffer, memory);
}


Graphics::TextureID Graphics::texture_create(const TextureCreateInfo& ci)
{
	return current_adapter.texture_create(ci);
}

void Graphics::texture_destroy(TextureID texture)
{
	current_adapter.texture_destroy(texture);
}

Vector2I Graphics::texture_get_size(TextureID texture)
{
	return current_adapter.texture_get_size(texture);
}

Graphics::RenderTargetID Graphics::render_target_create(const RenderTargetCreateInfo& ci)
{
	return current_adapter.render_target_create(ci);
}

void Graphics::render_target_destroy(RenderTargetID render_target)
{
	current_adapter.render_target_destroy(render_target);
}

Graphics::TextureID Graphics::render_target_get_texture(RenderTargetID render_target)
{
	return current_adapter.render_target_get_texture(render_target);
}


Graphics::PipelineID Graphics::pipeline_create(const Graphics::PipelineCreateInfo& ci)
{
	return current_adapter.pipeline_create(ci);
}

void Graphics::pipeline_destroy(PipelineID pipeline)
{
	current_adapter.pipeline_destroy(pipeline);
}

Graphics::CommandPoolID Graphics::command_pool_create(const CommandPoolCreateInfo &ci)
{
	return current_adapter.command_pool_create(ci);
}

void Graphics::command_pool_destroy(CommandPoolID command_pool)
{
	return current_adapter.command_pool_destroy(command_pool);
}

Graphics::CommandBufferID Graphics::command_buffer_allocate(const CommandBufferAllocateInfo& ci)
{
	return current_adapter.command_buffer_allocate(ci);
}

void Graphics::command_buffer_free(CommandBufferID cmd)
{
	current_adapter.command_buffer_free(cmd);
}

void Graphics::command_buffer_begin(CommandBufferID cmd)
{
	current_adapter.command_buffer_begin(cmd);
}

void Graphics::command_buffer_end(CommandBufferID cmd)
{
	current_adapter.command_buffer_end(cmd);
}

void Graphics::command_buffer_begin_renderpass(CommandBufferID cmd, const RenderPassBeginInfo& begin_info)
{
	current_adapter.command_buffer_begin_renderpass(cmd, begin_info);
}

void Graphics::command_buffer_end_renderpass(CommandBufferID cmd, const RenderPassEndInfo& end_info)
{
	current_adapter.command_buffer_end_renderpass(cmd, end_info);
}

void Graphics::command_buffer_memory_barrier(CommandBufferID command_buffer, const PipelineMemoryBarrier& memory_barrier)
{
	current_adapter.command_buffer_memory_barrier(command_buffer, memory_barrier);
}

void Graphics::command_buffer_buffer_barrier(CommandBufferID command_buffer, const PipelineBufferBarrier& buffer_barrier)
{
	current_adapter.command_buffer_buffer_barrier(command_buffer, buffer_barrier);
}

void Graphics::command_buffer_texture_barrier(CommandBufferID command_buffer, const PipelineTextureBarrier& texture_barrier)
{
	current_adapter.command_buffer_texture_barrier(command_buffer, texture_barrier);
}

void Graphics::command_buffer_blit_framebuffer(CommandBufferID cmd, RenderTargetID src_render_target, RenderTargetID dst_render_target, Rect2DI src_rect, Rect2DI dst_rect, TextureFilter filter)
{
	current_adapter.command_buffer_blit_framebuffer(cmd, src_render_target, dst_render_target, src_rect, dst_rect, filter);
}

void Graphics::command_buffer_bind_vertex_buffers(CommandBufferID cmd, u32 binding, const Slice<BufferID>& buffers, const Slice<u32>& offsets, const Slice<u32>& strides)
{
	current_adapter.command_buffer_bind_vertex_buffers(cmd, binding, buffers, offsets, strides);
}

void Graphics::command_buffer_bind_index_buffer(Graphics::CommandBufferID cmd, Graphics::BufferID index_buffer, u32 offset, Graphics::IndexType index_type)
{
	current_adapter.command_buffer_bind_index_buffer(cmd, index_buffer, offset, index_type);
}

void Graphics::command_buffer_bind_pipeline(CommandBufferID cmd, PipelineID pipeline)
{
	current_adapter.command_buffer_bind_pipeline(cmd, pipeline);
}

void Graphics::command_buffer_bind_render_target(CommandBufferID cmd, RenderTargetID render_target)
{
	current_adapter.command_buffer_bind_render_target(cmd, render_target);
}

void Graphics::command_buffer_set_texture_unit(CommandBufferID cmd, u32 set, u32 base_slot, const Slice<TextureID>& textures)
{
	current_adapter.command_buffer_set_texture_unit(cmd, set, base_slot, textures);
}

void Graphics::command_buffer_set_uniform(CommandBufferID cmd, u32 set, u32 base_slot, const Slice<BufferID>& buffers)
{
	current_adapter.command_buffer_set_uniform(cmd, set, base_slot, buffers);
}

void Graphics::command_buffer_set_viewport(CommandBufferID cmd, Rect2DI viewport_rect)
{
	current_adapter.command_buffer_set_viewport(cmd, viewport_rect);
}

void Graphics::command_buffer_clear(CommandBufferID cmd, RenderTargetID render_target, Color clear_color)
{
	current_adapter.command_buffer_clear(cmd, render_target, clear_color);
}

void Graphics::command_buffer_draw(Graphics::CommandBufferID cmd, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance)
{
	current_adapter.command_buffer_draw(cmd, vertex_count, instance_count, base_vertex, base_instance);
}

void Graphics::command_buffer_draw_indexed(Graphics::CommandBufferID cmd, u32 index_count, u32 instance_count, u32 base_index, u32 base_vertex, u32 base_instance)
{
	current_adapter.command_buffer_draw_indexed(cmd, index_count, instance_count, base_index, base_vertex, base_instance);
}

