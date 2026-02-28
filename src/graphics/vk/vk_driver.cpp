#include "graphics/vk/vk_driver.h"

#include "core/templates.h"
#include "graphics/graphics.h"
#include "graphics/vk/vk_header.h"
#include "math/funcs.h"
#include "os/os.h"



InternalGraphics::Adapter VulkanDriver::get_adapter()
{
    return InternalGraphics::Adapter
    {
        .initialize = &VulkanDriver::initialize,
        .shutdown = &VulkanDriver::shutdown,
        .physical_devices_enumerate = &VulkanDriver::physical_devices_enumerate,
        .physical_device_get_info = &VulkanDriver::physical_device_get_info,
        .device_create = &VulkanDriver::device_create,
        .device_destroy = &VulkanDriver::device_destroy,
        .surface_create = &VulkanDriver::surface_create,
        .surface_destroy = &VulkanDriver::surface_destroy,
        .swap_chain_create = &VulkanDriver::swap_chain_create,
        .swap_chain_destroy = &VulkanDriver::swap_chain_destroy,
        .swap_chain_get_render_target = &VulkanDriver::swap_chain_get_render_target,
        .swap_chain_present = &VulkanDriver::swap_chain_present,
        .buffer_create = &VulkanDriver::buffer_create,
        .buffer_destroy = &VulkanDriver::buffer_destroy,
        .buffer_map_memory = &VulkanDriver::buffer_map_memory,
        .buffer_unmap_memory = &VulkanDriver::buffer_unmap_memory,
        .texture_create = &VulkanDriver::texture_create,
        .texture_destroy = &VulkanDriver::texture_destroy,
        .texture_get_size = &VulkanDriver::texture_get_size,
        .render_target_create = &VulkanDriver::render_target_create,
        .render_target_destroy = &VulkanDriver::render_target_destroy,
        .render_target_get_texture = &VulkanDriver::render_target_get_texture,
        .pipeline_create = &VulkanDriver::pipeline_create,
        .pipeline_destroy = &VulkanDriver::pipeline_destroy,
        .command_buffer_create = &VulkanDriver::command_buffer_create,
        .command_buffer_destroy = &VulkanDriver::command_buffer_destroy,
        .command_buffer_begin = &VulkanDriver::command_buffer_begin,
        .command_buffer_blit_framebuffer = &VulkanDriver::command_buffer_blit_framebuffer,
        .command_buffer_bind_vertex_buffers = &VulkanDriver::command_buffer_bind_vertex_buffers,
        .command_buffer_bind_index_buffer = &VulkanDriver::command_buffer_bind_index_buffer,
        .command_buffer_bind_pipeline = &VulkanDriver::command_buffer_bind_pipeline,
        .command_buffer_bind_render_target = &VulkanDriver::command_buffer_bind_render_target,
        .command_buffer_set_texture_unit = &VulkanDriver::command_buffer_set_texture_unit,
        .command_buffer_set_uniform = &VulkanDriver::command_buffer_set_uniform,
        .command_buffer_set_viewport = &VulkanDriver::command_buffer_set_viewport,
        .command_buffer_clear = &VulkanDriver::command_buffer_clear,
        .command_buffer_draw = &VulkanDriver::command_buffer_draw,
        .command_buffer_draw_indexed = &VulkanDriver::command_buffer_draw_indexed,
        .command_buffer_end = &VulkanDriver::command_buffer_end,
        .queue_create = &VulkanDriver::queue_create,
        .queue_destroy = &VulkanDriver::queue_destroy,
        .queue_execute_command_buffer = &VulkanDriver::queue_execute_command_buffer,
    };
}

void VulkanDriver::initialize(const mem::Allocator &allocator)
{
    data.allocator = allocator;
    
    data.devices = FreeList<LogicalDevice, Graphics::DeviceID>::with_allocator(allocator);
    data.surfaces = FreeList<Surface, Graphics::SurfaceID>::with_allocator(allocator);
    data.swap_chains = FreeList<SwapChain, Graphics::SwapChainID>::with_allocator(allocator);
    data.vk_lib = OS::load_library("vulkan-1.dll");

    Vulkan::load_core_procs(data.vk_lib);

    data.info.api_version = Vulkan::get_api_version();
    VKDebugInfo(
        "Vulkan API Version: {}.{}.{}",
        VK_API_VERSION_MAJOR(data.info.api_version),
        VK_API_VERSION_MINOR(data.info.api_version),
        VK_API_VERSION_PATCH(data.info.api_version)
    );

    data.instance = Vulkan::create_instance();
    Vulkan::load_instance_procs(data.instance);

    // VK_ext_debug_util
#if defined(BREAD_SHOW_DEBUG_INFO)

    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info =
    {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = &Vulkan::_vk_debug_utils_callback,
        .pUserData = nullptr,
    };

    {
        VkResult result = vk.vkCreateDebugUtilsMessengerEXT(
            data.instance, &debug_messenger_info, Vulkan::allocation_callbacks(), &data.messenger
        );
        VKFailOn(result != VK_SUCCESS, "vkCreateDebugUtilsMessengerEXT({})", Vulkan::result_as_string(result));
    }
#endif

    // Getting physical devices
    _get_physical_devices();

    data.dummy_surface = Vulkan::create_surface(data.instance, 0);
}

void VulkanDriver::shutdown()
{
    vk.vkDestroySurfaceKHR(data.instance, data.dummy_surface, Vulkan::allocation_callbacks());

    data.swap_chains.destroy();
    data.surfaces.destroy();
    data.devices.destroy();

    get_allocator().free(mem::to_bytes(data.physical_device_ids));
    get_allocator().free(mem::to_bytes(data.physical_devices));

#if defined(BREAD_SHOW_DEBUG_INFO)
    vk.vkDestroyDebugUtilsMessengerEXT(
        data.instance, data.messenger, Vulkan::allocation_callbacks()
    );
#endif

    vk.vkDestroyInstance(data.instance, Vulkan::allocation_callbacks());

    OS::unload_library(data.vk_lib);
}

Slice<Graphics::PhysicalDeviceID> VulkanDriver::physical_devices_enumerate()
{
    return data.physical_device_ids;
}

Graphics::PhysicalDeviceInfo VulkanDriver::physical_device_get_info(Graphics::PhysicalDeviceID physical_device)
{
    VKFailOn(physical_device.integer() >= data.physical_devices.len);
    PhysicalDevice& pd = data.physical_devices[physical_device.integer()];

    return pd.info;
}

Graphics::DeviceID VulkanDriver::device_create(const Graphics::DeviceCreateInfo& ci)
{
    VKFailOn(
        ci.physical_device.is_valid() == false || ci.physical_device.integer() >= data.physical_devices.len, 
        "invalid physical device"
    );

    PhysicalDevice& pd = data.physical_devices[ci.physical_device.integer()];

    Graphics::DeviceID device_id = data.devices.add(LogicalDevice());
    LogicalDevice& ld = _get_logical_device(device_id);
    ld.vk_physical_device = pd.vk_physical_device;

    u32 property_count;
    vk.vkEnumerateDeviceExtensionProperties(pd.vk_physical_device, nullptr, &property_count, nullptr);
    
    Slice<VkExtensionProperties> device_extensions = get_allocator().array<VkExtensionProperties>(property_count);
    vk.vkEnumerateDeviceExtensionProperties(pd.vk_physical_device, nullptr, &property_count, device_extensions.ptr());
    
    VKDebugInfo("Device Extensions");
    for(VkExtensionProperties& extension : device_extensions)
    {
        VKDebugInfo("{}", Vulkan::vulkan_string_to_sv(extension.extensionName));
    }
    get_allocator().free(mem::to_bytes(device_extensions));

    Vulkan::check_device_extensions(pd.vk_physical_device);

    u32 family_count;
    vk.vkGetPhysicalDeviceQueueFamilyProperties(pd.vk_physical_device, &family_count, nullptr);

    Slice<VkQueueFamilyProperties> families = get_allocator().array<VkQueueFamilyProperties>(family_count);
    vk.vkGetPhysicalDeviceQueueFamilyProperties(pd.vk_physical_device, &family_count, families.ptr());    

    // 0->graphics, 1->present
    uint32_t queue_families[] = {MaxValue<uint32_t>, MaxValue<uint32_t>};
    for(usize i = 0; i < families.len; i++)
    {
        VkQueueFamilyProperties family = families[i];
        if(family.queueCount >= 1 && family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queue_families[0] = static_cast<uint32_t>(i);
            VkBool32 supported = false;
            vk.vkGetPhysicalDeviceSurfaceSupportKHR(pd.vk_physical_device, static_cast<uint32_t>(i), data.dummy_surface, &supported);
            if(supported)
            {
                queue_families[1] = static_cast<uint32_t>(i);
            }

            break;
        }
    }
    get_allocator().free(mem::to_bytes(families));

    VKFailOn(queue_families[0] != queue_families[1], "graphics and present queue must be the same for now");

    VkDeviceQueueCreateInfo queue_infos[ArraySize(queue_families)] = {};
    for(usize i = 0; i < ArraySize(queue_infos); i++)
    {
        f32 priority = 1.f;
        queue_infos[i] =
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queue_families[i],
            .queueCount = 1,
            .pQueuePriorities = &priority,
        };
    }

    VkDeviceCreateInfo device_info =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = static_cast<uint32_t>(ArraySize(queue_infos)),
        .pQueueCreateInfos = queue_infos,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(ArraySize(Vulkan::VkCoreDeviceExtensions)),
        .ppEnabledExtensionNames = Vulkan::VkCoreDeviceExtensions,
        .pEnabledFeatures = nullptr,
    };

    VkResult result = vk.vkCreateDevice(pd.vk_physical_device, &device_info, Vulkan::allocation_callbacks(), &ld.vk_device);
    VKFailOn(result != VK_SUCCESS, "vkCreateDevice({})", Vulkan::result_as_string(result));
    Vulkan::load_device_procs(ld.vk, ld.vk_device);

    ld.queue.graphics_index = queue_families[0];
    ld.queue.present_index = queue_families[1];

    ld.vk.vkGetDeviceQueue(ld.vk_device, queue_families[0], 0, &ld.queue.graphics);
    ld.vk.vkGetDeviceQueue(ld.vk_device, queue_families[1], 1, &ld.queue.present);

    return device_id;
}

void VulkanDriver::device_destroy(Graphics::DeviceID device)
{
    VKFailOn(device.is_valid() == false, "invalid device");
    LogicalDevice& ld = _get_logical_device(device);
    vk.vkDestroyDevice(ld.vk_device, Vulkan::allocation_callbacks());

    data.devices.remove(device);
}

Graphics::SurfaceID VulkanDriver::surface_create(const Graphics::SurfaceCreateInfo &ci)
{
    Graphics::SurfaceID surface_id = data.surfaces.add(Surface());
    Surface& surface = _get_surface(surface_id);
    surface.window_native_handle = ci.window_native_handle;
    surface.vk_surface = Vulkan::create_surface(data.instance, ci.window_native_handle);

    return surface_id;
}

void VulkanDriver::surface_destroy(Graphics::SurfaceID surface)
{
    VKFailOn(surface.is_valid() == false, "invalid surface");

    Surface& s = _get_surface(surface);
    vk.vkDestroySurfaceKHR(data.instance, s.vk_surface, Vulkan::allocation_callbacks());

    data.surfaces.remove(surface);
}

Graphics::SwapChainID VulkanDriver::swap_chain_create(const Graphics::SwapChainCreateInfo& ci)
{
    VKFailOn(ci.device.is_valid() == false, "invalid device");
    VKFailOn(ci.surface.is_valid() == false, "invalid surface");
    VKFailOn(ci.present_mode == Graphics::PRESENT_MODE_UNKNOWN, "invalid present mode");
    VKFailOn(ci.format == Graphics::SURFACE_FORMAT_UNKNOWN, "invalid surface format");
    VKFailOn(ci.image_count != 2 && ci.image_count != 3, "invalid image count");

    LogicalDevice& ld = _get_logical_device(ci.device);

    Graphics::SwapChainID sc_id = data.swap_chains.add(SwapChain());
    SwapChain& swap_chain = _get_swap_chain(sc_id);
    Surface& surface = _get_surface(ci.surface);

    // Creating the surface
    swap_chain.vk_device = ld.vk_device;
    swap_chain.vk_surface = surface.vk_surface;
    swap_chain.surface = ci.surface;
    swap_chain.device = ci.device;
    swap_chain.image_count = ci.image_count;

    VkFormat swapchain_format;
    VkColorSpaceKHR swap_chain_color_space;
    _surface_format_to_vk_swapchain_info(ci.format, &swapchain_format, &swap_chain_color_space);
    VkPresentModeKHR present_mode = _present_mode_to_vk_present_mode(ci.present_mode);

    VkSurfaceCapabilitiesKHR capabilities = _surface_get_capabilities(ld.vk_physical_device, surface.vk_surface);
    VkExtent2D swap_chain_extent = _swap_chain_get_extent(ci.size, capabilities);

    VkSwapchainCreateInfoKHR swap_chain_info =
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = surface.vk_surface,
        .minImageCount = ci.image_count,
        .imageFormat = swapchain_format,
        .imageColorSpace = swap_chain_color_space,
        .imageExtent = swap_chain_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = & ld.queue.present_index,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    VkResult result = ld.vk.vkCreateSwapchainKHR(ld.vk_device, &swap_chain_info, Vulkan::allocation_callbacks(), &swap_chain.vk_swapchain);
    VKFailOn(result != VK_SUCCESS, "vkCreateSwapchainKHR({})", Vulkan::result_as_string(result));

    return sc_id;
}

void VulkanDriver::swap_chain_destroy(Graphics::SwapChainID swap_chain)
{
    VKFailOn(swap_chain.is_valid() == false, "invalid swap chain");

    SwapChain& sc = _get_swap_chain(swap_chain);
    LogicalDevice& ld = _get_logical_device(sc.device);

    ld.vk.vkDestroySwapchainKHR(ld.vk_device, sc.vk_swapchain, Vulkan::allocation_callbacks());

    data.swap_chains.remove(swap_chain);
}

Graphics::RenderTargetID VulkanDriver::swap_chain_get_render_target(Graphics::SwapChainID swap_chain, usize render_target_index)
{
    Unused(swap_chain, render_target_index);
    return Graphics::RenderTargetID();
}

void VulkanDriver::swap_chain_present(Graphics::SwapChainID swap_chain, usize render_target_index)
{
    VKFailOn(swap_chain.is_valid() == false, "invalid swap chain");

    SwapChain& sc = _get_swap_chain(swap_chain);
    LogicalDevice& ld = _get_logical_device(sc.device);

    VKFailOn(render_target_index >= sc.image_count, "invalid swap chain render target index");

    VkResult result;
    uint32_t image_index = static_cast<uint32_t>(render_target_index);
    VkPresentInfoKHR present_info =
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .swapchainCount = 1,
        .pSwapchains = &sc.vk_swapchain,
        .pImageIndices = &image_index,
        .pResults = &result,
    };

    VkResult call_result = ld.vk.vkQueuePresentKHR(ld.queue.present, &present_info);
    VKFailOn(call_result != VK_SUCCESS && call_result != VK_SUBOPTIMAL_KHR, "vkQueuePresentKHR({})", Vulkan::result_as_string(call_result));
}

Graphics::BufferID VulkanDriver::buffer_create(const Graphics::BufferCreateInfo& ci)
{
    Unused(ci);
    return Graphics::BufferID();
}

void VulkanDriver::buffer_destroy(Graphics::BufferID buffer)
{
    Unused(buffer);
}

Slice<u8> VulkanDriver::buffer_map_memory(Graphics::BufferID buffer, usize offset, usize len)
{
    Unused(buffer, offset, len);
    return Slice<u8>();
}

void VulkanDriver::buffer_unmap_memory(Graphics::BufferID buffer, const Slice<u8>& memory)
{
    Unused(buffer, memory);
}

Graphics::TextureID VulkanDriver::texture_create(const Graphics::TextureCreateInfo& ci)
{
    Unused(ci);
    return Graphics::TextureID();
}

void VulkanDriver::texture_destroy(Graphics::TextureID texture)
{
    Unused(texture);
}

Vector2I VulkanDriver::texture_get_size(Graphics::TextureID texture)
{
    Unused(texture);
    return Vector2I();
}

Graphics::RenderTargetID VulkanDriver::render_target_create(const Graphics::RenderTargetCreateInfo& ci)
{
    Unused(ci);
    return Graphics::RenderTargetID();
}

void VulkanDriver::render_target_destroy(Graphics::RenderTargetID render_target)
{
    Unused(render_target);
}

Graphics::TextureID VulkanDriver::render_target_get_texture(Graphics::RenderTargetID render_target)
{
    Unused(render_target);
    return Graphics::TextureID();
}

Graphics::PipelineID VulkanDriver::pipeline_create(const Graphics::PipelineCreateInfo& ci)
{
    Unused(ci);
    return Graphics::PipelineID();
}

void VulkanDriver::pipeline_destroy(Graphics::PipelineID pipeline)
{
    Unused(pipeline);
}

Graphics::CommandBufferID VulkanDriver::command_buffer_create(const Graphics::CommandBufferCreateInfo& ci)
{
    Unused(ci);
    return Graphics::CommandBufferID();
}

void VulkanDriver::command_buffer_destroy(Graphics::CommandBufferID cmd)
{
    Unused(cmd);
}

void VulkanDriver::command_buffer_begin(Graphics::CommandBufferID cmd)
{
    Unused(cmd);
}

void VulkanDriver::command_buffer_blit_framebuffer(Graphics::CommandBufferID cmd, Graphics::RenderTargetID src_render_target, Graphics::RenderTargetID dst_render_target, Rect2DI src_rect, Rect2DI dst_rect, Graphics::TextureFilter filter)
{
    Unused(cmd, src_render_target, dst_render_target, src_rect, dst_rect, filter);
}

void VulkanDriver::command_buffer_bind_vertex_buffers(Graphics::CommandBufferID cmd, u32 binding, const Slice<Graphics::BufferID>& buffers, const Slice<u32>& offsets, const Slice<u32>& strides)
{
    Unused(cmd, binding, buffers, offsets, strides);
}

void VulkanDriver::command_buffer_bind_index_buffer(Graphics::CommandBufferID cmd, Graphics::BufferID index_buffer, u32 offset, Graphics::IndexType index_type)
{
    Unused(cmd, index_buffer, offset, index_type);
}

void VulkanDriver::command_buffer_bind_pipeline(Graphics::CommandBufferID cmd, Graphics::PipelineID pipeline)
{
    Unused(cmd, pipeline);
}

void VulkanDriver::command_buffer_bind_render_target(Graphics::CommandBufferID cmd, Graphics::RenderTargetID render_target)
{
    Unused(cmd, render_target);
}

void VulkanDriver::command_buffer_set_texture_unit(Graphics::CommandBufferID cmd, u32 set, u32 base_slot, const Slice<Graphics::TextureID>& textures)
{
    Unused(cmd, set, base_slot, textures);
}

void VulkanDriver::command_buffer_set_uniform(Graphics::CommandBufferID cmd, u32 set, u32 base_slot, const Slice<Graphics::BufferID>& buffers)
{
    Unused(cmd, set, base_slot, buffers);
}

void VulkanDriver::command_buffer_set_viewport(Graphics::CommandBufferID cmd, Rect2DI viewport_rect)
{
    Unused(cmd, viewport_rect);
}

void VulkanDriver::command_buffer_clear(Graphics::CommandBufferID cmd, Graphics::RenderTargetID render_target, Color clear_color)
{
    Unused(cmd, render_target, clear_color);
}

void VulkanDriver::command_buffer_draw(Graphics::CommandBufferID cmd, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance)
{
    Unused(cmd, vertex_count, instance_count, base_vertex, base_instance);
}

void VulkanDriver::command_buffer_draw_indexed(Graphics::CommandBufferID cmd, u32 index_count, u32 instance_count, u32 base_index, u32 base_vertex, u32 base_instance)
{
    Unused(cmd, index_count, instance_count, base_index, base_vertex, base_instance);
}

void VulkanDriver::command_buffer_end(Graphics::CommandBufferID cmd)
{
    Unused(cmd);
}

Graphics::QueueID VulkanDriver::queue_create(const Graphics::QueueCreateInfo& ci)
{
    Unused(ci);
    return Graphics::QueueID();
}

void VulkanDriver::queue_destroy(Graphics::QueueID queue)
{
    Unused(queue);
}

void VulkanDriver::queue_execute_command_buffer(Graphics::QueueID queue, const Slice<Graphics::CommandBufferID>& command_buffers)
{
    Unused(queue, command_buffers);
}

void VulkanDriver::_get_physical_devices()
{
    uint32_t physical_device_count = 0;
    vk.vkEnumeratePhysicalDevices(
        data.instance, &physical_device_count, nullptr
    );

    data.physical_devices = get_allocator().array<PhysicalDevice>(physical_device_count);
    Slice<VkPhysicalDevice> vk_physical_devices = get_allocator().array<VkPhysicalDevice>(physical_device_count);
    vk.vkEnumeratePhysicalDevices(data.instance, &physical_device_count, vk_physical_devices.ptr());

    data.physical_device_ids = get_allocator().array<Graphics::PhysicalDeviceID>(data.physical_devices.len);
    for(usize i = 0; i < data.physical_device_ids.len; i++)
    {
        data.physical_device_ids[i] = Graphics::PhysicalDeviceID(static_cast<u32>(i));
    }

    for(usize device_index = 0; device_index < data.physical_devices.len; device_index++)
    {
        PhysicalDevice& physical_device = data.physical_devices[device_index];
        physical_device.vk_physical_device = vk_physical_devices[device_index];

        VkPhysicalDeviceProperties2 properties =
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
            .pNext = nullptr,
            .properties = {},
        };
        vk.vkGetPhysicalDeviceProperties2(physical_device.vk_physical_device, &properties);
        physical_device.info.device_type = _vk_device_type_to_device_type(properties.properties.deviceType);
    }
    get_allocator().free(mem::to_bytes(vk_physical_devices));
}

void VulkanDriver::_surface_format_to_vk_swapchain_info(Graphics::SurfaceFormat sf, VkFormat* imgf, VkColorSpaceKHR* cs)
{
    switch(sf)
    {
    case Graphics::SURFACE_FORMAT_RGBA8_UNORM:
        *imgf = VK_FORMAT_R8G8B8A8_UNORM;
        *cs = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        return;
    case Graphics::SURFACE_FORMAT_RGBA8_SRGB:
        *imgf = VK_FORMAT_R8G8B8A8_SRGB;
        *cs = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        return;
    case Graphics::SURFACE_FORMAT_BGRA8_UNORM:
        *imgf = VK_FORMAT_B8G8R8A8_UNORM;
        *cs = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        return;
    case Graphics::SURFACE_FORMAT_BGRA8_SRGB:
        *imgf = VK_FORMAT_B8G8R8A8_SRGB;
        *cs = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        return;
    default:
        break;
    }

    VKFailOn(true, "invalid surface format");
}

VkPresentModeKHR VulkanDriver::_present_mode_to_vk_present_mode(Graphics::PresentMode present_mode)
{
    switch(present_mode)
    {
    case Graphics::PRESENT_MODE_IMMEDIATE:
        return VK_PRESENT_MODE_IMMEDIATE_KHR;
        case Graphics::PRESENT_MODE_VSYNC:
        return VK_PRESENT_MODE_FIFO_KHR;
    default:
        break;
    }

    VKFailOn(true, "invalid present mode");
    return VK_PRESENT_MODE_IMMEDIATE_KHR;
}

VkSurfaceCapabilitiesKHR VulkanDriver::_surface_get_capabilities(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    VkSurfaceCapabilitiesKHR capabilities;
    vk.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);
    return capabilities;   
}

VkExtent2D VulkanDriver::_swap_chain_get_extent(const Vector2I& size, const VkSurfaceCapabilitiesKHR& capabilities)
{
    if(capabilities.currentExtent.width != MaxValue<uint32_t>)
    {
        return capabilities.currentExtent;
    }
    
    VkExtent2D extent = {};
    extent.width = math::clamp(static_cast<uint32_t>(size.width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = math::clamp(static_cast<uint32_t>(size.height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return extent;
}

Graphics::DeviceType VulkanDriver::_vk_device_type_to_device_type(VkPhysicalDeviceType vk_dt)
{
    if(vk_dt == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
    {
        return Graphics::DEVICE_TYPE_INTEGRATED_GPU;
    }
    else if(vk_dt == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        return Graphics::DEVICE_TYPE_DISCRETE_GPU;
    }

    VKFailOn(true, "invalid vulkan physical device type");
    return Graphics::DEVICE_TYPE_UNKNOWN;
}

Graphics::SurfaceFormat VulkanDriver::_vk_surface_format_to_surface_format(VkSurfaceFormatKHR vk_sf)
{
    if(vk_sf.format == VK_FORMAT_R8G8B8A8_UNORM)
    {
        return Graphics::SURFACE_FORMAT_RGBA8_UNORM;
    }
    else if(vk_sf.format == VK_FORMAT_R8G8B8A8_SRGB)
    {
        return Graphics::SURFACE_FORMAT_RGBA8_SRGB;
    }
    else if(vk_sf.format == VK_FORMAT_B8G8R8A8_UNORM)
    {
        return Graphics::SURFACE_FORMAT_BGRA8_UNORM;
    }
    else if(vk_sf.format == VK_FORMAT_B8G8R8A8_SRGB)
    {
        return Graphics::SURFACE_FORMAT_BGRA8_SRGB;
    }

    VKFailOn(true, "invalid vulkan surface format");
    return Graphics::SURFACE_FORMAT_UNKNOWN;
}

Graphics::PresentMode VulkanDriver::_vk_present_mode_to_present_mode(VkPresentModeKHR vk_pm)
{
    if(vk_pm == VK_PRESENT_MODE_IMMEDIATE_KHR)
    {
        return Graphics::PRESENT_MODE_IMMEDIATE;
    }
    else if(vk_pm == VK_PRESENT_MODE_FIFO_KHR)
    {
        return Graphics::PRESENT_MODE_VSYNC;
    }

    VKFailOn(true, "invalid vulkan present mode");
    return Graphics::PRESENT_MODE_UNKNOWN;
}

