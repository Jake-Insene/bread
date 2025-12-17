#pragma once
#include "graphics/graphics.h"


struct Adapter
{
	VTFunc(void, initialize, const mem::Allocator&);
	VTFunc(void, shutdown);

	VTFunc(Graphics::SwapChainID, swap_chain_create, const Graphics::SwapChainCreateInfo& ci);
	VTFunc(void, swap_chain_destroy, Graphics::SwapChainID swap_chain);
	VTFunc(Graphics::RenderTargetID, swap_chain_get_render_target, Graphics::SwapChainID swap_chain, usize render_target_index);
	VTFunc(void, swap_chain_present, Graphics::SwapChainID swap_chain, usize render_target_index);
	
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

	VTFunc(Graphics::PipelineID, pipeline_create, const Graphics::PipelineCreateInfo& ci);
	VTFunc(void, pipeline_destroy, Graphics::PipelineID pipeline);
	
	VTFunc(Graphics::ProgramID, program_create, const Graphics::ProgramCreateInfo& ci);
	VTFunc(void, program_destroy, Graphics::ProgramID program);

	VTFunc(Graphics::CommandBufferID, command_buffer_create, const Graphics::CommandBufferCreateInfo& ci);
	VTFunc(void, command_buffer_destroy, Graphics::CommandBufferID cmd);
	VTFunc(void, command_buffer_begin, Graphics::CommandBufferID cmd);
	VTFunc(void, command_buffer_blit_framebuffer, Graphics::CommandBufferID cmd, Graphics::RenderTargetID src_render_target, Graphics::RenderTargetID dst_render_target, Rect2DI src_rect, Rect2DI dst_rect, Graphics::TextureFilter filter);
	VTFunc(void, command_buffer_bind_vertex_buffers, Graphics::CommandBufferID cmd, u32 binding, const Slice<Graphics::BufferID>& buffers, const Slice<u32>& offsets, const Slice<u32>& strides);
	VTFunc(void, command_buffer_bind_index_buffer, Graphics::CommandBufferID cmd, Graphics::BufferID index_buffer, u32 offset, Graphics::IndexType index_type);
	VTFunc(void, command_buffer_bind_pipeline, Graphics::CommandBufferID cmd, Graphics::PipelineID pipeline);
	VTFunc(void, command_buffer_bind_render_target, Graphics::CommandBufferID cmd, Graphics::RenderTargetID render_target);
	VTFunc(void, command_buffer_set_texture_unit, Graphics::CommandBufferID cmd, u32 set, u32 base_slot, const Slice<Graphics::TextureID>& textures);
	VTFunc(void, command_buffer_set_uniform, Graphics::CommandBufferID cmd, u32 set, u32 base_slot, const Slice<Graphics::BufferID>& buffers);
	VTFunc(void, command_buffer_set_viewport, Graphics::CommandBufferID cmd, Rect2DI viewport_rect);
	VTFunc(void, command_buffer_clear, Graphics::CommandBufferID cmd, Graphics::RenderTargetID render_target, Color clear_color);
	VTFunc(void, command_buffer_draw, Graphics::CommandBufferID cmd, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance);
	VTFunc(void, command_buffer_draw_indexed, Graphics::CommandBufferID cmd, u32 index_count, u32 instance_count, u32 base_index, u32 base_vertex, u32 base_instance);
	VTFunc(void, command_buffer_end, Graphics::CommandBufferID cmd);

	VTFunc(Graphics::QueueID, queue_create, const Graphics::QueueCreateInfo& ci);
	VTFunc(void, queue_destroy, Graphics::QueueID queue);
	VTFunc(void, queue_execute_command_buffer, Graphics::QueueID queue, const Slice<Graphics::CommandBufferID>& command_buffers);
};

