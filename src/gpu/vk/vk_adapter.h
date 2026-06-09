#pragma once
#include "collections/free_list.h"
#include "collections/hash_map.h"
#include "gpu/gpu_adapter.h"
#include "gpu/vk/vk_header.h"
#include "math/hash.h"
#include "mem/stack_allocator.h"
#include "os/os.h"


struct VulkanAdapter;

// Members are in GPU format
struct VkDriverAttachmentInfo
{
	u32 format : 10;
	u32 layout : 4;
	u32 load_op : 2;
	u32 store_op : 2;
};

union VkDriverAttachment
{
	VkDriverAttachmentInfo attachment;
	u32 bits;
};

struct VkDriverRenderPassKey
{
	VkDriverAttachment render_attachments[GPU::MaxRenderAttachmentCount];
	VkDriverAttachment depth_attachment;
	VkDriverAttachment stencil_attachment;

	static VkDriverRenderPassKey from_rendering_info(const GPU::RenderingInfo& rendering_info);
	static VkDriverRenderPassKey from_render_pass_begin_info(VulkanAdapter* adapter, const GPU::RenderPassBeginInfo& begin_info);
};

template<>
struct HashOfType<VkDriverRenderPassKey>
{
	[[nodiscard]] static constexpr u64 hashfunc(const VkDriverRenderPassKey& k)
    {
		return Math::Hash::fnv1a(
			Slice(
				reinterpret_cast<const u8*>(&k),
				sizeof(k)
			)
		);
    }
};

template<>
struct Comparator<VkDriverRenderPassKey>
{
	[[nodiscard]] static constexpr bool compare(const VkDriverRenderPassKey& k1, const VkDriverRenderPassKey& k2)
    {
		return Mem::compare(
			Slice(reinterpret_cast<const u8*>(&k1), sizeof(k1)),
			Slice(reinterpret_cast<const u8*>(&k2), sizeof(k2))
		);
    }
};

struct VulkanAdapter final : InternalGPU::GPUAdapter
{
	struct PhysicalDevice
	{
		VkPhysicalDevice vk_physical_device;
		GPU::PhysicalDeviceInfo info;
	};

	struct Surface
	{
		VkSurfaceKHR vk_surface;
		MemoryAddress window_native_handle;
	};

	struct RenderPassCache
	{
		VkRenderPass vk_render_pass;
		VkFramebuffer vk_framebuffer;

		u32 vk_attachment_count;

		GPU::DeviceID device;
	};

	// RenderPass Hash
	using RenderPassEntry = HashMap<VkDriverRenderPassKey, RenderPassCache>::KeyValue;
	using FramebufferEntry = HashMap<VkImageView, VkFramebuffer>::KeyValue;

	enum class FeatureLevel
	{
		// shader_float16_int8, index_type_uint8, create_renderpass2, imageless_framebuffer
		Level0 = 0,
		// dynamic rendering
		Level1,
	};

	static constexpr usize MaxQueueFamilyCount = usize(GPU::QueueUsage::Present);
	struct LogicalDevice
	{
		VkDevice vk_device;
		VkPhysicalDevice vk_physical_device;

		VkPhysicalDeviceFeatures vk_physical_device_features;
		VkPhysicalDeviceProperties vk_physical_device_properties;
		VkPhysicalDeviceMemoryProperties vk_physical_device_memory_properties;
		Vulkan::AdditionalExtensionSupport additional_extension_support;

		struct QueueFamily
		{
			uint32_t vk_family_index;
			Slice<VkQueue> vk_queues;
			Slice<GPU::QueueID> queue_ids;
		};

		QueueFamily families[MaxQueueFamilyCount];

		DeviceVulkanTable vk;
		
		FeatureLevel feature_level;

		// device resources
		HashMap<VkDriverRenderPassKey, RenderPassCache> render_pass_cache;

		GPU::DeviceID device;
	};

	struct SwapChainImage
	{
		VkImage vk_image;
		VkImageView vk_image_view;
		GPU::TextureID texture;
		GPU::TextureViewID texture_view;
	};

	struct SwapChain
	{
		VkDevice vk_device;
		VkSurfaceKHR vk_surface;
		VkSwapchainKHR vk_swapchain;

		u32 image_count;
        Slice<SwapChainImage> images;

		GPU::SurfaceID surface;
		GPU::DeviceID device;
	};

	struct Fence
	{
		VkDevice vk_device;
		VkFence vk_fence;

		GPU::DeviceID device;
	};

	struct Semaphore
	{
		VkDevice vk_device;
		VkSemaphore vk_semaphore;
	
		GPU::DeviceID device;
	};

	struct Queue
	{
		VkDevice vk_device;
		VkQueue vk_queue;

		uint32_t family_index;
		uint32_t queue_index;

		GPU::DeviceID device;
		GPU::QueueID queue;
	};

	struct MemoryHeap
	{
		VkDevice vk_device;
		VkDeviceMemory vk_memory;
        u32 vk_type_index;
        VkMemoryPropertyFlags vk_memory_flags;

		GPU::DeviceID device;
		GPU::MemoryHeapID memory_heap;
	};

	struct Buffer
	{
		VkDevice vk_device;
		VkBuffer vk_buffer;

		GPU::DeviceID device;
		GPU::BufferID buffer;
	};

	struct Sampler
	{
		VkDevice vk_device;
		VkSampler vk_sampler;

		GPU::DeviceID device;
		GPU::SamplerID sampler;
	};

	struct Texture
	{
		VkDevice vk_device;
		VkImage vk_image;
		
		GPU::TextureFormat format;
		Vector3U extent;

		GPU::DeviceID device;
		GPU::TextureID texture;
	};

	struct TextureView
	{
		VkDevice vk_device;
		VkImageView vk_image_view;

		GPU::TextureFormat format;

		GPU::DeviceID device;
		GPU::TextureViewID texture_view;
	};

	struct RenderTarget
	{
		VkDevice vk_device;
		VkFramebuffer vk_framebuffer;

		GPU::DeviceID device;
	};

	struct DescriptorSetLayout
	{
		VkDevice vk_device;
		VkDescriptorSetLayout vk_set_layout;

		GPU::DeviceID device;
		GPU::DescriptorSetLayoutID descriptor_set_layout;
	};

	struct DescriptorPool
	{
		VkDevice vk_device;
		VkDescriptorPool vk_descriptor_pool;

		GPU::DeviceID device;
		GPU::DescriptorPoolID descriptor_pool;
	};

	struct DescriptorSet
	{
		VkDevice vk_device;
		VkDescriptorPool vk_descriptor_pool;
		VkDescriptorSet vk_descriptor_set;

		GPU::DeviceID device;
		GPU::DescriptorSetID descriptor_set;
		GPU::DescriptorPoolID descriptor_pool;
	};

	struct PipelineLayout
	{
		VkDevice vk_device;
		VkPipelineLayout vk_pipeline_layout;

		GPU::DeviceID device;
		GPU::PipelineLayoutID pipeline_layout;
	};

	struct Pipeline
	{
		VkDevice vk_device;
		VkPipeline vk_pipeline;

		GPU::DeviceID device;
		GPU::PipelineID pipeline;
	};

	struct CommandPool
	{
		VkDevice vk_device;
		VkCommandPool vk_command_pool;
	
		GPU::DeviceID device;
	};

	struct CommandBuffer
	{
		VkDevice vk_device;
		VkCommandPool vk_command_pool;
		VkCommandBuffer vk_command_buffer;

		GPU::DeviceID device;
	};

	Mem::Allocator* internal_allocator;
	Mem::StackAllocator tmp_allocator;

	Slice<PhysicalDevice> physical_devices;
	Slice<GPU::PhysicalDeviceID> physical_device_ids;
	FreeList<Surface, GPU::SurfaceID> surfaces;
	FreeList<LogicalDevice, GPU::DeviceID> devices;
	FreeList<SwapChain, GPU::SwapChainID> swap_chains;
	FreeList<Fence, GPU::FenceID> fences;
	FreeList<Semaphore, GPU::SemaphoreID> semaphores;

	// Device will allocate more queue infos, so the GPU::QueueID can be unique per device created.
	Array<Queue> queues;
	FreeList<MemoryHeap, GPU::MemoryHeapID> memory_heaps;
	FreeList<Buffer, GPU::BufferID> buffers;
	FreeList<Sampler, GPU::SamplerID> samplers;
	FreeList<Texture, GPU::TextureID> textures;
	FreeList<TextureView, GPU::TextureViewID> texture_views;
	FreeList<DescriptorSetLayout, GPU::DescriptorSetLayoutID> descriptor_set_layouts;
	FreeList<DescriptorPool, GPU::DescriptorPoolID> descriptor_pools;
	FreeList<DescriptorSet, GPU::DescriptorSetID> descriptor_sets;
	FreeList<PipelineLayout, GPU::PipelineLayoutID> pipeline_layouts;
	FreeList<Pipeline, GPU::PipelineID> pipelines;
	FreeList<CommandPool, GPU::CommandPoolID> command_pools;
	FreeList<CommandBuffer, GPU::CommandBufferID> command_buffers;

	struct
	{
		uint32_t api_version;
	} info;

	OS::Handle vk_lib;

	VkInstance instance;
	VkSurfaceKHR dummy_surface;
	VkDebugUtilsMessengerEXT messenger;

    [[nodiscard]] Mem::Allocator* get_allocator() const { return internal_allocator; }
	[[nodiscard]] Mem::StackAllocator* acquire_tmp_allocator()
	{
		tmp_allocator.reset();
		return &tmp_allocator;
	}

    void initialize(Mem::Allocator* _allocator) override;
    void shutdown() override;

	Slice<GPU::PhysicalDeviceID> physical_devices_enumerate() override;
	GPU::PhysicalDeviceInfo physical_device_get_info(GPU::PhysicalDeviceID physical_device) override;

	GPU::SurfaceID surface_create(const GPU::SurfaceCreateInfo& ci) override;
	void surface_destroy(GPU::SurfaceID surface) override;

	GPU::DeviceID device_create(const GPU::DeviceCreateInfo& ci) override;
	void device_destroy(GPU::DeviceID device) override;

	GPU::SwapChainID swap_chain_create(const GPU::SwapChainCreateInfo& ci) override;
	void swap_chain_destroy(GPU::SwapChainID swap_chain) override;
	u32 swap_chain_get_image_count(GPU::SwapChainID swap_chain) override;
	GPU::TextureID swap_chain_get_image(GPU::SwapChainID swap_chain, u32 image_index) override;
	GPU::TextureViewID swap_chain_get_image_view(GPU::SwapChainID swap_chain, u32 image_index) override;
	GPU::AcquireResult swap_chain_acquire_next_image(GPU::SwapChainID swap_chain, const GPU::AcquireInfo& acquire_info, u32* image_index) override;

	GPU::FenceID fence_create(const GPU::FenceCreateInfo& ci) override;
	void fence_destroy(GPU::FenceID fence) override;
	bool fence_get_state(GPU::FenceID fence) override;
	void fence_reset(Slice<GPU::FenceID> fences) override;
	void fence_wait_for(Slice<GPU::FenceID> fences, bool wait_for_all, u64 timeout) override;

	GPU::SemaphoreID semaphore_create(const GPU::SemaphoreCreateInfo& ci) override;
	void semaphore_destroy(GPU::SemaphoreID semaphore) override;

	u32 queue_get_count(const GPU::QueueGetCountInfo& gci) override;
	GPU::QueueID queue_get(const GPU::QueueGetInfo& gi) override;
	void queue_execute_command_buffer(GPU::QueueID queue, const GPU::QueueExecuteInfo& execute_info) override;
	GPU::AcquireResult queue_present(GPU::QueueID queue, const GPU::QueuePresentInfo& present_info) override;
	void queue_wait_idle(GPU::QueueID queue) override;

	GPU::MemoryHeapID memory_heap_create(const GPU::MemoryHeapCreateInfo& ci) override;
	void memory_heap_destroy(GPU::MemoryHeapID memory_heap) override;
	Slice<u8> memory_heap_map(GPU::MemoryHeapID memory_heap, usize offset, usize len) override;
	void memory_heap_unmap(GPU::MemoryHeapID memory_heap, const Slice<u8>& memory) override;

	GPU::BufferID buffer_create(const GPU::BufferCreateInfo& ci) override;
	void buffer_destroy(GPU::BufferID buffer) override;
	GPU::MemoryRequirements buffer_get_memory_requirements(GPU::BufferID buffer) override;
	void buffer_bind_memory_heap(GPU::BufferID buffer, const GPU::BindMemoryInfo& bind_info) override;

	GPU::SamplerID sampler_create(const GPU::SamplerCreateInfo& ci) override;
	void sampler_destroy(GPU::SamplerID sampler) override;
	
	GPU::TextureID texture_create(const GPU::TextureCreateInfo& ci) override;
	void texture_destroy(GPU::TextureID texture) override;
	GPU::MemoryRequirements texture_get_memory_requirements(GPU::TextureID texture) override;
	void texture_bind_memory_heap(GPU::TextureID texture, const GPU::BindMemoryInfo& bind_info) override;

	GPU::TextureViewID texture_view_create(const GPU::TextureViewCreateInfo &ci) override;
	void texture_view_destroy(GPU::TextureViewID texture_view) override;

	GPU::DescriptorSetLayoutID descriptor_set_layout_create(const GPU::DescriptorSetLayoutCreateInfo& ci) override;
	void descriptor_set_layout_destroy(GPU::DescriptorSetLayoutID descriptor_set_layout) override;

	GPU::DescriptorPoolID descriptor_pool_create(const GPU::DescriptorPoolCreateInfo& ci) override;
	void descriptor_pool_destroy(GPU::DescriptorPoolID descriptor_pool) override;

	GPU::DescriptorSetID descriptor_set_allocate(const GPU::DescriptorSetAllocateInfo& ci) override;
	void descriptor_set_free(GPU::DescriptorSetID descriptor_set) override;
	void descriptor_set_update_descriptors(GPU::DescriptorSetID descriptor_set, const GPU::UpdateDescriptorInfo& update_info) override;

	GPU::PipelineLayoutID pipeline_layout_create(const GPU::PipelineLayoutCreateInfo& ci) override;
	void pipeline_layout_destroy(GPU::PipelineLayoutID pipeline_layout) override;

	GPU::PipelineID pipeline_create(const GPU::PipelineCreateInfo& ci) override;
	void pipeline_destroy(GPU::PipelineID pipeline) override;

	GPU::CommandPoolID command_pool_create(const GPU::CommandPoolCreateInfo& ci) override;
	void command_pool_destroy(GPU::CommandPoolID command_pool) override;

	GPU::CommandBufferID command_buffer_allocate(const GPU::CommandBufferAllocateInfo& ci) override;
	void command_buffer_free(GPU::CommandBufferID command_buffer) override;

	void command_buffer_begin(GPU::CommandBufferID command_buffer) override;
	void command_buffer_end(GPU::CommandBufferID command_buffer) override;
	void command_buffer_begin_renderpass(GPU::CommandBufferID command_buffer, const GPU::RenderPassBeginInfo& begin_info) override;
	void command_buffer_end_renderpass(GPU::CommandBufferID command_buffer, const GPU::RenderPassEndInfo& end_info) override;

	void command_buffer_pipeline_barrier(GPU::CommandBufferID command_buffer, const GPU::PipelineBarrier& pipeline_barrier) override;

	void command_buffer_copy_buffer_to_texture(GPU::CommandBufferID command_buffer, const GPU::CopyBufferToTextureInfo& copy_info) override;
	void command_buffer_copy_buffer(GPU::CommandBufferID command_buffer, const GPU::BufferCopyInfo& copy_info) override;

	void command_buffer_bind_pipeline(GPU::CommandBufferID command_buffer, GPU::PipelineBindPoint bind_point, GPU::PipelineID pipeline) override;
	void command_buffer_bind_descriptor_sets(GPU::CommandBufferID command_buffer, GPU::PipelineBindPoint bind_point, GPU::PipelineLayoutID pipeline_layout, u32 base_set, const Slice<GPU::DescriptorSetID>& descriptor_sets) override;
	void command_buffer_bind_vertex_buffers(GPU::CommandBufferID command_buffer, u32 base_binding, const Slice<GPU::BufferID>& buffers, const Slice<usize>& offsets) override;
	void command_buffer_constant_block(GPU::CommandBufferID command_buffer, GPU::PipelineLayoutID pipeline_layout, GPU::ShaderStage stages, u32 offset, u32 size, MemoryAddress block_address) override;

	void command_buffer_set_viewports(GPU::CommandBufferID command_buffer, u32 base_viewport, const Slice<const GPU::Viewport>& viewports) override;
	void command_buffer_set_scissors(GPU::CommandBufferID command_buffer, u32 base_scissor, const Slice<const GPU::Scissor>& scissors) override;

	void command_buffer_draw(GPU::CommandBufferID command_buffer, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance) override;

	Surface& _get_surface(GPU::SurfaceID surface) { return surfaces.get(surface); }
	LogicalDevice& _get_logical_device(GPU::DeviceID device) { return devices.get(device); }
	SwapChain& _get_swap_chain(GPU::SwapChainID swap_chain) { return swap_chains.get(swap_chain); }
	Fence& _get_fence(GPU::FenceID fence) { return fences.get(fence); }
	Semaphore& _get_semaphore(GPU::SemaphoreID semaphore) { return semaphores.get(semaphore); }
	Queue& _get_queue(GPU::QueueID queue) { return queues.get(queue.integer()); }
	MemoryHeap& _get_memory_heap(GPU::MemoryHeapID memory_heap) { return memory_heaps.get(memory_heap); }
	Buffer& _get_buffer(GPU::BufferID buffer) { return buffers.get(buffer); }
	Sampler& _get_sampler(GPU::SamplerID sampler) { return samplers.get(sampler); }
	Texture& _get_texture(GPU::TextureID texture) { return textures.get(texture); }
	TextureView& _get_texture_view(GPU::TextureViewID texture_view) { return texture_views.get(texture_view); }
	DescriptorSetLayout& _get_descriptor_set_layout(GPU::DescriptorSetLayoutID descriptor_set_layout) { return descriptor_set_layouts.get(descriptor_set_layout); }
	DescriptorPool& _get_descriptor_pool(GPU::DescriptorPoolID descriptor_pool) { return descriptor_pools.get(descriptor_pool); }
	DescriptorSet& _get_descriptor_set(GPU::DescriptorSetID descriptor_set) { return descriptor_sets.get(descriptor_set); }
	PipelineLayout& _get_pipeline_layout(GPU::PipelineLayoutID pipeline_layout) { return pipeline_layouts.get(pipeline_layout); }
	Pipeline& _get_pipeline(GPU::PipelineID pipeline) { return pipelines.get(pipeline); }
	CommandPool& _get_command_pool(GPU::CommandPoolID command_pool) { return command_pools.get(command_pool); }
	CommandBuffer& _get_command_buffer(GPU::CommandBufferID command_buffer) { return command_buffers.get(command_buffer); }

	void _get_physical_devices();

	VkShaderModule _vk_create_shader_module(LogicalDevice& ld, const GPU::ShaderStageInfo& shader_stage_info);

	RenderPassCache& _get_render_pass_for(LogicalDevice& ld, const GPU::RenderPassBeginInfo& begin_info);
	RenderPassCache& _get_render_pass_for_pipeline(LogicalDevice& ld, const GPU::RenderingInfo& pipeline_rendering_info);
	void _get_render_pass_and_framebuffer_for(LogicalDevice& ld, const GPU::RenderPassBeginInfo& begin_info,
		VkRenderPass* vk_render_pass, VkFramebuffer* vk_framebuffer);

	static GPU::DeviceType _vk_device_type_to_device_type(VkPhysicalDeviceType vk_device_type);
	static GPU::PresentMode _vk_present_mode_to_present_mode(VkPresentModeKHR vk_present_mode);
	static GPU::HeapUsage _vk_memory_property_to_heap_usage(VkMemoryPropertyFlags vk_memory_properties);

	static uint32_t _get_queue_family_for(const Slice<VkQueueFamilyProperties2>& vk_families,
		const Slice<u32>& acquired, VkQueueFlags vk_queue_flags);
	static uint32_t _get_queue_family_for_present(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface,
		const Slice<VkQueueFamilyProperties2>& vk_families);
	
	static void _vk_get_surface_format(GPU::TextureFormat surface_format, VkFormat* vk_image_format, VkColorSpaceKHR* vk_color_space);
	static VkSurfaceCapabilitiesKHR _vk_get_surface_capabilities(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface);
	static VkExtent2D _vk_get_swap_chain_extent(const Vector2U& size, const VkSurfaceCapabilitiesKHR& vk_capabilities);
};
