#pragma once
#include "core/header.h"
#include "collections/string_view.h"
#include "mem/allocator.h"
#include "math/vec2.h"
#include "math/vec3.h"


namespace InternalGPU
{
struct GPUAdapter;
}

namespace GPU
{
	/*
	* GPU API
	*/

	enum class ObjectType
	{
		Unknown = 0,
		PhysicalDevice,
		Surface,
		Device,
		SwapChain,
		Fence,
		Semaphore,
		Queue,
		MemoryHeap,
		Buffer,
		Sampler,
		Texture,
		TextureView,
		DescriptorSetLayout,
		DescriptorPool,
		DescriptorSet,
		PipelineLayout,
		Pipeline,
		CommandPool,
		CommandBuffer,

		ObjectCount = CommandBuffer,
	};

	using IntegralIDType = u32;

	using PhysicalDeviceID = ID<IntegralIDType, struct _PhysicalDeviceTag>;
	using SurfaceID = ID<IntegralIDType, struct _SurfaceTag>;
	using DeviceID = ID<IntegralIDType, struct _DeviceTag>;
	using SwapChainID = ID<IntegralIDType, struct _SwapChainTag>;
	using FenceID = ID<IntegralIDType, struct _FenceID>;
	using SemaphoreID = ID<IntegralIDType, struct _SemaphoreD>;
	using QueueID = ID<IntegralIDType, struct _QueueID>;
	using MemoryHeapID = ID<IntegralIDType, struct _MemoryHeapTag>;
	using BufferID = ID<IntegralIDType, struct _BufferTag>;
	using SamplerID = ID<IntegralIDType, struct _SamplerTag>;
	using TextureID = ID<IntegralIDType, struct _TextureTag>;
	using TextureViewID = ID<IntegralIDType, struct _TextureViewTag>;
	using DescriptorSetLayoutID = ID<IntegralIDType, struct _DescriptorSetLayout>;
	using DescriptorPoolID = ID<IntegralIDType, struct _DescriptorPoolTag>;
	using DescriptorSetID = ID<IntegralIDType, struct _DescriptorSet>;
	using PipelineLayoutID = ID<IntegralIDType, struct _PipelineLayoutTag>;
	using PipelineID = ID<IntegralIDType, struct _PipelineTag>;
	using CommandPoolID = ID<IntegralIDType, struct _CommandPoolID>;
	using CommandBufferID = ID<IntegralIDType, struct _CommandBufferTag>;

    void initialize(Mem::Allocator* allocator);
    void initialize_from_adapter(InternalGPU::GPUAdapter* adapter);
    void shutdown();

	InternalGPU::GPUAdapter* get_adapter();

	// ====== Enums ======

	enum class TextureFormat
	{
		Unknown = 0,
		RGBA8Srgb,
		RGB8Srgb,
		RG8Srgb,
		R8Srgb,

		RGBA8Unorm,
		RGB8Unorm,
		RG8Unorm,
		R8Unorm,

		BGRA8Srgb,
		BGRA8Unorm,
	};

	enum class DescriptorType
	{
		Unknown = 0,
		UniformBuffer,
		StorageBuffer,
		CombinedTextureSampler,
	};
	
	enum class ShaderStage
	{
		/*
		* Vertex stage.
		*/
		Vertex = Bit(0),
		/*
		* Fragment stage.
		*/
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

	enum class LoadOp
	{
		Unknown = 0,
		Load,
    	Clear,
    	DontCare,
	};

	enum class StoreOp
	{
		Unknown = 0,
		Store,
    	DontCare,
	};

	// ====== STRUCTS ======
	
	/*
	* Physical Device API
	*/
	enum class DeviceType
	{
		Unknown = 0,
		IntegratedGPU,
		DiscreteGPU,
		Cpu,
	};

	struct PhysicalDeviceSurfaceInfo
	{
	};

	struct PhysicalDeviceInfo
	{
		DeviceType device_type;
		PhysicalDeviceSurfaceInfo surface;
	};

	/**
	* Returns the gpu physical devices installed on this device.
	*/
	Slice<PhysicalDeviceID> physical_devices_enumerate();

	/**
	* Return the physical device information/capabilities of the given physical device.
	* @param physical_device The target physical device
	*/
	PhysicalDeviceInfo physical_device_get_info(PhysicalDeviceID physical_device);

	/**
	* Surface API
	*/
	struct SurfaceCreateInfo
	{
		/**
		* Platform native window handle.
		*/
		MemoryAddress window_native_handle;
	};

	/**
	* Creates a surface for the given window. Use this to create a SwapChain.
	* @param ci Surface creation parameters.
	*/
	SurfaceID surface_create(const SurfaceCreateInfo& ci);

	/**
	* Destroy the given surface.
	* @warning SwapChains created with this surface will be in an invalid state, they must be destroyed.
	*/
	void surface_destroy(SurfaceID surface);

	/*
	* Device API
	*/
	struct DeviceCreateInfo
	{
	};

	/**
	* Create a logical device that operates with the given device to commit work, allocate memory, etc...
	* @param physical_device Selected physical device.
	* @param ci Device creation parameters.
	*/
	DeviceID device_create(PhysicalDeviceID physical_device, const DeviceCreateInfo& ci);

	/**
	* Destroy the given logical device.
	* @warning Any resource allocated from this device must be destroyed before calling this function,
	* ignore this will cause undefined behaviour.
	*/
	void device_destroy(DeviceID device);

	/*
	* SwapChain API
	*/

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
		/*
		* Surface as present target of the images.
		*/
		SurfaceID surface;
		/*
		* Present mode/rate
		*/
		PresentMode present_mode;
		/*
		* Image texture format.
		* @note RGBA8Srgb/RGBA8Unorm are recomended.
		*/
		TextureFormat format;
		/*
		* Minimum images to allocate.
		* @note 3 is recomended.
		*/
		u32 min_image_count;
		/*
		* Image size, usually window size.
		*/
		Vector2U size;
	};

	struct AcquireInfo
	{
		u64 timeout;
		SemaphoreID semaphore;
		FenceID fence;
	};
	
	/**
	* Create a swap chain to present content on a surface.
	* @param device Logical device where the resource will reside.
	*/
	SwapChainID swap_chain_create(DeviceID device, const SwapChainCreateInfo& ci);

	void swap_chain_destroy(SwapChainID swap_chain);
	u32 swap_chain_get_image_count(SwapChainID swap_chain);
	TextureID swap_chain_get_image(SwapChainID swap_chain, u32 image_index);
	TextureViewID swap_chain_get_image_view(SwapChainID swap_chain, u32 image_index);
	AcquireResult swap_chain_acquire_next_image(SwapChainID swap_chain, const AcquireInfo& acquire_info, u32* image_index);

	/*
	* Fence
	*/
	struct FenceCreateInfo
	{
		bool signaled;
	};

	FenceID fence_create(DeviceID device, const FenceCreateInfo& ci);
	void fence_destroy(FenceID fence);
	bool fence_get_state(FenceID fence);
	void fence_reset(Slice<FenceID> fences);
	void fence_wait_for(Slice<FenceID> fences, bool wait_for_all, u64 timeout);

	/*
	* Semaphore
	*/
	struct SemaphoreCreateInfo
	{
	};

	SemaphoreID semaphore_create(DeviceID device, const SemaphoreCreateInfo& ci);
	void semaphore_destroy(SemaphoreID semaphore);

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

	struct QueueGetCountInfo
	{
		QueueUsage usage;
	};

	struct QueueGetInfo
	{
		QueueUsage usage;
		u32 index;
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
		Slice<const SemaphoreID> wait_semaphores;
		Slice<const SwapChainID> swapchains;
		Slice<const u32> image_indices;
	};

	u32 queue_get_count(DeviceID device, const QueueGetCountInfo& gci);
	QueueID queue_get(DeviceID device, const QueueGetInfo& gi);
	void queue_execute_command_buffer(QueueID queue, const QueueExecuteInfo& execute_info);
	AcquireResult queue_present(QueueID queue, const QueuePresentInfo& present_info);
	void queue_wait_idle(QueueID queue);

	// ====== Resources ======
	static constexpr u32 MaxRenderAttachmentCount = 8;

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
		HeapUsage heap_usage;
		usize heap_size;

		static constexpr MemoryHeapCreateInfo create(HeapUsage heap_usage, usize heap_size)
		{
			return MemoryHeapCreateInfo
			{
				.heap_usage = heap_usage,
				.heap_size = heap_size,
			};
		}
	};

	struct MemoryRequirements
	{
		usize size;
		usize alignment;
		HeapUsage heap_usage;
	};

	struct BindMemoryInfo
	{
		MemoryHeapID memory_heap;
		usize heap_offset;

		static constexpr BindMemoryInfo bind(MemoryHeapID memory_heap, usize heap_offset)
		{
			return BindMemoryInfo
			{
				.memory_heap = memory_heap,
				.heap_offset = heap_offset,
			};
		}
	};

	MemoryHeapID memory_heap_create(DeviceID device, const MemoryHeapCreateInfo& ci);
	void memory_heap_destroy(MemoryHeapID memory_heap);

	Slice<u8> memory_heap_map(MemoryHeapID memory_heap, usize offset, usize len);
	void memory_heap_unmap(MemoryHeapID memory_heap, const Slice<u8>& memory);

	/*
	* Buffer API
	*/
	enum class BufferUsage
	{
		/*
		* Can be used on command_buffer_bind_vertex_buffers.
		*/
		VertexBuffer = Bit(0),
		IndexBuffer = Bit(1),
		/*
		* Can be used for a descriptor slot of type UniformBuffer.
		*/
		UniformBuffer = Bit(2),
		/*
		* Can be used as source of a copy operation.
		*/
		TransferSource = Bit(3),
		/*
		* Can be used as destination of a copy operation.
		*/
		TransferDestination = Bit(4),
	};
	
	struct BufferCreateInfo
	{
		BufferUsage usage;
		usize size;

		static constexpr BufferCreateInfo create(BufferUsage usage, usize size)
		{
			return BufferCreateInfo
			{
				.usage = usage,
				.size = size,
			};
		}
	};

	BufferID buffer_create(DeviceID device, const BufferCreateInfo& ci);
	void buffer_destroy(BufferID buffer);

	MemoryRequirements buffer_get_memory_requirements(BufferID buffer);
	void buffer_bind_memory_heap(BufferID buffer, const BindMemoryInfo& bind_info);

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

	SamplerID sampler_create(DeviceID device, const SamplerCreateInfo& ci);
	void sampler_destroy(SamplerID sampler);

	/*
	* Texture API
	*/
	enum class TextureType
	{
		Unknown = 0,
		Texture2D,
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
		RenderAttachment,
		Present,
		ShaderReadOnly,
		TransferSource,
		TransferDestination,
	};

	struct TextureSubresourceRange
	{
		TextureAspect aspect;
		u32 base_mip_level;
		u32 level_count;
		u32 base_array_layer;
		u32 layer_count;

		static constexpr TextureSubresourceRange color(u32 base_mip_level, u32 level_count, u32 base_array_layer, u32 layer_count)
		{
			return TextureSubresourceRange
			{
				.aspect = GPU::TextureAspect::Color,
				.base_mip_level = base_mip_level,
				.level_count = level_count,
				.base_array_layer = base_array_layer,
				.layer_count = layer_count,
			};
		}
	};

	struct TextureSubresourceLayers
	{
    	TextureAspect aspect;
    	uint32_t mip_level;
    	uint32_t base_array_layer;
    	uint32_t layer_count;

		static constexpr TextureSubresourceLayers color(u32 mip_level, u32 base_array_layer, u32 layer_count)
		{
			return TextureSubresourceLayers
			{
				.aspect = GPU::TextureAspect::Color,
				.mip_level = mip_level,
				.base_array_layer = base_array_layer,
				.layer_count = layer_count,
			};
		}
	};

	struct TextureCreateInfo
	{
		TextureType type;
		TextureFormat format;
		Vector3U extent;
		u32 mip_levels;
		u32 array_levels;
		SampleCount sample_count;
		TextureTiling tiling;
		TextureUsage usage;
		TextureLayout initial_layout;
		TextureSubresourceRange subresource_range;

		static constexpr TextureCreateInfo create(TextureType type, TextureFormat format,
			const Vector3U& extent, u32 mip_levels, u32 array_levels, SampleCount sample_count,
			TextureTiling tiling, TextureUsage usage, TextureLayout initial_layout,
			TextureSubresourceRange subresource_range)
		{
			return TextureCreateInfo
			{
				.type = type,
				.format = format,
				.extent = extent,
				.mip_levels = mip_levels,
				.array_levels = array_levels,
				.sample_count = sample_count,
				.tiling = tiling,
				.usage = usage,
				.initial_layout = initial_layout,
				.subresource_range = subresource_range,
			};
		}
	};

	TextureID texture_create(DeviceID device, const TextureCreateInfo& ci);
	void texture_destroy(TextureID texture);

	MemoryRequirements texture_get_memory_requirements(TextureID texture);
	void texture_bind_memory_heap(TextureID texture, const BindMemoryInfo& bind_info);

	/*
	* Texture View
	*/
	enum class TextureViewType
	{
		Unknown = 0,
		Texture2D,
	};

	struct TextureViewCreateInfo
	{
		TextureViewType type;
		TextureFormat format;
		TextureID texture;
		TextureSubresourceRange subresource_range;

		static constexpr TextureViewCreateInfo create(TextureViewType type, TextureFormat format,
			TextureID texture, TextureSubresourceRange subresource_range)
		{
			return TextureViewCreateInfo
			{
				.type = type,
				.format = format,
				.texture = texture,
				.subresource_range = subresource_range,
			};
		}
	};

	TextureViewID texture_view_create(DeviceID device, const TextureViewCreateInfo& ci);
	void texture_view_destroy(TextureViewID texture_view);

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
		Slice<const DescriptorBinding> bindings;

		static constexpr DescriptorSetLayoutCreateInfo create(const Slice<const DescriptorBinding>& bindings)
		{
			return DescriptorSetLayoutCreateInfo
			{
				.bindings = bindings,
			};
		}
	};

	DescriptorSetLayoutID descriptor_set_layout_create(DeviceID device, const DescriptorSetLayoutCreateInfo& ci);
	void descriptor_set_layout_destroy(DescriptorSetLayoutID descriptor_set_layout);

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
		u32 max_sets;
		Slice<const DescriptorPoolSize> sizes;

		static constexpr DescriptorPoolCreateInfo create(u32 max_sets, Slice<const DescriptorPoolSize> sizes)
		{
			return DescriptorPoolCreateInfo
			{
				.max_sets = max_sets,
				.sizes = sizes,
			};
		}
	};

	DescriptorPoolID descriptor_pool_create(DeviceID device, const DescriptorPoolCreateInfo& ci);
	void descriptor_pool_destroy(DescriptorPoolID descriptor_pool);


	/*
	* Descriptor Set
	*/

	struct DescriptorSetAllocateInfo
	{
		DescriptorPoolID pool;
		Slice<const DescriptorSetLayoutID> set_layouts;
	};

	struct DescriptorBufferInfo
	{
		BufferID buffer;
		usize offset;
		usize range;
	};

	struct DescriptorTextureInfo
	{
		TextureViewID texture_view;
		TextureLayout layout;
		SamplerID sampler;
	};

	struct WriteDescriptorInfo
	{
		DescriptorSetID descriptor_set;
		u32 binding;
		u32 array_element;
		DescriptorType type;
		Slice<const DescriptorBufferInfo> buffers;
		Slice<const DescriptorTextureInfo> textures;
	};

	struct UpdateDescriptorInfo
	{
		Slice<const WriteDescriptorInfo> write_infos;
	};

	void descriptor_set_allocate(DeviceID device, const DescriptorSetAllocateInfo& ci, Slice<DescriptorSetID> out_descriptor_sets);
	void descriptor_set_free(DescriptorPoolID descriptor_pool, const Slice<const GPU::DescriptorSetID>& descriptor_sets);
	void descriptor_set_update_descriptors(DeviceID device, const UpdateDescriptorInfo& update_info);

	/*
	* Pipeline Layout API
	*/
	struct ConstantBlock
	{
		ShaderStage stages;
		u32 offset;
		u32 size;
	};

	struct PipelineLayoutCreateInfo
	{
		Slice<const ConstantBlock> constant_blocks;
		Slice<const DescriptorSetLayoutID> set_layouts;

		static constexpr PipelineLayoutCreateInfo create(const Slice<const ConstantBlock>& constant_blocks,
			const Slice<const DescriptorSetLayoutID>& set_layouts)
		{
			return PipelineLayoutCreateInfo
			{
				.constant_blocks = constant_blocks,
				.set_layouts = set_layouts,
			};
		}
	};

	PipelineLayoutID pipeline_layout_create(DeviceID device, const PipelineLayoutCreateInfo& ci);
	void pipeline_layout_destroy(PipelineLayoutID pipeline_layout);

	/*
	* Pipeline API
	*/
	static constexpr usize MaxVertexInputBindings = 16;
	static constexpr usize MaxVertexInputAttributes = 16;

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
		Slice<const VertexBinding> bindings;
		Slice<const VertexAttribute> attributes;

		static constexpr VertexInput input(const Slice<const VertexBinding>& bindings,
			const Slice<const VertexAttribute>& attributes)
		{
			return VertexInput
			{
				.bindings = bindings,
				.attributes = attributes,
			};
		}
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

		static constexpr RasterizerState state(PolygonMode polygon_mode, CullMode cull_mode, FrontFace front_face)
		{
			return RasterizerState
			{
				.depth_clamp_enable = false,
				.rasterizer_discard_enable = false,
				.polygon_mode = polygon_mode,
				.cull_mode = cull_mode,
				.front_face = front_face,
				.line_width = 1.F,
			};
		}
	};

	struct MultisampleState
	{
		SampleCount sample_count;
		f32 min_sample_shading;
		bool sample_shading_enable;
		bool alpha_to_coverage_enable;
		bool alpha_one_enable;

		static constexpr MultisampleState disable()
		{
			return MultisampleState
			{
				.sample_count = SampleCount::Sample1,
				.min_sample_shading = 0.F,
				.sample_shading_enable = false,
				.alpha_to_coverage_enable = false,
				.alpha_one_enable = false,
			};
		}
	};

	struct DepthStencilState
	{
		bool depth_test_enable;
		bool depth_write_enable;
		bool depth_bounds_test_enable;
		bool stencil_test_enable;
		f32 min_depth_bounds;		
		f32 max_depth_bounds;	
		
		static constexpr DepthStencilState depth_stencil_disable()
		{
			return DepthStencilState
			{
				.depth_test_enable = false,
				.depth_write_enable = false,
				.depth_bounds_test_enable = false,
				.stencil_test_enable = false,
				.min_depth_bounds = 0.F,
				.max_depth_bounds = 1.F,
			};
		}
	};

	struct RenderingInfo
	{
		Slice<const TextureFormat> render_attachment_formats;
		TextureFormat depth_attachment_format;
		TextureFormat stencil_attachment_format;

		static constexpr RenderingInfo render_attachments(const Slice<const TextureFormat>& render_attachment_formats)
		{
			return RenderingInfo
			{
				.render_attachment_formats = render_attachment_formats,
				.depth_attachment_format = TextureFormat::Unknown,
				.stencil_attachment_format = TextureFormat::Unknown,
			};
		}
	};

	struct PipelineCreateInfo
	{
		PipelineBindPoint bind_point;
		Slice<const ShaderStageInfo> shader_stages;
		VertexInput vertex_input;
		InputAssembly input_assembly;
		RasterizerState rasterizer_state;
		MultisampleState multisample_state;
		DepthStencilState depth_stencil_state;
		PipelineLayoutID pipeline_layout;
		RenderingInfo rendering_info;
	};

	PipelineID pipeline_create(DeviceID device, const PipelineCreateInfo& ci);
	void pipeline_destroy(PipelineID pipeline);

	/*
	* CommandPool
	*/
	struct CommandPoolCreateInfo
	{
		QueueUsage usage;
	};

	CommandPoolID command_pool_create(DeviceID device, const CommandPoolCreateInfo& ci);
	void command_pool_destroy(CommandPoolID command_pool);

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
		RenderAttachmentRead = Bit(0),
		RenderAttachmentWrite = Bit(1),
		TransferRead = Bit(2),
		TransferWrite = Bit(3),
		ShaderRead = Bit(4),
		ShaderWrite = Bit(5),
	};

	struct RenderArea
	{
		Vector2 offset;
		Vector2 extent;
		i32 x;
		i32 y;
		u32 width;
		u32 height;
	};

	struct ClearColor
	{
		f32 r;
		f32 g;
		f32 b;
		f32 a;
	};

	struct ClearDepthStencil
	{
		f32 depth;
		u32 stencil;
	};

	union ClearValue
	{
		ClearColor clear_color;
		ClearDepthStencil depth_stencil;

		static constexpr ClearValue rgba(f32 red, f32 green, f32 blue, f32 alpha)
		{
			return ClearValue
			{
				.clear_color =
				{
					.r = red,
					.g = green,
					.b = blue,
					.a = alpha,
				}
			};
		}
	};

	struct CommandBufferAllocateInfo
	{
		CommandPoolID pool;
	};

	struct AttachmentInfo
	{
		TextureViewID texture_view;
		TextureLayout layout;
		TextureViewID resolve_texture_view;
		TextureLayout resolve_layout;
		LoadOp load_op;
		StoreOp store_op;
		ClearValue clear_value;
	};

	struct RenderPassBeginInfo
	{
		Vector2I offset;
		Vector3U extent;
		Slice<const AttachmentInfo> render_attachments;
		AttachmentInfo depth_attachment;
		AttachmentInfo stencil_attachment;
	};

	struct RenderPassEndInfo
	{
	};

	struct PipelineMemoryBarrier
	{
		AccessMasks src_masks;
		AccessMasks dest_masks;
	};

	struct PipelineBufferBarrier
	{
		AccessMasks src_masks;
		AccessMasks dest_masks;
		BufferID buffer;
		usize offset;
		usize size;
	};

	struct PipelineTextureBarrier
	{
		AccessMasks src_masks;
		AccessMasks dest_masks;
		TextureLayout src_layout;
		TextureLayout dest_layout;
		TextureID texture;
		TextureSubresourceRange subresource_range;
	};

	struct PipelineBarrier
	{
		PipelineStages src_stages;
		PipelineStages dest_stages;
		Slice<const PipelineMemoryBarrier> memory_barriers;
		Slice<const PipelineBufferBarrier> buffer_barriers;
		Slice<const PipelineTextureBarrier> texture_barriers;

		static constexpr PipelineBarrier texture_barrier(PipelineStages src_stages, PipelineStages dest_stages,
			const Slice<const PipelineTextureBarrier>& texture_barries)
		{
			return PipelineBarrier
			{
				.src_stages = src_stages,
				.dest_stages = dest_stages,
				.memory_barriers = {},
				.buffer_barriers = {},
				.texture_barriers = texture_barries,
			};
		}
	};

	struct BufferCopyRegion
	{
		usize src_offset;
		usize dest_offset;
		usize size;
	};

	struct BufferTextureCopyRegion
	{
		usize buffer_offset;
		u32 buffer_row_length;
		u32 buffer_texture_height;
		TextureSubresourceLayers texture_subresource_layer;
		Vector3I texture_offset;
		Vector3U texture_extent;

		static constexpr BufferTextureCopyRegion region(usize buffer_offset,
			const TextureSubresourceLayers& texture_subresource_layer,
			const Vector3I& texture_offset, const Vector3U& texture_extent)
		{
			return BufferTextureCopyRegion
			{
				.buffer_offset = buffer_offset,
				.buffer_row_length = 0,
				.buffer_texture_height = 0,
				.texture_subresource_layer = texture_subresource_layer,
				.texture_offset = texture_offset,
				.texture_extent = texture_extent,
			};
		}
	};
	
	struct CopyBufferToTextureInfo
	{
		BufferID src_buffer;
		TextureID dest_texture;
		TextureLayout dest_layout;
		Slice<const BufferTextureCopyRegion> regions;
		
		static constexpr CopyBufferToTextureInfo copy(BufferID src_buffer, TextureID dest_texture,
			TextureLayout dest_layout, const Slice<const BufferTextureCopyRegion>& regions)
		{
			return CopyBufferToTextureInfo
			{
				.src_buffer = src_buffer,
				.dest_texture = dest_texture,
				.dest_layout = dest_layout,
				.regions = regions,
			};
		}
	};

	struct CopyBufferInfo
	{
		BufferID src_buffer;
		BufferID dest_buffer;
		Slice<const BufferCopyRegion> copy_regions;
	};

	struct Viewport
	{
		f32 x;
		f32 y;
		f32 width;
		f32 height;
		f32 min_depth;
		f32 max_depth;

		static constexpr Viewport extent(f32 width, f32 height)
		{
			return Viewport
			{
				.x = 0,
				.y = 0,
				.width = width,
				.height = height,
				.min_depth = 0.F,
				.max_depth = 1.F,
			};
		}
	};

	struct Scissor
	{
		i32 x;
		i32 y;
		u32 width;
		u32 height;

		static constexpr Scissor extent(u32 width, u32 height)
		{
			return Scissor
			{
				.x = 0,
				.y = 0,
				.width = width,
				.height = height,
			};
		}

		static constexpr Scissor scissor(i32 x, i32 y, u32 width, u32 height)
		{
			return Scissor
			{
				.x = x,
				.y = y,
				.width = width,
				.height = height,
			};
		}
	};

	CommandBufferID command_buffer_allocate(DeviceID device, const CommandBufferAllocateInfo& ci);
	void command_buffer_free(CommandBufferID command_buffer);

	void command_buffer_begin(CommandBufferID command_buffer);
	void command_buffer_end(CommandBufferID command_buffer);

	void command_buffer_begin_renderpass(CommandBufferID command_buffer, const RenderPassBeginInfo& begin_info);
	void command_buffer_end_renderpass(CommandBufferID command_buffer, const RenderPassEndInfo& end_info);

	void command_buffer_pipeline_barrier(CommandBufferID command_buffer, const PipelineBarrier& pipeline_barrier);

	void command_buffer_copy_buffer_to_texture(CommandBufferID command_buffer, const CopyBufferToTextureInfo& copy_info);
	void command_buffer_copy_buffer(CommandBufferID command_buffer, const CopyBufferInfo& copy_info);

	void command_buffer_bind_pipeline(CommandBufferID command_buffer, PipelineBindPoint bind_point, PipelineID pipeline);
	void command_buffer_bind_descriptor_sets(CommandBufferID command_buffer, PipelineBindPoint bind_point, PipelineLayoutID pipeline_layout, u32 base_set, const Slice<const DescriptorSetID>& descriptor_sets);
	void command_buffer_bind_vertex_buffers(CommandBufferID command_buffer, u32 base_binding, const Slice<const BufferID>& buffers, const Slice<const usize>& offsets);
	void command_buffer_constant_block(CommandBufferID command_buffer, PipelineLayoutID pipeline_layout, ShaderStage stages, u32 offset, u32 size, MemoryAddress block_address);

	void command_buffer_set_viewports(CommandBufferID command_buffer, u32 base_viewport, const Slice<const Viewport>& viewports);
	void command_buffer_set_scissors(CommandBufferID command_buffer, u32 base_scissor, const Slice<const Scissor>& scissors);

	void command_buffer_draw(CommandBufferID command_buffer, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance);
};


EnableBitOp(GPU::BufferUsage);
EnableBitOp(GPU::TextureUsage);
EnableBitOp(GPU::TextureAspect);
EnableBitOp(GPU::ShaderStage);
EnableBitOp(GPU::PipelineStages);
EnableBitOp(GPU::AccessMasks);

