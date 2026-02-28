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
		} queue;

		DeviceVulkanTable vk;
	};

	struct Surface
	{
		VkSurfaceKHR vk_surface;
		MemoryAddress window_native_handle;
	};

	struct SwapChain
	{
		VkDevice vk_device;
		VkSurfaceKHR vk_surface;
		VkSwapchainKHR vk_swapchain;

		u32 image_count;

		Graphics::SurfaceID surface;
		Graphics::DeviceID device;
	};

    struct InternalData
    {
        mem::Allocator allocator;

		Slice<PhysicalDevice> physical_devices;
		Slice<Graphics::PhysicalDeviceID> physical_device_ids;
		FreeList<LogicalDevice, Graphics::DeviceID> devices;
		FreeList<Surface, Graphics::SurfaceID> surfaces;
		FreeList<SwapChain, Graphics::SwapChainID> swap_chains;

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

	static Graphics::DeviceID device_create(const Graphics::DeviceCreateInfo& ci);
	static void device_destroy(Graphics::DeviceID device);

	static Graphics::SurfaceID surface_create(const Graphics::SurfaceCreateInfo& ci);
	static void surface_destroy(Graphics::SurfaceID surface);

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

	static LogicalDevice& _get_logical_device(Graphics::DeviceID device) { return data.devices.get(device); }
	static Surface& _get_surface(Graphics::SurfaceID surface) { return data.surfaces.get(surface); }
	static SwapChain& _get_swap_chain(Graphics::SwapChainID swap_chain) { return data.swap_chains.get(swap_chain); }

	static void _get_physical_devices();
	
	static void _surface_format_to_vk_swapchain_info(Graphics::SurfaceFormat sf, VkFormat* imgf, VkColorSpaceKHR* cs);
	static VkPresentModeKHR _present_mode_to_vk_present_mode(Graphics::PresentMode present_mode);
	static VkSurfaceCapabilitiesKHR _surface_get_capabilities(VkPhysicalDevice physical_device, VkSurfaceKHR surface); 
	static VkExtent2D _swap_chain_get_extent(const Vector2I& size, const VkSurfaceCapabilitiesKHR& capabilities);

	static Graphics::DeviceType _vk_device_type_to_device_type(VkPhysicalDeviceType vk_dt);
	static Graphics::SurfaceFormat _vk_surface_format_to_surface_format(VkSurfaceFormatKHR vk_sf);
	static Graphics::PresentMode _vk_present_mode_to_present_mode(VkPresentModeKHR vk_pm);
};
