#pragma once
#include "collections/array.h"
#include "collections/static_array.h"
#include "collections/free_list.h"
#include "graphics/adapter.h"
#include "log/log.h"
#include "platform/platform_header.h"

#define GLESDebugInfo(...) Log::debug("[GLESDriver]: " __VA_ARGS__)
#define GLESFatal(...) Fatal("[GLESDriver]: " __VA_ARGS__)
#define GLESFailOn(cond, ...) FailOn(cond, "[GLESDriver]: " __VA_ARGS__)


struct GLESDriver
{
	static constexpr usize DefaultCommandBufferSize = 4096;
	using GLID = u32;

    struct GLInfo
    {
        i32 max_texture_units;
    };

	struct SwapChainImage
	{
		Graphics::RenderTargetID render_target;
	};

	struct SwapChain
	{
		StaticArray<SwapChainImage, 3> images;
		Display::WindowID window;

		Graphics::TextureFormat format;
		Graphics::PresentMode present_mode;
		Vector2I size;
	};

	struct Buffer
	{
		GLID glid;
		GLenum target;
		usize size;
		
		Graphics::BufferUsage usage;
		Graphics::UploadRate upload_rate;
		Graphics::MemoryAccess access;
	};

	struct Texture
	{
		GLID glid;
		Vector2I size;
	};

	struct RenderTarget
	{
		GLID glid;
		Graphics::TextureID texture;

		Graphics::TextureFormat format;
		Vector2I size;
	};

	struct Pipeline
	{
		GLID vertex_array;
		GLenum gl_topology;
		GLenum gl_program;

		Graphics::PipelineUsage usage;
		Graphics::PrimitiveTopology topology;
		Graphics::ProgramID program;
	};

	struct Program
	{
		GLID glid;
	};

	struct CommandBuffer
	{
		struct VertexBufferBinding
		{
			Graphics::BufferID buffer;
			u32 offset;
			u32 stride;
		};

		enum CommandType
		{
			COMMAND_TYPE_UNKNOWN = 0,
			COMMAND_TYPE_BIND_VERTEX_BUFFER,
			COMMAND_TYPE_BIND_INDEX_BUFFER,
			COMMAND_TYPE_BIND_PIPELINE,
			COMMAND_TYPE_BIND_RENDER_TARGET,
			COMMAND_TYPE_SET_TEXTURE_UNIT,
			COMMAND_TYPE_SET_UNIFORM,
			COMMAND_TYPE_SET_VIEWPORT,
			COMMAND_TYPE_CLEAR,
			COMMAND_TYPE_DRAW,
			COMMAND_TYPE_DRAW_INDEXED,
		};

		struct alignas(16) Command
		{
			CommandType type;
			u64 padding;
		};

		struct BindVertexBuffer : Command
		{
			u32 binding;
			u32 binding_count;
			VertexBufferBinding bindings[8];
		};

		struct BindIndexBuffer : Command
		{
			Graphics::BufferID index_buffer;
			u32 offset;
			Graphics::IndexType index_type;
		};

		struct BindPipeline : Command
		{
			Graphics::PipelineID pipeline;
		};

		struct BindRenderTarget : Command
		{
			Graphics::RenderTargetID render_target;
		};

		struct SetTextureUnit : Command
		{
			u32 set;
			u32 base_slot;
			u32 texture_count;
			Graphics::TextureID textures[16];
		};

		struct SetUniform: Command
		{
			u32 set;
			u32 base_slot;
			u32 uniform_count;
			Graphics::BufferID uniforms[16];
		};

		struct SetViewport : Command
		{
			Rect2DI viewport_rect;
		};

		struct Clear : Command
		{
			Graphics::RenderTargetID render_target;
			Color clear_color;
		};

		struct Draw : Command
		{
			u32 vertex_count;
			u32 instance_count;
			u32 base_vertex;
			u32 base_instance;
		};

		struct DrawIndexed : Command
		{
			u32 index_count;
			u32 instance_count;
			u32 base_index;
			u32 base_vertex;
			u32 base_instance;
		};

		union CommandUnit
		{
			Command base;
			BindVertexBuffer bind_vertex_buffer;
			BindIndexBuffer bind_index_buffer;
			BindPipeline bind_pipeline;
			BindRenderTarget bind_render_target;
			SetTextureUnit set_texture_unit;
			SetUniform set_uniform;
			SetViewport set_viewport;
			Clear clear;
			Draw draw;
			DrawIndexed draw_indexed;
		};

		[[nodiscard]] CommandUnit& allocate();

		Graphics::CommandBufferUsage usage;
		Slice<CommandUnit> commands;
		usize offset;
		bool ended;
	};

	struct Queue
	{
		Graphics::QueueUsage usage;
	};

	struct MemoryInfo
	{
		usize allocated;
		usize current;
		usize free;
	};

	struct GLState
	{
		GLID draw_framebuffer;
		GLID read_framebuffer;
		GLenum gl_topology;
		GLenum gl_index_type;
		Graphics::PipelineID current_pipeline;
	};

    struct InternalData
    {
        mem::Allocator allocator;
        
        GLInfo info;
		MemoryInfo memory;
		GLState state;

		FreeList<SwapChain, Graphics::SwapChainID> swapchains;
		FreeList<Buffer, Graphics::BufferID> buffers;
		FreeList<Texture, Graphics::TextureID> textures;
		FreeList<RenderTarget, Graphics::RenderTargetID> render_targets;
		FreeList<Pipeline, Graphics::PipelineID> pipelines;
		FreeList<Program, Graphics::ProgramID> programs;
		FreeList<CommandBuffer, Graphics::CommandBufferID> command_buffers;
		FreeList<Queue, Graphics::QueueID> queues;
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

	static Graphics::PipelineID pipeline_create(const Graphics::PipelineCreateInfo& ci);
	static void pipeline_destroy(Graphics::PipelineID pipeline);

	static Graphics::ProgramID program_create(const Graphics::ProgramCreateInfo& ci);
	static void program_destroy(Graphics::ProgramID program);

	static Graphics::CommandBufferID command_buffer_create(const Graphics::CommandBufferCreateInfo& ci);
	static void command_buffer_destroy(Graphics::CommandBufferID cmd);

	static void command_buffer_begin(Graphics::CommandBufferID cmd);
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

	static void _queue_execute_command_buffer(Queue& q, CommandBuffer& command_buffer);

	static Graphics::BufferID _allocate_buffer();
	static Graphics::TextureID _allocate_texture();
	static Graphics::RenderTargetID _allocate_render_target();
    static void _init_context();
};

