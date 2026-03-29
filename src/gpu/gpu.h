#pragma once
#include "core/header.h"
#include "collections/string_view.h"
#include "mem/allocator.h"
#include "math/vec2.h"
#include "math/vec3.h"
#include "math/rect_2d.h"
#include "math/color.h"



struct GPU
{
	/*
	*	GPU API
	*/

	using PhysicalDeviceID = ID<u32, struct _PhysicalDeviceTag>;
	using SurfaceID = ID<u32, struct _SurfaceTag>;
	using DeviceID = ID<u32, struct _DeviceTag>;
	using FenceID = ID<u32, struct _FenceID>;
	using SemaphoreID = ID<u32, struct _SemaphoreD>;
	using QueueID = ID<u32, struct _QueueID>;
	using SwapChainID = ID<u32, struct _SwapChainTag>;
	using MemoryHeapID = ID<u32, struct _MemoryHeapTag>;
	using BufferID = ID<u32, struct _BufferTag>;
	using SamplerID = ID<u32, struct _SamplerTag>;
	using TextureID = ID<u32, struct _TextureTag>;
	using RenderTargetID = ID<u32, struct _RenderTargetTag>;
	using DescriptorSetLayoutID = ID<u32, struct _DescriptorSetLayout>;
	using DescriptorPoolID = ID<u32, struct _DescriptorPoolTag>;
	using DescriptorSetID = ID<u32, struct _DescriptorSet>;
	using PipelineID = ID<u32, struct _PipelineTag>;
	using CommandPoolID = ID<u32, struct _CommandPoolID>;
	using CommandBufferID = ID<u32, struct _CommandBufferTag>;

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

	// ====== Enums ======

	enum class DescriptorType
	{
		Unknown = 0,
		UniformBuffer,
		StorageBuffer,
		CombinedTextureSampler,
	};
	
	enum class ShaderStage
	{
		Vertex = Bit(0),
		Fragment = Bit(1),
	};

	enum class CompareOp
	{
		Unknown = 0,
		Never,
		Always,
    	Equal,
    	NotEqual,
    	Less,
    	LessOrEqual,
    	Greater,
    	GreaterOrEqual,
	};

	enum class PipelineStages
	{
		Begin = Bit(0),
		
		VertexInput = Bit(1),
		VertexShader = Bit(2),
		FragmentShader = Bit(3),
		EarlyFragmentTestShader = Bit(4),
		LateFragmentTestShader = Bit(5),
		ComputeShader = Bit(6),
		RenderOutput = Bit(7),
		Transfer = Bit(8),
		
		End = Bit(31),
	};

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

	enum class AcquireResult
	{
		Uknown = 0,
		Acquired,
		Suboptimal,
		OutOfDate,
	};

	struct SwapChainCreateInfo
	{
		DeviceID device;
		SurfaceID surface;
		PresentMode present_mode;
		SurfaceFormat format;
		u32 min_image_count;
		Vector2U size;
	};

	struct AcquireInfo
	{
		u64 timeout;
		SemaphoreID semaphore;
		FenceID fence;
	};
	
	static SwapChainID swap_chain_create(const SwapChainCreateInfo& ci);
	static void swap_chain_destroy(SwapChainID swap_chain);
	static u32 swap_chain_get_image_count(SwapChainID swap_chain);
	static TextureID swap_chain_get_texture(SwapChainID swap_chain, u32 image_index);
	static AcquireResult swap_chain_acquire_next_image(SwapChainID swap_chain, const AcquireInfo& acquire_info, u32* image_index);

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
	static bool fence_get_state(FenceID fence);
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
		Compute,
		Copy,
		Present,
	};

	struct QueueCreateInfo
	{
		DeviceID device;
		QueueUsage usage;
	};

	struct QueueExecuteInfo
	{
		Slice<const SemaphoreID> wait_semaphores;
		Slice<const PipelineStages> wait_stages;
		Slice<const CommandBufferID> command_buffers;
		Slice<const SemaphoreID> signal_semaphores;
		FenceID fence;
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
	static AcquireResult queue_present(QueueID queue, const QueuePresentInfo& present_info);
	static void queue_wait_idle(QueueID queue);

	// ====== Resources ======

	static constexpr u32 ConstantBlockAlignment = 4;
	static constexpr u32 MaxConstantBlockSize = 128;

	/*
	* Memory Heap API
	*/

	static constexpr usize HeapAlignment = 4096;
	static constexpr usize MinHeapResourceAlignment = 4096;

	enum class HeapUsage
	{
		Unknown = 0,
		CPUExclusive,
		GPUExclusive,
		CPUGPUCoherent,
	};

	struct MemoryHeapCreateInfo
	{
		DeviceID device;
		HeapUsage heap_usage;
		usize heap_size;
	};

	static MemoryHeapID memory_heap_create(const MemoryHeapCreateInfo& ci);
	static void memory_heap_destroy(MemoryHeapID memory_heap);

	static Slice<u8> memory_heap_map(MemoryHeapID memory_heap, usize offset, usize len);
	static void memory_heap_unmap(MemoryHeapID memory_heap, const Slice<u8>& memory);

	/*
	* Buffer API
	*/
	enum class BufferUsage
	{
		VertexBuffer = Bit(0),
		IndexBuffer = Bit(1),
		UniformBuffer = Bit(2),
		TransferSource = Bit(3),
		TransferDestination = Bit(4),
	};
	
	struct BufferCreateInfo
	{
		DeviceID device;
		BufferUsage usage;
		usize size;
		MemoryHeapID memory_heap;
		usize heap_offset;
	};

	static BufferID buffer_create(const BufferCreateInfo& ci);
	static void buffer_destroy(BufferID buffer);

	/*
	* Sampler API
	*/

	enum class Filter
	{
		Unknown = 0,
		Nearest,
		Linear,
	};

	enum class SamplerMipMapMode
	{
		Unknown = 0,
		Nearest,
		Linear,
	};

	enum class SamplerAddressMode
	{
		Unknown = 0,
 		Repeat,
    	MirroredRepeat,
    	ClampToEdge,
    	ClampToBorder,
	};

	struct SamplerCreateInfo
	{
		DeviceID device;
		Filter min_filter;
		Filter mag_filter;
		SamplerMipMapMode mipmap_mode;
		SamplerAddressMode address_mode_u;
		SamplerAddressMode address_mode_v;
		SamplerAddressMode address_mode_w;
		f32 mip_lod_bias;
		bool anisotropy_enable;
		f32 max_anisotropy;
		bool compare_enable;
		CompareOp compare_op;
		f32 min_lod;
		f32 max_lod;
	};

	static SamplerID sampler_create(const SamplerCreateInfo& ci);
	static void sampler_destroy(SamplerID sampler);
	

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
		RGBA8Srgb,
		RGB8Srgb,
		RG8Srgb,
		R8Srgb,
	};

	enum class SampleCount
	{
		Unknown = 0,
		Sample1,
		Sample2,
		Sample4,
		Sample8,
		Sample16,
		Sample32,
		Sample64,
	};

	enum class TextureTiling
	{
		Unknown = 0,
		Optimal,
		Linear,
	};

	enum class TextureUsage
	{
		TransferSource = Bit(0),
		TransferDestination = Bit(1),
		Sampled = Bit(2),
		Storage = Bit(3),
		RenderOutput = Bit(4),
	};

	enum class TextureAspect
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
		ShaderReadOnly,
		TransferSource,
		TransferDestination,
	};

	struct TextureSubresourceRanges
	{
		TextureAspect aspect;
		u32 base_mip_level;
		u32 level_count;
		u32 base_array_layer;
		u32 layer_count;
	};

	struct TextureSubresourceLayers
	{
    	TextureAspect aspect;
    	uint32_t mip_level;
    	uint32_t base_array_layer;
    	uint32_t layer_count;
	};

	struct TextureCreateInfo
	{
		DeviceID device;
		TextureType type;
		TextureFormat format;
		Vector3U extent;
		u32 mip_levels;
		u32 array_levels;
		SampleCount sample_count;
		TextureTiling tiling;
		TextureUsage usage;
		TextureLayout initial_layout;

		MemoryHeapID memory_heap;
		usize heap_offset;
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
	* Descriptor Set
	*/
	struct DescriptorBinding
	{
		DescriptorType type;
		u32 binding;
		u32 count;
		ShaderStage stages;
	};

	struct DescriptorSetLayoutCreateInfo
	{
		DeviceID device;
		Slice<DescriptorBinding> bindings;
	};

	static DescriptorSetLayoutID descriptor_set_layout_create(const DescriptorSetLayoutCreateInfo& ci);
	static void descriptor_set_layout_destroy(DescriptorSetLayoutID descriptor_set_layout);

	/*
	* Descriptor Pool
	*/

	struct DescriptorPoolSize
	{
		DescriptorType type;
		u32 count;
	};
	
	struct DescriptorPoolCreateInfo
	{
		DeviceID device;
		u32 max_sets;
		Slice<const DescriptorPoolSize> sizes;
	};

	static DescriptorPoolID descriptor_pool_create(const DescriptorPoolCreateInfo& ci);
	static void descriptor_pool_destroy(DescriptorPoolID descriptor_pool);


	/*
	* Descriptor Set
	*/

	struct DescriptorSetAllocateInfo
	{
		DeviceID device;
		DescriptorPoolID pool;
		DescriptorSetLayoutID set_layout;
	};

	struct DescriptorTextureInfo
	{
		TextureID texture;
		TextureLayout layout;
		SamplerID sampler;
	};

	struct DescriptorBufferInfo
	{
		BufferID buffer;
		usize offset;
		usize range;
	};

	struct WriteDescriptorInfo
	{
		u32 binding;
		u32 array_element;
		u32 count;
		DescriptorType type;
		Slice<DescriptorTextureInfo> textures;
		Slice<DescriptorBufferInfo> buffers;
	};

	struct UpdateDescriptorInfo
	{
		Slice<WriteDescriptorInfo> write_infos;
	};

	static DescriptorSetID descriptor_set_allocate(const DescriptorSetAllocateInfo& ci);
	static void descriptor_set_free(DescriptorSetID descriptor_set);
	static void descriptor_set_update_descriptors(DescriptorSetID descriptor_set, const UpdateDescriptorInfo& update_info);

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

	enum class PolygonMode
	{
		Unknown = 0,
		Fill,
		Line,
		Point,
	};

	enum class CullMode
	{
		Unknown = 0,
		Front,
		Back,
		FrontAndBack,
	};

	enum class FrontFace
	{
		Unknown = 0,
		CounterClockWise,
		ClockWise,
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
		RGB32Float,
		RG32Float,
		R32Float,
	};

	struct ShaderStageInfo
	{
		ShaderStage stage;
		Slice<const u8> code;
		StringView name;
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
		VertexFormat format;
		u32 offset;
	};

	struct VertexInput
	{
		Slice<VertexBinding> bindings;
		Slice<VertexAttribute> attributes;
	};

	struct InputAssembly
	{
		PrimitiveTopology topology;
	};

	struct RasterizerState
	{
		bool depth_clamp_enable;
		bool rasterizer_discard_enable;
		PolygonMode polygon_mode;
		CullMode cull_mode;
		FrontFace front_face;
		f32 line_width;
	};

	struct MultisampleState
	{
		SampleCount sample_count;
		f32 min_sample_shading;
		bool sample_shading_enable;
		bool alpha_to_coverage_enable;
		bool alpha_one_enable;
	};

	struct DepthStencilState
	{
		bool depth_test_enable;
		bool depth_write_enable;
		bool depth_bounds_test_enable;
		bool stencil_test_enable;
		f32 min_depth_bounds;		
		f32 max_depth_bounds;		
	};

	struct ConstantBlock
	{
		ShaderStage stages;
		u32 offset;
		u32 size;
	};

	struct PipelineLayout
	{
		Slice<const ConstantBlock> constant_blocks;
		Slice<const DescriptorSetLayoutID> set_layouts;
	};

	struct PipelineCreateInfo
	{
		DeviceID device;
		PipelineBindPoint bind_point;
		Slice<const ShaderStageInfo> shader_stages;
		VertexInput vertex_input;
		InputAssembly input_assembly;
		RasterizerState rasterizer_state;
		MultisampleState multisample_state;
		DepthStencilState depth_stencil_state;
		PipelineLayout pipeline_layout;
		SurfaceFormat surface_format;
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

	enum class IndexType
	{
		Unknown = 0,
		UInt8,
		UInt16,
		UInt32,
	};

	enum class AccessMasks
	{
		RenderOutputRead = Bit(0),
		RenderOutputWrite = Bit(1),
		TransferRead = Bit(2),
		TransferWrite = Bit(3),
		ShaderRead = Bit(4),
		ShaderWrite = Bit(5),
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
		TextureID texture;
		TextureSubresourceRanges subresource_range;
	};

	struct BufferCopyRegion
	{
		usize source_offset;
		usize destination_offset;
		usize size;
	};
	
	struct CopyBufferToTextureInfo
	{
		BufferID source_buffer;
		usize source_offset;
		u32 row_length;
		u32 image_height;
		TextureID destination_texture;
		TextureLayout destination_layout;
		TextureSubresourceLayers subresource_layer;
		Vector3I offset;
		Vector3U extent;
	};

	struct  BufferCopyInfo
	{
		BufferID source_buffer;
		BufferID destination_buffer;
		Slice<BufferCopyRegion> copy_regions;
	};

	struct Viewport
	{
		f32 x;
		f32 y;
		f32 width;
		f32 height;
		f32 min_depth;
		f32 max_depth;
	};

	struct Scissor
	{
		i32 x;
		i32 y;
		u32 width;
		u32 height;
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

	static void command_buffer_copy_buffer_to_texture(CommandBufferID command_buffer, const CopyBufferToTextureInfo& copy_info);
	static void command_buffer_copy_buffer(CommandBufferID command_buffer, const BufferCopyInfo& copy_info);

	static void command_buffer_bind_pipeline(CommandBufferID command_buffer, PipelineBindPoint bind_point, PipelineID pipeline);
	static void command_buffer_bind_descriptor_sets(CommandBufferID command_buffer, PipelineBindPoint bind_point, PipelineID pipeline, u32 base_set, const Slice<DescriptorSetID>& descriptor_sets);
	static void command_buffer_bind_vertex_buffers(CommandBufferID command_buffer, u32 base_binding, const Slice<BufferID>& buffers, const Slice<usize>& offsets);
	static void command_buffer_constant_block(CommandBufferID command_buffer, PipelineID pipeline, ShaderStage stages, u32 offset, u32 size, MemoryAddress block_address);

	static void command_buffer_set_viewports(CommandBufferID command_buffer, u32 base_viewport, const Slice<Viewport>& viewports);
	static void command_buffer_set_scissors(CommandBufferID command_buffer, u32 base_scissor, const Slice<Scissor>& scissors);

	static void command_buffer_draw(CommandBufferID command_buffer, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance);
};


EnableBitOp(GPU::BufferUsage);
EnableBitOp(GPU::TextureUsage);
EnableBitOp(GPU::TextureAspect);
EnableBitOp(GPU::ShaderStage);
EnableBitOp(GPU::PipelineStages);
EnableBitOp(GPU::AccessMasks);

