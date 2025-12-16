#include "graphics/graphics.h"

#include "graphics/adapter.h"
#include "graphics/gles/gles_driver.h"


static inline Adapter current_adapter;

void Graphics::initialize(const mem::Allocator& allocator)
{
	current_adapter = GLESDriver::get_adapter();

	current_adapter.initialize(allocator);
}

void Graphics::shutdown()
{
	current_adapter.shutdown();
}

Graphics::SwapChainID Graphics::swap_chain_create(const SwapChainCreateInfo& ci)
{
	return current_adapter.swap_chain_create(ci);
}

void Graphics::swap_chain_destroy(SwapChainID swap_chain)
{
	current_adapter.swap_chain_destroy(swap_chain);
}

Graphics::RenderTargetID Graphics::swap_chain_get_render_target(SwapChainID swap_chain, usize render_target_index)
{
	return current_adapter.swap_chain_get_render_target(swap_chain, render_target_index);
}

void Graphics::swap_chain_present(SwapChainID swap_chain, usize render_target_index)
{
	current_adapter.swap_chain_present(swap_chain, render_target_index);
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

Graphics::PipelineID Graphics::pipeline_create(const Graphics::PipelineCreateInfo& ci)
{
	return current_adapter.pipeline_create(ci);
}

void Graphics::pipeline_destroy(PipelineID pipeline)
{
	current_adapter.pipeline_destroy(pipeline);
}

Graphics::ProgramID Graphics::program_create(const ProgramCreateInfo& ci)
{
	return current_adapter.program_create(ci);
}

void Graphics::program_destroy(ProgramID pipeline)
{
	current_adapter.program_destroy(pipeline);
}

Graphics::CommandBufferID Graphics::command_buffer_create(const CommandBufferCreateInfo& ci)
{
	return current_adapter.command_buffer_create(ci);
}

void Graphics::command_buffer_destroy(CommandBufferID cmd)
{
	current_adapter.command_buffer_destroy(cmd);
}

void Graphics::command_buffer_begin(CommandBufferID cmd)
{
	current_adapter.command_buffer_begin(cmd);
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

void Graphics::command_buffer_end(CommandBufferID cmd)
{
	current_adapter.command_buffer_end(cmd);
}

Graphics::QueueID Graphics::queue_create(const QueueCreateInfo& ci)
{
	return current_adapter.queue_create(ci);
}

void Graphics::queue_destroy(QueueID queue)
{
	current_adapter.queue_destroy(queue);
}

void Graphics::queue_execute_command_buffer(QueueID queue, const Slice<CommandBufferID>& command_buffers)
{
	current_adapter.queue_execute_command_buffer(queue, command_buffers);
}

