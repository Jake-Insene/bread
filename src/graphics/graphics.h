#pragma once
#include "core/header.h"
#include "collections/string_view.h"
#include "mem/allocator.h"
#include "math/vec2.h"
#include "math/rect_2d.h"
#include "math/color.h"



struct Graphics
{
	/*
	*	Graphics API
	*/

	using PhysicalDeviceID = ID<u32, struct _PhysicalDeviceTag>;
	using SurfaceID = ID<u32, struct _SurfaceTag>;
	using DeviceID = ID<u32, struct _DeviceTag>;
	using FenceID = ID<u32, struct _FenceID>;
	using SemaphoreID = ID<u32, struct _SemaphoreD>;
	using QueueID = ID<u32, struct _QueueID>;
	using SwapChainID = ID<u32, struct _SwapChainTag>;
	using BufferID = ID<u32, struct _BufferTag>;
	using TextureID = ID<u32, struct _TextureTag>;
	using RenderTargetID = ID<u32, struct _RenderTargetTag>;
	using PipelineID = ID<u32, struct _PipelineTag>;
	using CommandPoolID = ID<u32, struct _CommandPoolID>;
	using CommandBufferID = ID<u32, struct _CommandBufferTag>;

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

	/*
	* Physical Device API
	*/
	enum class DeviceType
	{
		Unknown = 0,
		IntegratedGPU,
		DiscreteGPU,
	};

	struct PhysicalDeviceSurfaceInfo
	{
	};

	struct PhysicalDeviceInfo
	{
		DeviceType device_type;
		PhysicalDeviceSurfaceInfo surface;
	};

	static Slice<PhysicalDeviceID> physical_devices_enumerate();
	static PhysicalDeviceInfo physical_device_get_info(PhysicalDeviceID physical_device);

	/*
	* Surface API
	*/
	struct SurfaceCreateInfo
	{
		MemoryAddress window_native_handle;
	};


	static SurfaceID surface_create(const SurfaceCreateInfo& ci);
	static void surface_destroy(SurfaceID surface);

	/*
	* Device API
	*/
	struct DeviceCreateInfo
	{
		PhysicalDeviceID physical_device;
	};

	static DeviceID device_create(const DeviceCreateInfo& ci);
	static void device_destroy(DeviceID device);

	/*
	* SwapChain API
	*/
	enum class SurfaceFormat
	{
		Unknown = 0,
		RGBA8Unorm,
		RGBA8Srgb,
		BGRA8Unorm,
		BGRA8Srgb,
	};

	enum class PresentMode
	{
		Unknown = 0,
		Immediate,
		VSync,
	};

	struct SwapChainCreateInfo
	{
		DeviceID device;
		SurfaceID surface;
		PresentMode present_mode;
		SurfaceFormat format;
		u32 image_count;
		Vector2I size;
	};

	struct AcquireInfo
	{
		u64 timeout;
		Graphics::SemaphoreID semaphore;
		Graphics::FenceID fence;
	};
	
	static SwapChainID swap_chain_create(const SwapChainCreateInfo& ci);
	static void swap_chain_destroy(SwapChainID swap_chain);
	static TextureID swap_chain_get_texture(SwapChainID swap_chain, u32 image_index);
	static void swap_chain_acquire_next_image(SwapChainID swap_chain, const AcquireInfo& acquire_info, u32* image_index);

	/*
	* Fence
	*/
	struct FenceCreateInfo
	{
		DeviceID device;
		bool signaled;
	};

	static FenceID fence_create(const FenceCreateInfo& ci);
	static void fence_destroy(FenceID fence);
	static void fence_reset(Slice<FenceID> fences);
	static void fence_wait_for(Slice<FenceID> fences, bool wait_for_all, u64 timeout);

	/*
	* Semaphore
	*/
	struct SemaphoreCreateInfo
	{
		DeviceID device;
	};

	static SemaphoreID semaphore_create(const SemaphoreCreateInfo& ci);
	static void semaphore_destroy(SemaphoreID semaphore);

	/*
	* Queue API
	*/
	enum class QueueUsage
	{
		Unknown = 0,
		Graphics,
		Present,
	};

	struct QueueCreateInfo
	{
		QueueUsage usage;
		DeviceID device;
	};

	struct QueueExecuteInfo
	{
		Slice<SemaphoreID> wait_semaphores;
		Slice<Graphics::CommandBufferID> command_buffers;
		Slice<SemaphoreID> signal_semaphores;
		Graphics::FenceID fence;
	};

	struct QueuePresentInfo
	{
		Slice<SemaphoreID> wait_semaphores;
		Slice<SwapChainID> swapchains;
		Slice<u32> image_indices;
	};

	static QueueID queue_create(const QueueCreateInfo& ci);
	static void queue_destroy(QueueID queue);
	static void queue_execute_command_buffer(QueueID queue, const QueueExecuteInfo& execute_info);
	static void queue_present(QueueID queue, const QueuePresentInfo& present_info);
	static void queue_wait_idle(QueueID queue);

	// ====== Resources ======

	/*
	* Buffer API
	*/
	enum class BufferUsage
	{
		Unknown = 0,
		VertexBuffer,
		IndexBuffer,
		UniformBuffer,
	};
	
	struct BufferCreateInfo
	{
		BufferUsage usage;
		Slice<const u8> data;
	};

	static BufferID buffer_create(const BufferCreateInfo& ci);
	static void buffer_destroy(BufferID buffer);

	static Slice<u8> buffer_map_memory(BufferID buffer, usize offset, usize len);
	static void buffer_unmap_memory(BufferID buffer, const Slice<u8>& memory);

	/*
	* Texture API
	*/
	enum class TextureType
	{
		Unknown = 0,
		Texture2D,
	};

	enum class TextureFormat
	{
		Unknown = 0,
		RGBA8,
		RGB8,
		R8,
	};

	enum class TextureFilter
	{
		Unknown = 0,
		Nearest,
		Linear,
	};

	enum class TextureAspects
	{
		Color = Bit(0),
		Depth = Bit(1),
		Stencil = Bit(2),
	};

	enum class TextureLayout
	{
		Unknown = 0,
		RenderOutput,
		Present,
	};

	struct TextureSubresourceRange
	{
		TextureAspects aspects;
		u32 base_mip_level;
		u32 level_count;
		u32 base_array_layer;
		u32 layer_count;
	};

	struct TextureCreateInfo
	{
		TextureType type;
		TextureFormat format;
		TextureFilter min_filter;
		TextureFilter mag_filter;
		Vector2I size;
		Slice<const u8> pixels;
	};

	static TextureID texture_create(const TextureCreateInfo& ci);
	static void texture_destroy(TextureID texture);

	static Vector2I texture_get_size(TextureID texture);

	/*
	* Render Target API
	*/
	struct RenderTargetCreateInfo
	{
		TextureFormat format;
		TextureFormat depth_stencil_format;
		Vector2I size;
	};

	static RenderTargetID render_target_create(const RenderTargetCreateInfo& ci);
	static void render_target_destroy(RenderTargetID render_target);
	static TextureID render_target_get_texture(RenderTargetID render_target);

	/*
	* Pipeline API
	*/
	enum class PipelineBindPoint
	{
		Unknown = 0,
		Graphics,
	};

	enum class PrimitiveTopology
	{
		Unknown = 0,
		TriangleList,
		LineList,
	};

	enum class InputRate
	{
		Unknown = 0,
		Vertex,
		Instance,
	};

	enum class VertexFormat
	{
		Unknown = 0,
		RGBA32Float,
	};
	
	enum class ShaderStage
	{
		Unknown = 0,
		Vertex,
		Fragment,
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
		Slice<const u8> code;
	};

	struct PipelineCreateInfo
	{
		PipelineBindPoint bind_point;
		PrimitiveTopology topology;
		InputAssembly input_assembly;
		Slice<ShaderInfo> stages;
	};

	static PipelineID pipeline_create(const PipelineCreateInfo& ci);
	static void pipeline_destroy(PipelineID pipeline);

	/*
	* CommandPool
	*/
	struct CommandPoolCreateInfo
	{
		DeviceID device;
		QueueID queue;
	};

	static CommandPoolID command_pool_create(const CommandPoolCreateInfo& ci);
	static void command_pool_destroy(CommandPoolID command_pool);

	/*
	* CommandBuffer API
	*/
	enum class CommandBufferUsage
	{
		Unknown = 0,
		Graphics,
	};

	enum class IndexType
	{
		Unknown = 0,
		UInt8,
		UInt16,
		UInt32,
	};

	enum class PipelineStages
	{
		Begin = Bit(0),
		
		VertexInput = Bit(1),
		VertexShader = Bit(2),
		FragmentShader = Bit(3),
		RenderOutput = Bit(4),
		
		End = Bit(31),
	};

	enum class AccessMasks
	{
		RenderOutputRead = Bit(0),
		RenderOutputWrite = Bit(1),
	};

	struct CommandBufferAllocateInfo
	{
		CommandPoolID pool;
	};

	struct RenderPassBeginInfo
	{
		Vector2I size;
		SwapChainID swap_chain;
		u32 image_index;
		Color clear_color;
	};

	struct RenderPassEndInfo
	{

	};

	struct PipelineMemoryBarrier
	{

	};

	struct PipelineBufferBarrier
	{

	};

	struct PipelineTextureBarrier
	{
		PipelineStages src_stages;
		PipelineStages dest_stages;
		AccessMasks src_masks;
		AccessMasks dest_masks;
		TextureLayout src_layout;
		TextureLayout dest_layout;
		Graphics::TextureID texture;
		TextureSubresourceRange subresource_range;
	};

	static CommandBufferID command_buffer_allocate(const CommandBufferAllocateInfo& ci);
	static void command_buffer_free(CommandBufferID command_buffer);

	static void command_buffer_begin(CommandBufferID command_buffer);
	static void command_buffer_end(CommandBufferID command_buffer);

	static void command_buffer_begin_renderpass(CommandBufferID command_buffer, const RenderPassBeginInfo& begin_info);
	static void command_buffer_end_renderpass(CommandBufferID command_buffer, const RenderPassEndInfo& end_info);

	static void command_buffer_memory_barrier(CommandBufferID command_buffer, const PipelineMemoryBarrier& memory_barrier);
	static void command_buffer_buffer_barrier(CommandBufferID command_buffer, const PipelineBufferBarrier& buffer_barrier);
	static void command_buffer_texture_barrier(CommandBufferID command_buffer, const PipelineTextureBarrier& texture_barrier);

	static void command_buffer_blit_framebuffer(CommandBufferID command_buffer, RenderTargetID src_render_target, RenderTargetID dst_render_target, Rect2DI src_rect, Rect2DI dst_rect, TextureFilter filter);
	static void command_buffer_bind_vertex_buffers(CommandBufferID command_buffer, u32 binding, const Slice<BufferID>& buffers, const Slice<u32>& offsets, const Slice<u32>& strides);
	static void command_buffer_bind_index_buffer(CommandBufferID command_buffer, BufferID index_buffer, u32 offset, IndexType index_type);
	static void command_buffer_bind_pipeline(CommandBufferID command_buffer, PipelineID pipeline);
	static void command_buffer_bind_render_target(CommandBufferID command_buffer, RenderTargetID render_target);
	static void command_buffer_set_texture_unit(CommandBufferID command_buffer, u32 set, u32 base_slot, const Slice<TextureID>& textures);
	static void command_buffer_set_uniform(CommandBufferID command_buffer, u32 set, u32 base_slot, const Slice<BufferID>& buffers);
	static void command_buffer_set_viewport(CommandBufferID command_buffer, Rect2DI viewport_rect);
	static void command_buffer_clear(CommandBufferID command_buffer, RenderTargetID render_target, Color clear_color);
	static void command_buffer_draw(CommandBufferID command_buffer, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance);
	static void command_buffer_draw_indexed(CommandBufferID command_buffer, u32 index_count, u32 instance_count, u32 base_index, u32 base_vertex, u32 base_instance);
};


EnableBitOp(Graphics::TextureAspects);
EnableBitOp(Graphics::PipelineStages);
EnableBitOp(Graphics::AccessMasks);

