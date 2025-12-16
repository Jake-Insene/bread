#pragma once
#include "core/header.h"
#include "collections/string_view.h"
#include "display/window.h"
#include "mem/allocator.h"
#include "math/vec2.h"
#include "math/rect_2d.h"
#include "math/color.h"



struct Graphics
{
	/*
	*	Graphics API
	*/

	using SwapChainID = ID<u32, struct _SwapChainTag>;
	using BufferID = ID<u32, struct _BufferTag>;
	using TextureID = ID<u32, struct _TextureTag>;
	using RenderTargetID = ID<u32, struct _RenderTargetTag>;
	using PipelineID = ID<u32, struct _PipelineTag>;
	using ProgramID = ID<u32, struct _ProgramTag>;
	using CommandBufferID = ID<u32, struct _CommandBufferTag>;
	using QueueID = ID<u32, struct _CommandBufferTag>;

	enum PresentMode
	{
		PRESENT_MODE_UNKNOWN = 0,
		PRESENT_MODE_IMMEDIATE,
		PRESENT_MODE_VSYNC,
	};

	enum MemoryAccess
	{
		MEMORY_ACCESS_UNKNOWN = 0,
		MEMORY_ACCESS_READ_ONLY,
		MEMORY_ACCESS_WRITE_ONLY,
		MEMORY_ACCESS_READ_WRITE,
	};

	enum PrimitiveTopology
	{
		PRIMITIVE_TOPOLOGY_UNKNOWN = 0,
		PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		PRIMITIVE_TOPOLOGY_LINE_LIST,
	};

	enum InputRate
	{
		INPUT_RATE_PER_UNKNOWN = 0,
		INPUT_RATE_PER_VERTEX,
		INPUT_RATE_PER_INSTANCE,
	};

	enum VertexFormat
	{
		VERTEX_FORMAT_UNKNOWN = 0,
		VERTEX_FORMAT_RGBA32F,
	};

	enum IndexType
	{
		INDEX_TYPE_UNKNOWN = 0,
		INDEX_TYPE_UINT8,
		INDEX_TYPE_UINT16,
		INDEX_TYPE_UINT32,
	};

	enum UploadRate
	{
		UPLOAD_RATE_UNKNOWN = 0,
		UPLOAD_RATE_STATIC,
		UPLOAD_RATE_DYNAMIC,
	};

	enum BufferUsage
	{
		BUFFER_USAGE_UNKNOWN = 0,
		BUFFER_USAGE_VERTEX,
		BUFFER_USAGE_INDEX,
		BUFFER_USAGE_UNIFORM,
	};

	enum TextureUsage
	{
		TEXTURE_USAGE_UNKNOWN = 0,
		TEXTURE_USAGE_UPLOAD_ONCE,
		TEXTURE_USAGE_DYNAMIC,
	};

	enum PipelineUsage
	{
		PIPELINE_USAGE_UNKNOWN = 0,
		PIPELINE_USAGE_GRAPHICS,
	};

	enum ShaderStage
	{
		SHADER_STAGE_UNKNOWN = 0,
		SHADER_STAGE_VERTEX,
		SHADER_STAGE_FRAGMENT,
	};

	enum CommandBufferUsage
	{
		COMMAND_BUFFER_USAGE_UNKNOWN = 0,
		COMMAND_BUFFER_USAGE_GRAPHICS,
	};

	enum QueueUsage
	{
		QUEUE_USAGE_UNKNOWN = 0,
		QUEUE_USAGE_GRAPHICS,
	};

	enum TextureType
	{
		TEXTURE_UNKNOWN = 0,
		TEXTURE_2D,
	};

	enum TextureFormat
	{
		TEXTURE_FORMAT_UNKNOWN = 0,
		TEXTURE_FORMAT_RGBA8,
		TEXTURE_FORMAT_RGB8,
		TEXTURE_FORMAT_R8,
	};

	enum TextureFilter
	{
		TEXTURE_FILTER_UNKNOWN = 0,
		TEXTURE_FILTER_NEAREST,
		TEXTURE_FILTER_LINEAR,
	};

	struct VertexBinding
	{
		u32 binding;
		u32 stride;
		InputRate input_rate;
	};

	struct VertexAttribute
	{
		u32 location;
		u32 binding;
		u32 offset;
		VertexFormat format;
	};

	struct InputAssembly
	{
		Slice<VertexBinding> bindings;
		Slice<VertexAttribute> attributes;
	};
	
	struct ShaderInfo
	{
		ShaderStage stage;
		StringView source_path;
		Slice<u8> code;
	};

	struct SwapChainCreateInfo
	{
		Display::WindowID window;
		PresentMode present_mode;
		TextureFormat format;
		u32 image_count;
		Vector2I size;
	};

	struct BufferCreateInfo
	{
		BufferUsage usage;
		UploadRate upload_rate;
		MemoryAccess access;
		Slice<const u8> data;
	};

	struct TextureCreateInfo
	{
		TextureUsage usage;
		TextureType type;
		TextureFormat format;
		TextureFilter min_filter;
		TextureFilter mag_filter;
		Vector2I size;
		Slice<const u8> pixels;
	};

	struct RenderTargetCreateInfo
	{
		TextureFormat format;
		TextureFormat depth_stencil_format;
		Vector2I size;
	};

	struct PipelineCreateInfo
	{
		PipelineUsage usage;
		PrimitiveTopology topology;
		InputAssembly input_assembly;
		ProgramID pipeline_program;
	};

	struct ProgramCreateInfo
	{
		Slice<ShaderInfo> shaders;
	};

	struct CommandBufferCreateInfo
	{
		CommandBufferUsage usage;
	};

	struct QueueCreateInfo
	{
		QueueUsage usage;
	};

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

	/*
	* SwapChain API
	*/
	static SwapChainID swap_chain_create(const SwapChainCreateInfo& ci);
	static void swap_chain_destroy(SwapChainID swap_chain);

	static RenderTargetID swap_chain_get_render_target(SwapChainID swap_chain, usize render_target_index);
	static void swap_chain_present(SwapChainID swap_chain, usize render_target_index);


	/*
	* Buffer API
	*/

	static BufferID buffer_create(const BufferCreateInfo& ci);
	static void buffer_destroy(BufferID buffer);

	static Slice<u8> buffer_map_memory(BufferID buffer, usize offset, usize len);
	static void buffer_unmap_memory(BufferID buffer, const Slice<u8>& memory);

	/*
	* Texture API
	*/
	static TextureID texture_create(const TextureCreateInfo& ci);
	static void texture_destroy(TextureID texture);

	static Vector2I texture_get_size(TextureID texture);

	/*
	* Render Target API
	*/
	static RenderTargetID render_target_create(const RenderTargetCreateInfo& ci);
	static void render_target_destroy(RenderTargetID render_target);

	/*
	* Pipeline API
	*/
	static PipelineID pipeline_create(const PipelineCreateInfo& ci);
	static void pipeline_destroy(PipelineID pipeline);

	/*
	* Program API
	*/
	static ProgramID program_create(const ProgramCreateInfo& ci);
	static void program_destroy(ProgramID program);

	/*
	* CommandBuffer API
	*/
	static CommandBufferID command_buffer_create(const CommandBufferCreateInfo& ci);
	static void command_buffer_destroy(CommandBufferID cmd);

	static void command_buffer_begin(CommandBufferID cmd);
	static void command_buffer_bind_vertex_buffers(CommandBufferID cmd, u32 binding, const Slice<BufferID>& buffers, const Slice<u32>& offsets, const Slice<u32>& strides);
	static void command_buffer_bind_index_buffer(CommandBufferID cmd, BufferID index_buffer, u32 offset, IndexType index_type);
	static void command_buffer_bind_pipeline(CommandBufferID cmd, PipelineID pipeline);
	static void command_buffer_bind_render_target(CommandBufferID cmd, RenderTargetID render_target);
	static void command_buffer_set_texture_unit(CommandBufferID cmd, u32 set, u32 base_slot, const Slice<TextureID>& textures);
	static void command_buffer_set_uniform(CommandBufferID cmd, u32 set, u32 base_slot, const Slice<BufferID>& buffers);
	static void command_buffer_set_viewport(CommandBufferID cmd, Rect2DI viewport_rect);
	static void command_buffer_clear(CommandBufferID cmd, RenderTargetID render_target, Color clear_color);
	static void command_buffer_draw(CommandBufferID cmd, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance);
	static void command_buffer_draw_indexed(CommandBufferID cmd, u32 index_count, u32 instance_count, u32 base_index, u32 base_vertex, u32 base_instance);
	static void command_buffer_end(CommandBufferID cmd);

	/*
	* Queue API
	*/
	static QueueID queue_create(const QueueCreateInfo& ci);
	static void queue_destroy(QueueID queue);
	static void queue_execute_command_buffer(QueueID queue, const Slice<CommandBufferID>& command_buffers);
};

