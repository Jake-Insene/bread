#pragma once
#include "graphics/adapter.h"
#include "graphics/vk/vk_header.h"
#include "log/log.h"
#include "debug/fail.h"
#include "platform/platform_header.h"


#define VKDebugInfo(...) Log::debug("[VKDriver]: " __VA_ARGS__)
#define VKFatal(...) Fatal("[VKDriver]: " __VA_ARGS__)
#define VKFailOn(cond, ...) FailOn(cond, "[VKDriver]: " __VA_ARGS__)



struct VulkanDriver
{
    struct InternalData
    {
        mem::Allocator allocator;

		struct
		{
			uint32_t api_version;
		} info;

		MemoryAddress vk_lib;

		VkInstance instance;
		VkDebugUtilsMessengerEXT messenger;
    };

    static inline InternalData data;

    [[nodiscard]] static mem::Allocator& get_allocator() { return data.allocator; }

    static Adapter get_adapter();

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static Graphics::SwapChainID swap_chain_create(const Graphics::SwapChainCreateInfo& ci);
	static void swap_chain_destroy(Graphics::SwapChainID swap_chain);
	static Graphics::RenderTargetID swap_chain_get_render_target(Graphics::SwapChainID swap_chain, usize render_target_index);
	static void swap_chain_present(Graphics::SwapChainID swap_chain, usize render_target_index);

	static Graphics::BufferID buffer_create(const Graphics::BufferCreateInfo& ci);
	static void buffer_destroy(Graphics::BufferID buffer);
	static Slice<u8> buffer_map_memory(Graphics::BufferID buffer, usize offset, usize len);
	static void buffer_unmap_memory(Graphics::BufferID buffer, const Slice<u8>& memory);
	
	static Graphics::TextureID texture_create(const Graphics::TextureCreateInfo& ci);
	static void texture_destroy(Graphics::TextureID texture);
	static Vector2I texture_get_size(Graphics::TextureID texture);

	static Graphics::RenderTargetID render_target_create(const Graphics::RenderTargetCreateInfo& ci);
	static void render_target_destroy(Graphics::RenderTargetID render_target);
	static Graphics::TextureID render_target_get_texture(Graphics::RenderTargetID render_target);

	static Graphics::PipelineID pipeline_create(const Graphics::PipelineCreateInfo& ci);
	static void pipeline_destroy(Graphics::PipelineID pipeline);

	static Graphics::ProgramID program_create(const Graphics::ProgramCreateInfo& ci);
	static void program_destroy(Graphics::ProgramID program);

	static Graphics::CommandBufferID command_buffer_create(const Graphics::CommandBufferCreateInfo& ci);
	static void command_buffer_destroy(Graphics::CommandBufferID cmd);

	static void command_buffer_begin(Graphics::CommandBufferID cmd);
	static void command_buffer_blit_framebuffer(Graphics::CommandBufferID cmd, Graphics::RenderTargetID src_render_target, Graphics::RenderTargetID dst_render_target, Rect2DI src_rect, Rect2DI dst_rect, Graphics::TextureFilter filter);
	static void command_buffer_bind_vertex_buffers(Graphics::CommandBufferID cmd, u32 binding, const Slice<Graphics::BufferID>& buffers, const Slice<u32>& offsets, const Slice<u32>& strides);
	static void command_buffer_bind_index_buffer(Graphics::CommandBufferID cmd, Graphics::BufferID index_buffer, u32 offset, Graphics::IndexType index_type);
	static void command_buffer_bind_pipeline(Graphics::CommandBufferID cmd, Graphics::PipelineID pipeline);
	static void command_buffer_bind_render_target(Graphics::CommandBufferID cmd, Graphics::RenderTargetID render_target);
	static void command_buffer_set_texture_unit(Graphics::CommandBufferID cmd, u32 set, u32 base_slot, const Slice<Graphics::TextureID>& textures);
	static void command_buffer_set_uniform(Graphics::CommandBufferID cmd, u32 set, u32 base_slot, const Slice<Graphics::BufferID>& buffers);
	static void command_buffer_set_viewport(Graphics::CommandBufferID cmd, Rect2DI viewport_rect);
	static void command_buffer_clear(Graphics::CommandBufferID cmd, Graphics::RenderTargetID render_target, Color clear_color);
	static void command_buffer_draw(Graphics::CommandBufferID cmd, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance);
	static void command_buffer_draw_indexed(Graphics::CommandBufferID cmd, u32 index_count, u32 instance_count, u32 base_index, u32 base_vertex, u32 base_instance);
	static void command_buffer_end(Graphics::CommandBufferID cmd);

	static Graphics::QueueID queue_create(const Graphics::QueueCreateInfo& ci);
	static void queue_destroy(Graphics::QueueID queue);
	static void queue_execute_command_buffer(Graphics::QueueID queue, const Slice<Graphics::CommandBufferID>& command_buffers);
};
