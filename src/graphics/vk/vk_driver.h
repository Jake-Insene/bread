#pragma once
#include "collections/free_list.h"
#include "collections/hash_map.h"
#include "graphics/adapter.h"
#include "graphics/vk/vk_header.h"
#include "platform/platform_header.h"



template<>
struct HashOfType<VkFormat>
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


struct VulkanDriver
{
	struct PhysicalDevice
	{
		VkPhysicalDevice vk_physical_device;
		Graphics::PhysicalDeviceInfo info;
	};

	struct Surface
	{
		VkSurfaceKHR vk_surface;
		MemoryAddress window_native_handle;
	};

	struct RenderPassCache
	{
		VkRenderPass vk_render_pass;

		Graphics::DeviceID device;
	};

	using RenderPassEntry = HashMap<VkFormat, RenderPassCache>::KeyValue;

	struct LogicalDevice
	{
		VkDevice vk_device;
		VkPhysicalDevice vk_physical_device;

		VkPhysicalDeviceFeatures vk_physical_device_features;
		VkPhysicalDeviceProperties vk_physical_device_properties;
		VkPhysicalDeviceMemoryProperties vk_physical_device_memory_properties;

		struct QueueFamily
		{
			Graphics::QueueUsage usage;
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

		VkDescriptorPool vk_global_descriptor_pool;

		// device resources
		HashMap<VkFormat, RenderPassCache> render_pass_cache;

		Graphics::DeviceID device;
	};

	struct SwapChainImage
	{
		VkImage vk_image;
		VkImageView vk_image_view;
		VkFramebuffer vk_framebuffer;
		Graphics::TextureID texture;
	};

	struct SwapChain
	{
		VkDevice vk_device;
		VkSurfaceKHR vk_surface;
		VkSwapchainKHR vk_swapchain;
		VkRenderPass vk_render_pass;

		u32 image_count;
        Slice<SwapChainImage> images;

		Graphics::SurfaceID surface;
		Graphics::DeviceID device;
	};

	struct Fence
	{
		VkDevice vk_device;
		VkFence vk_fence;

		Graphics::DeviceID device;
	};

	struct Semaphore
	{
		VkDevice vk_device;
		VkSemaphore vk_semaphore;
	
		Graphics::DeviceID device;
	};

	struct Queue
	{
		VkDevice vk_device;
		VkQueue vk_queue;

		usize device_queue_index;
		Graphics::DeviceID device;
		Graphics::QueueID queue;
	};

	struct MemoryHeap
	{
		VkDevice vk_device;
		VkDeviceMemory vk_memory;
        u32 vk_type_index;
        VkMemoryPropertyFlags vk_memory_flags;

		Graphics::DeviceID device;
		Graphics::MemoryHeapID memory_heap;
	};

	struct Buffer
	{
		VkDevice vk_device;
		VkBuffer vk_buffer;
		VkBufferView vk_buffer_view;

		Graphics::DeviceID device;
		Graphics::BufferID buffer;
		Graphics::MemoryHeapID memory_heap;
	};

	struct Sampler
	{
		VkDevice vk_device;
		VkSampler vk_sampler;

		Graphics::DeviceID device;
		Graphics::SamplerID sampler;
	};

	struct Texture
	{
		VkDevice vk_device;
		VkImage vk_image;
		VkImageView vk_image_view;

		Graphics::DeviceID device;
		Graphics::TextureID texture;
	};

	struct RenderTarget
	{
		VkDevice vk_device;
		VkFramebuffer vk_framebuffer;

		Graphics::DeviceID device;
	};

	struct DescriptorSetLayout
	{
		VkDevice vk_device;
		VkDescriptorSetLayout vk_set_layout;

		Graphics::DeviceID device;
		Graphics::DescriptorSetLayoutID descriptor_set_layout;
	};

	struct DescriptorSet
	{
		VkDevice vk_device;
		VkDescriptorSet vk_descriptor_set;

		Graphics::DeviceID device;
		Graphics::DescriptorSetID descriptor_set;
	};

	struct Pipeline
	{
		VkDevice vk_device;
		VkPipeline vk_pipeline;
		VkPipelineLayout vk_pipeline_layout;

		Graphics::DeviceID device;
		Graphics::PipelineID pipeline;
	};

	struct CommandPool
	{
		VkDevice vk_device;
		VkCommandPool vk_command_pool;
	
		Graphics::DeviceID device;
	};

	struct CommandBuffer
	{
		VkDevice vk_device;
		VkCommandPool vk_command_pool;
		VkCommandBuffer vk_command_buffer;

		Graphics::DeviceID device;
		Graphics::PipelineID last_binded_pipeline;
	};

    struct InternalData
    {
        mem::Allocator allocator;

		Slice<PhysicalDevice> physical_devices;
		Slice<Graphics::PhysicalDeviceID> physical_device_ids;
		FreeList<Surface, Graphics::SurfaceID> surfaces;
		FreeList<LogicalDevice, Graphics::DeviceID> devices;
		FreeList<SwapChain, Graphics::SwapChainID> swap_chains;
		FreeList<Fence, Graphics::FenceID> fences;
		FreeList<Semaphore, Graphics::SemaphoreID> semaphores;
		FreeList<Queue, Graphics::QueueID> queues;
		FreeList<MemoryHeap, Graphics::MemoryHeapID> memory_heaps;
		FreeList<Buffer, Graphics::BufferID> buffers;
		FreeList<Sampler, Graphics::SamplerID> samplers;
		FreeList<Texture, Graphics::TextureID> textures;
		FreeList<RenderTarget, Graphics::RenderTargetID> render_targets;
		FreeList<DescriptorSetLayout, Graphics::DescriptorSetLayoutID> descriptor_set_layouts;
		FreeList<DescriptorSet, Graphics::DescriptorSetID> descriptor_sets;
		FreeList<Pipeline, Graphics::PipelineID> pipelines;
		FreeList<CommandPool, Graphics::CommandPoolID> command_pools;
		FreeList<CommandBuffer, Graphics::CommandBufferID> command_buffers;

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

    [[nodiscard]] static mem::Allocator& get_allocator() { return data.allocator; }

    static InternalGraphics::Adapter get_adapter();

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

	static Slice<Graphics::PhysicalDeviceID> physical_devices_enumerate();
	static Graphics::PhysicalDeviceInfo physical_device_get_info(Graphics::PhysicalDeviceID physical_device);

	static Graphics::SurfaceID surface_create(const Graphics::SurfaceCreateInfo& ci);
	static void surface_destroy(Graphics::SurfaceID surface);

	static Graphics::DeviceID device_create(const Graphics::DeviceCreateInfo& ci);
	static void device_destroy(Graphics::DeviceID device);

	static Graphics::SwapChainID swap_chain_create(const Graphics::SwapChainCreateInfo& ci);
	static void swap_chain_destroy(Graphics::SwapChainID swap_chain);
	static u32 swap_chain_get_image_count(Graphics::SwapChainID swap_chain);
	static Graphics::TextureID swap_chain_get_texture(Graphics::SwapChainID swap_chain, u32 image_index);
	static void swap_chain_acquire_next_image(Graphics::SwapChainID swap_chain, const Graphics::AcquireInfo& acquire_info, u32* image_index);

	static Graphics::FenceID fence_create(const Graphics::FenceCreateInfo& ci);
	static void fence_destroy(Graphics::FenceID fence);
	static void fence_reset(Slice<Graphics::FenceID> fences);
	static void fence_wait_for(Slice<Graphics::FenceID> fences, bool wait_for_all, u64 timeout);

	static Graphics::SemaphoreID semaphore_create(const Graphics::SemaphoreCreateInfo& ci);
	static void semaphore_destroy(Graphics::SemaphoreID semaphore);

	static Graphics::QueueID queue_create(const Graphics::QueueCreateInfo& ci);
	static void queue_destroy(Graphics::QueueID queue);
	static void queue_execute_command_buffer(Graphics::QueueID queue, const Graphics::QueueExecuteInfo& execute_info);
	static void queue_present(Graphics::QueueID queue, const Graphics::QueuePresentInfo& present_info);
	static void queue_wait_idle(Graphics::QueueID queue);

	static Graphics::MemoryHeapID memory_heap_create(const Graphics::MemoryHeapCreateInfo& ci);
	static void memory_heap_destroy(Graphics::MemoryHeapID memory_heap);

	static Graphics::BufferID buffer_create(const Graphics::BufferCreateInfo& ci);
	static void buffer_destroy(Graphics::BufferID buffer);
	static Slice<u8> buffer_map_memory(Graphics::BufferID buffer, usize offset, usize len);
	static void buffer_unmap_memory(Graphics::BufferID buffer, const Slice<u8>& memory);

	static Graphics::SamplerID sampler_create(const Graphics::SamplerCreateInfo& ci);
	static void sampler_destroy(Graphics::SamplerID sampler);
	
	static Graphics::TextureID texture_create(const Graphics::TextureCreateInfo& ci);
	static void texture_destroy(Graphics::TextureID texture);
	static Vector2I texture_get_size(Graphics::TextureID texture);

	static Graphics::RenderTargetID render_target_create(const Graphics::RenderTargetCreateInfo& ci);
	static void render_target_destroy(Graphics::RenderTargetID render_target);
	static Graphics::TextureID render_target_get_texture(Graphics::RenderTargetID render_target);

	static Graphics::DescriptorSetLayoutID descriptor_set_layout_create(const Graphics::DescriptorSetLayoutCreateInfo& ci);
	static void descriptor_set_layout_destroy(Graphics::DescriptorSetLayoutID descriptor_set_layout);

	static Graphics::DescriptorSetID descriptor_set_create(const Graphics::DescriptorSetCreateInfo& ci);
	static void descriptor_set_destroy(Graphics::DescriptorSetID descriptor_set);
	static void descriptor_set_update_descriptors(Graphics::DescriptorSetID descriptor_set, const Graphics::UpdateDescriptorInfo& update_info);

	static Graphics::PipelineID pipeline_create(const Graphics::PipelineCreateInfo& ci);
	static void pipeline_destroy(Graphics::PipelineID pipeline);

	static Graphics::CommandPoolID command_pool_create(const Graphics::CommandPoolCreateInfo& ci);
	static void command_pool_destroy(Graphics::CommandPoolID command_pool);

	static Graphics::CommandBufferID command_buffer_allocate(const Graphics::CommandBufferAllocateInfo& ci);
	static void command_buffer_free(Graphics::CommandBufferID command_buffer);

	static void command_buffer_begin(Graphics::CommandBufferID command_buffer);
	static void command_buffer_end(Graphics::CommandBufferID command_buffer);

	static void command_buffer_begin_renderpass(Graphics::CommandBufferID command_buffer, const Graphics::RenderPassBeginInfo& begin_info);
	static void command_buffer_end_renderpass(Graphics::CommandBufferID command_buffer, const Graphics::RenderPassEndInfo& end_info);

	static void command_buffer_memory_barrier(Graphics::CommandBufferID command_buffer, const Graphics::PipelineMemoryBarrier& memory_barrier);
	static void command_buffer_buffer_barrier(Graphics::CommandBufferID command_buffer, const Graphics::PipelineBufferBarrier& buffer_barrier);
	static void command_buffer_texture_barrier(Graphics::CommandBufferID command_buffer, const Graphics::PipelineTextureBarrier& texture_barrier);

	static void command_buffer_copy_buffer_to_texture(Graphics::CommandBufferID command_buffer, const Graphics::CopyBufferToTextureInfo& copy_info);
	static void command_buffer_copy_buffer(Graphics::CommandBufferID command_buffer, const Graphics::BufferCopyInfo& copy_info);

	static void command_buffer_bind_pipeline(Graphics::CommandBufferID command_buffer, Graphics::PipelineBindPoint bind_point, Graphics::PipelineID pipeline);
	static void command_buffer_bind_descriptor_sets(Graphics::CommandBufferID command_buffer, Graphics::PipelineBindPoint bind_point, u32 base_set, const Slice<Graphics::DescriptorSetID>& descriptor_sets);
	static void command_buffer_bind_vertex_buffers(Graphics::CommandBufferID command_buffer, u32 base_binding, const Slice<Graphics::BufferID>& buffers, const Slice<usize>& offsets);
	static void command_buffer_constant_block(Graphics::CommandBufferID command_buffer, Graphics::PipelineID pipeline, Graphics::ShaderStage stages, u32 offset, u32 size, MemoryAddress block_address);

	static void command_buffer_set_viewports(Graphics::CommandBufferID command_buffer, u32 base_viewport, const Slice<Graphics::Viewport>& viewports);
	static void command_buffer_set_scissors(Graphics::CommandBufferID command_buffer, u32 base_scissor, const Slice<Graphics::Scissor>& scissors);

	static void command_buffer_draw(Graphics::CommandBufferID command_buffer, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance);

	static Surface& _get_surface(Graphics::SurfaceID surface) { return data.surfaces.get(surface); }
	static LogicalDevice& _get_logical_device(Graphics::DeviceID device) { return data.devices.get(device); }
	static SwapChain& _get_swap_chain(Graphics::SwapChainID swap_chain) { return data.swap_chains.get(swap_chain); }
	static Fence& _get_fence(Graphics::FenceID fence) { return data.fences.get(fence); }
	static Semaphore& _get_semaphore(Graphics::SemaphoreID semaphore) { return data.semaphores.get(semaphore); }
	static Queue& _get_queue(Graphics::QueueID queue) { return data.queues.get(queue); }
	static MemoryHeap& _get_memory_heap(Graphics::MemoryHeapID memory_heap) { return data.memory_heaps.get(memory_heap); }
	static Buffer& _get_buffer(Graphics::BufferID buffer) { return data.buffers.get(buffer); }
	static Sampler& _get_sampler(Graphics::SamplerID sampler) { return data.samplers.get(sampler); }
	static Texture& _get_texture(Graphics::TextureID texture) { return data.textures.get(texture); }
	static RenderTarget& _get_render_target(Graphics::RenderTargetID render_target) { return data.render_targets.get(render_target); }
	static DescriptorSetLayout& _get_descriptor_set_layout(Graphics::DescriptorSetLayoutID descriptor_set_layout) { return data.descriptor_set_layouts.get(descriptor_set_layout); }
	static DescriptorSet& _get_descriptor_set(Graphics::DescriptorSetID descriptor_set) { return data.descriptor_sets.get(descriptor_set); }
	static Pipeline& _get_pipeline(Graphics::PipelineID pipeline) { return data.pipelines.get(pipeline); }
	static CommandPool& _get_command_pool(Graphics::CommandPoolID command_pool) { return data.command_pools.get(command_pool); }
	static CommandBuffer& _get_command_buffer(Graphics::CommandBufferID command_buffer) { return data.command_buffers.get(command_buffer); }

	static void _get_physical_devices();
	
	static void _vk_get_surface_format(Graphics::SurfaceFormat surface_format, VkFormat* vk_image_format, VkColorSpaceKHR* vk_color_space);
	static VkSurfaceCapabilitiesKHR _vk_get_surface_capabilities(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface);
	static VkExtent2D _vk_get_swap_chain_extent(const Vector2U& size, const VkSurfaceCapabilitiesKHR& vk_capabilities);

	static VkShaderModule _vk_create_shader_module(LogicalDevice& ld, const Graphics::ShaderStageInfo& shader_stage_info);

	static RenderPassCache& _get_render_pass_for(LogicalDevice& ld, VkFormat format);

	static Graphics::DeviceType _vk_device_type_to_device_type(VkPhysicalDeviceType vk_device_type);
	static Graphics::SurfaceFormat _vk_surface_format_to_surface_format(VkSurfaceFormatKHR vk_surface_format);
	static Graphics::PresentMode _vk_present_mode_to_present_mode(VkPresentModeKHR vk_present_mode);
};
