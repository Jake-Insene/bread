#pragma once
#include "collections/free_list.h"
#include "collections/hash_map.h"
#include "gpu/gpu_adapter.h"
#include "gpu/vk/vk_header.h"
#include "mem/stack_allocator.h"
#include "platform/platform_header.h"



template<>
struct HashOfType<VkImage>
{
    [[nodiscard]] static constexpr u64 hashfunc(const VkFormat& k)
    {
        return k;
    }

    [[nodiscard]] static constexpr bool compare(const VkFormat& k1, const VkFormat& k2)
    {
        return k1 == k2;
    }
};

struct VkDriverRenderPassKey
{
	GPU::TextureFormat format : 16;
	GPU::LoadOp load_op : 4;
	GPU::StoreOp store_op : 4;
};

template<>
struct HashOfType<VkDriverRenderPassKey>
{
	// 0-15: texture format
	// 16-19: load op
	// 20-23: store op
	[[nodiscard]] static constexpr u64 hashfunc(const VkDriverRenderPassKey& k)
    {
        return u64(k.format)
			| (u64(k.load_op) << (16))
			| u64(k.store_op) << (16 + 4);
    }

    [[nodiscard]] static constexpr bool compare(const VkDriverRenderPassKey& k1, const VkDriverRenderPassKey& k2)
    {
        return k1.format == k2.format
			&& k1.load_op == k2.load_op
			&& k1.store_op == k2.store_op;
    }
};

struct VulkanDriver
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
			GPU::QueueUsage usage;
			uint32_t vk_family_index;
			Slice<VkQueue> vk_queues;
		};

		struct DeviceQueue
		{
			usize family_index;
		};

		Slice<QueueFamily> families;
		Slice<DeviceQueue> device_queues;

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

		usize device_queue_index;
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
		VkImageView vk_image_view;
		VkImageViewCreateInfo vk_image_view_info;
		
		VkFormat vk_format;

		GPU::TextureFormat format;
		Vector3U extent;

		GPU::DeviceID device;
		GPU::TextureID texture;
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

    struct InternalData
    {
        mem::Allocator allocator;
		mem::StackAllocator tmp_allocator;

		Slice<PhysicalDevice> physical_devices;
		Slice<GPU::PhysicalDeviceID> physical_device_ids;
		FreeList<Surface, GPU::SurfaceID> surfaces;
		FreeList<LogicalDevice, GPU::DeviceID> devices;
		FreeList<SwapChain, GPU::SwapChainID> swap_chains;
		FreeList<Fence, GPU::FenceID> fences;
		FreeList<Semaphore, GPU::SemaphoreID> semaphores;
		FreeList<Queue, GPU::QueueID> queues;
		FreeList<MemoryHeap, GPU::MemoryHeapID> memory_heaps;
		FreeList<Buffer, GPU::BufferID> buffers;
		FreeList<Sampler, GPU::SamplerID> samplers;
		FreeList<Texture, GPU::TextureID> textures;
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

		MemoryAddress vk_lib;

		VkInstance instance;
		VkSurfaceKHR dummy_surface;
		VkDebugUtilsMessengerEXT messenger;
    };

    static inline InternalData data;

    [[nodiscard]] static mem::Allocator get_allocator() { return data.allocator; }
	[[nodiscard]]static mem::Allocator acquire_tmp_allocator()
	{
		data.tmp_allocator.reset();
		return data.tmp_allocator.allocator();
	}

    static InternalGPU::GPUAdapter get_adapter();

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

	static Slice<GPU::PhysicalDeviceID> physical_devices_enumerate();
	static GPU::PhysicalDeviceInfo physical_device_get_info(GPU::PhysicalDeviceID physical_device);

	static GPU::SurfaceID surface_create(const GPU::SurfaceCreateInfo& ci);
	static void surface_destroy(GPU::SurfaceID surface);

	static GPU::DeviceID device_create(const GPU::DeviceCreateInfo& ci);
	static void device_destroy(GPU::DeviceID device);

	static GPU::SwapChainID swap_chain_create(const GPU::SwapChainCreateInfo& ci);
	static void swap_chain_destroy(GPU::SwapChainID swap_chain);
	static u32 swap_chain_get_image_count(GPU::SwapChainID swap_chain);
	static GPU::TextureID swap_chain_get_texture(GPU::SwapChainID swap_chain, u32 image_index);
	static GPU::AcquireResult swap_chain_acquire_next_image(GPU::SwapChainID swap_chain, const GPU::AcquireInfo& acquire_info, u32* image_index);

	static GPU::FenceID fence_create(const GPU::FenceCreateInfo& ci);
	static void fence_destroy(GPU::FenceID fence);
	static bool fence_get_state(GPU::FenceID fence);
	static void fence_reset(Slice<GPU::FenceID> fences);
	static void fence_wait_for(Slice<GPU::FenceID> fences, bool wait_for_all, u64 timeout);

	static GPU::SemaphoreID semaphore_create(const GPU::SemaphoreCreateInfo& ci);
	static void semaphore_destroy(GPU::SemaphoreID semaphore);

	static GPU::QueueID queue_create(const GPU::QueueCreateInfo& ci);
	static void queue_destroy(GPU::QueueID queue);
	static void queue_execute_command_buffer(GPU::QueueID queue, const GPU::QueueExecuteInfo& execute_info);
	static GPU::AcquireResult queue_present(GPU::QueueID queue, const GPU::QueuePresentInfo& present_info);
	static void queue_wait_idle(GPU::QueueID queue);

	static GPU::MemoryHeapID memory_heap_create(const GPU::MemoryHeapCreateInfo& ci);
	static void memory_heap_destroy(GPU::MemoryHeapID memory_heap);
	static Slice<u8> memory_heap_map(GPU::MemoryHeapID memory_heap, usize offset, usize len);
	static void memory_heap_unmap(GPU::MemoryHeapID memory_heap, const Slice<u8>& memory);

	static GPU::BufferID buffer_create(const GPU::BufferCreateInfo& ci);
	static void buffer_destroy(GPU::BufferID buffer);
	static GPU::MemoryRequirements buffer_get_memory_requirements(GPU::BufferID buffer);
	static void buffer_bind_memory_heap(GPU::BufferID buffer, const GPU::BindMemoryInfo& bind_info);

	static GPU::SamplerID sampler_create(const GPU::SamplerCreateInfo& ci);
	static void sampler_destroy(GPU::SamplerID sampler);
	
	static GPU::TextureID texture_create(const GPU::TextureCreateInfo& ci);
	static void texture_destroy(GPU::TextureID texture);
	static GPU::MemoryRequirements texture_get_memory_requirements(GPU::TextureID texture);
	static void texture_bind_memory_heap(GPU::TextureID texture, const GPU::BindMemoryInfo& bind_info);

	static GPU::DescriptorSetLayoutID descriptor_set_layout_create(const GPU::DescriptorSetLayoutCreateInfo& ci);
	static void descriptor_set_layout_destroy(GPU::DescriptorSetLayoutID descriptor_set_layout);

	static GPU::DescriptorPoolID descriptor_pool_create(const GPU::DescriptorPoolCreateInfo& ci);
	static void descriptor_pool_destroy(GPU::DescriptorPoolID descriptor_pool);

	static GPU::DescriptorSetID descriptor_set_allocate(const GPU::DescriptorSetAllocateInfo& ci);
	static void descriptor_set_free(GPU::DescriptorSetID descriptor_set);
	static void descriptor_set_update_descriptors(GPU::DescriptorSetID descriptor_set, const GPU::UpdateDescriptorInfo& update_info);

	static GPU::PipelineLayoutID pipeline_layout_create(const GPU::PipelineLayoutCreateInfo& ci);
	static void pipeline_layout_destroy(GPU::PipelineLayoutID pipeline_layout);

	static GPU::PipelineID pipeline_create(const GPU::PipelineCreateInfo& ci);
	static void pipeline_destroy(GPU::PipelineID pipeline);

	static GPU::CommandPoolID command_pool_create(const GPU::CommandPoolCreateInfo& ci);
	static void command_pool_destroy(GPU::CommandPoolID command_pool);

	static GPU::CommandBufferID command_buffer_allocate(const GPU::CommandBufferAllocateInfo& ci);
	static void command_buffer_free(GPU::CommandBufferID command_buffer);

	static void command_buffer_begin(GPU::CommandBufferID command_buffer);
	static void command_buffer_end(GPU::CommandBufferID command_buffer);

	static void command_buffer_begin_renderpass(GPU::CommandBufferID command_buffer, const GPU::RenderPassBeginInfo& begin_info);
	static void command_buffer_end_renderpass(GPU::CommandBufferID command_buffer, const GPU::RenderPassEndInfo& end_info);

	static void command_buffer_memory_barrier(GPU::CommandBufferID command_buffer, const GPU::PipelineMemoryBarrier& memory_barrier);
	static void command_buffer_buffer_barrier(GPU::CommandBufferID command_buffer, const GPU::PipelineBufferBarrier& buffer_barrier);
	static void command_buffer_texture_barrier(GPU::CommandBufferID command_buffer, const GPU::PipelineTextureBarrier& texture_barrier);

	static void command_buffer_copy_buffer_to_texture(GPU::CommandBufferID command_buffer, const GPU::CopyBufferToTextureInfo& copy_info);
	static void command_buffer_copy_buffer(GPU::CommandBufferID command_buffer, const GPU::BufferCopyInfo& copy_info);

	static void command_buffer_bind_pipeline(GPU::CommandBufferID command_buffer, GPU::PipelineBindPoint bind_point, GPU::PipelineID pipeline);
	static void command_buffer_bind_descriptor_sets(GPU::CommandBufferID command_buffer, GPU::PipelineBindPoint bind_point, GPU::PipelineLayoutID pipeline_layout, u32 base_set, const Slice<GPU::DescriptorSetID>& descriptor_sets);
	static void command_buffer_bind_vertex_buffers(GPU::CommandBufferID command_buffer, u32 base_binding, const Slice<GPU::BufferID>& buffers, const Slice<usize>& offsets);
	static void command_buffer_constant_block(GPU::CommandBufferID command_buffer, GPU::PipelineLayoutID pipeline_layout, GPU::ShaderStage stages, u32 offset, u32 size, MemoryAddress block_address);

	static void command_buffer_set_viewports(GPU::CommandBufferID command_buffer, u32 base_viewport, const Slice<GPU::Viewport>& viewports);
	static void command_buffer_set_scissors(GPU::CommandBufferID command_buffer, u32 base_scissor, const Slice<GPU::Scissor>& scissors);

	static void command_buffer_draw(GPU::CommandBufferID command_buffer, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance);

	static Surface& _get_surface(GPU::SurfaceID surface) { return data.surfaces.get(surface); }
	static LogicalDevice& _get_logical_device(GPU::DeviceID device) { return data.devices.get(device); }
	static SwapChain& _get_swap_chain(GPU::SwapChainID swap_chain) { return data.swap_chains.get(swap_chain); }
	static Fence& _get_fence(GPU::FenceID fence) { return data.fences.get(fence); }
	static Semaphore& _get_semaphore(GPU::SemaphoreID semaphore) { return data.semaphores.get(semaphore); }
	static Queue& _get_queue(GPU::QueueID queue) { return data.queues.get(queue); }
	static MemoryHeap& _get_memory_heap(GPU::MemoryHeapID memory_heap) { return data.memory_heaps.get(memory_heap); }
	static Buffer& _get_buffer(GPU::BufferID buffer) { return data.buffers.get(buffer); }
	static Sampler& _get_sampler(GPU::SamplerID sampler) { return data.samplers.get(sampler); }
	static Texture& _get_texture(GPU::TextureID texture) { return data.textures.get(texture); }
	static DescriptorSetLayout& _get_descriptor_set_layout(GPU::DescriptorSetLayoutID descriptor_set_layout) { return data.descriptor_set_layouts.get(descriptor_set_layout); }
	static DescriptorPool& _get_descriptor_pool(GPU::DescriptorPoolID descriptor_pool) { return data.descriptor_pools.get(descriptor_pool); }
	static DescriptorSet& _get_descriptor_set(GPU::DescriptorSetID descriptor_set) { return data.descriptor_sets.get(descriptor_set); }
	static PipelineLayout& _get_pipeline_layout(GPU::PipelineLayoutID pipeline_layout) { return data.pipeline_layouts.get(pipeline_layout); }
	static Pipeline& _get_pipeline(GPU::PipelineID pipeline) { return data.pipelines.get(pipeline); }
	static CommandPool& _get_command_pool(GPU::CommandPoolID command_pool) { return data.command_pools.get(command_pool); }
	static CommandBuffer& _get_command_buffer(GPU::CommandBufferID command_buffer) { return data.command_buffers.get(command_buffer); }

	static void _get_physical_devices();
	
	static void _vk_get_surface_format(GPU::TextureFormat surface_format, VkFormat* vk_image_format, VkColorSpaceKHR* vk_color_space);
	static VkSurfaceCapabilitiesKHR _vk_get_surface_capabilities(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface);
	static VkExtent2D _vk_get_swap_chain_extent(const Vector2U& size, const VkSurfaceCapabilitiesKHR& vk_capabilities);

	static VkShaderModule _vk_create_shader_module(LogicalDevice& ld, const GPU::ShaderStageInfo& shader_stage_info);

	static RenderPassCache& _get_render_pass_for(LogicalDevice& ld, Texture& texture, const GPU::RenderPassBeginInfo& begin_info);
	static RenderPassCache& _get_render_pass_for_pipeline(LogicalDevice& ld, const GPU::RenderingInfo& pipeline_rendering_info);
	static void _get_render_pass_and_framebuffer_for(LogicalDevice& ld, Texture& texture, 
		const GPU::RenderPassBeginInfo& begin_info, VkRenderPass* vk_render_pass, VkFramebuffer* vk_framebuffer);

	static GPU::DeviceType _vk_device_type_to_device_type(VkPhysicalDeviceType vk_device_type);
	static GPU::PresentMode _vk_present_mode_to_present_mode(VkPresentModeKHR vk_present_mode);
	static GPU::HeapUsage _vk_memory_property_to_heap_usage(VkMemoryPropertyFlags vk_memory_properties);
};
