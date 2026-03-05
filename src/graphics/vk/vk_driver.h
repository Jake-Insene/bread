#pragma once
#include "collections/free_list.h"
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

	struct LogicalDevice
	{
		VkDevice vk_device;
		VkPhysicalDevice vk_physical_device;

		struct
		{
			uint32_t graphics_index;
			uint32_t present_index;

			VkQueue graphics;
			VkQueue present;

			Graphics::QueueID graphics_queue_id;
			Graphics::QueueID present_queue_id;
		} queue;

		DeviceVulkanTable vk;
	};

	static constexpr usize MaxSwapChainImageCount = 3;

	struct SwapChainImage
	{
		VkImage vk_image;
		VkImageView vk_image_view;
		Graphics::TextureID texture;
	};

	struct SwapChain
	{
		VkDevice vk_device;
		VkSurfaceKHR vk_surface;
		VkSwapchainKHR vk_swapchain;

		u32 image_count;
		SwapChainImage images[MaxSwapChainImageCount];

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

		uint32_t queue_index;
		Graphics::DeviceID device;
	};

	struct Buffer
	{
		VkDevice vk_device;
		VkBuffer buffer;

		Graphics::DeviceID device;
	};

	struct Texture
	{
		VkDevice vk_device;
		VkImage vk_image;

		Graphics::DeviceID device;
	};

	struct RenderTarget
	{
		VkDevice vk_device;
		VkFramebuffer vk_framebuffer;

		Graphics::DeviceID device;
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
		FreeList<Buffer, Graphics::BufferID> buffers;
		FreeList<Texture, Graphics::TextureID> textures;
		FreeList<RenderTarget, Graphics::RenderTargetID> render_targets;
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

	static void command_buffer_blit_framebuffer(Graphics::CommandBufferID command_buffer, Graphics::RenderTargetID src_render_target, Graphics::RenderTargetID dst_render_target, Rect2DI src_rect, Rect2DI dst_rect, Graphics::TextureFilter filter);
	static void command_buffer_bind_vertex_buffers(Graphics::CommandBufferID command_buffer, u32 binding, const Slice<Graphics::BufferID>& buffers, const Slice<u32>& offsets, const Slice<u32>& strides);
	static void command_buffer_bind_index_buffer(Graphics::CommandBufferID command_buffer, Graphics::BufferID index_buffer, u32 offset, Graphics::IndexType index_type);
	static void command_buffer_bind_pipeline(Graphics::CommandBufferID command_buffer, Graphics::PipelineID pipeline);
	static void command_buffer_bind_render_target(Graphics::CommandBufferID command_buffer, Graphics::RenderTargetID render_target);
	static void command_buffer_set_texture_unit(Graphics::CommandBufferID command_buffer, u32 set, u32 base_slot, const Slice<Graphics::TextureID>& textures);
	static void command_buffer_set_uniform(Graphics::CommandBufferID command_buffer, u32 set, u32 base_slot, const Slice<Graphics::BufferID>& buffers);
	static void command_buffer_set_viewport(Graphics::CommandBufferID command_buffer, Rect2DI viewport_rect);
	static void command_buffer_clear(Graphics::CommandBufferID command_buffer, Graphics::RenderTargetID render_target, Color clear_color);
	static void command_buffer_draw(Graphics::CommandBufferID command_buffer, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance);
	static void command_buffer_draw_indexed(Graphics::CommandBufferID command_buffer, u32 index_count, u32 instance_count, u32 base_index, u32 base_vertex, u32 base_instance);

	static Surface& _get_surface(Graphics::SurfaceID surface) { return data.surfaces.get(surface); }
	static LogicalDevice& _get_logical_device(Graphics::DeviceID device) { return data.devices.get(device); }
	static SwapChain& _get_swap_chain(Graphics::SwapChainID swap_chain) { return data.swap_chains.get(swap_chain); }
	static Fence& _get_fence(Graphics::FenceID fence) { return data.fences.get(fence); }
	static Semaphore& _get_semaphore(Graphics::SemaphoreID semaphore) { return data.semaphores.get(semaphore); }
	static Queue& _get_queue(Graphics::QueueID queue) { return data.queues.get(queue); }
	static Buffer& _get_buffer(Graphics::BufferID buffer) { return data.buffers.get(buffer); }
	static Texture& _get_texture(Graphics::TextureID texture) { return data.textures.get(texture); }
	static RenderTarget& _get_render_target(Graphics::RenderTargetID render_target) { return data.render_targets.get(render_target); }
	static CommandPool& _get_command_pool(Graphics::CommandPoolID command_pool) { return data.command_pools.get(command_pool); }
	static CommandBuffer& _get_command_buffer(Graphics::CommandBufferID command_buffer) { return data.command_buffers.get(command_buffer); }

	static void _get_physical_devices();
	
	static void _surface_format_to_vk_swapchain_info(Graphics::SurfaceFormat surface_format, VkFormat* vk_image_format, VkColorSpaceKHR* vk_color_space);
	static VkPresentModeKHR _present_mode_to_vk_present_mode(Graphics::PresentMode present_mode);
	static VkSurfaceCapabilitiesKHR _surface_get_capabilities(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface);
	static VkExtent2D _swap_chain_get_vk_extent(const Vector2I& size, const VkSurfaceCapabilitiesKHR& vk_capabilities);
	static VkPipelineStageFlags _vk_get_pipeline_stages(Graphics::PipelineStages stages);
	static VkImageAspectFlags _vk_get_aspect_masks(Graphics::TextureAspects aspects);
	static VkAccessFlags _vk_get_access_masks(Graphics::AccessMasks access_masks);
	static VkImageLayout _vk_get_image_layout(Graphics::TextureLayout texture_layout);

	static Graphics::DeviceType _vk_device_type_to_device_type(VkPhysicalDeviceType vk_dt);
	static Graphics::SurfaceFormat _vk_surface_format_to_surface_format(VkSurfaceFormatKHR vk_sf);
	static Graphics::PresentMode _vk_present_mode_to_present_mode(VkPresentModeKHR vk_pm);
};
