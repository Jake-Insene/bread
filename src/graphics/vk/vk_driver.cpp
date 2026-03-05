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
        .surface_create = &VulkanDriver::surface_create,
        .surface_destroy = &VulkanDriver::surface_destroy,
        .device_create = &VulkanDriver::device_create,
        .device_destroy = &VulkanDriver::device_destroy,
        .swap_chain_create = &VulkanDriver::swap_chain_create,
        .swap_chain_destroy = &VulkanDriver::swap_chain_destroy,
        .swap_chain_get_texture = &VulkanDriver::swap_chain_get_texture,
        .swap_chain_acquire_next_image = &VulkanDriver::swap_chain_acquire_next_image,
        .fence_create = &VulkanDriver::fence_create,
        .fence_destroy = &VulkanDriver::fence_destroy,
        .fence_reset = &VulkanDriver::fence_reset,
        .fence_wait_for = &VulkanDriver::fence_wait_for,
        .semaphore_create = &VulkanDriver::semaphore_create,
        .semaphore_destroy = &VulkanDriver::semaphore_destroy,
        .queue_create = &VulkanDriver::queue_create,
        .queue_destroy = &VulkanDriver::queue_destroy,
        .queue_execute_command_buffer = &VulkanDriver::queue_execute_command_buffer,
        .queue_present = &VulkanDriver::queue_present,
        .queue_wait_idle = &VulkanDriver::queue_wait_idle,
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
        .command_pool_create = &VulkanDriver::command_pool_create,
        .command_pool_destroy = &VulkanDriver::command_pool_destroy,
        .command_buffer_allocate = &VulkanDriver::command_buffer_allocate,
        .command_buffer_free = &VulkanDriver::command_buffer_free,
        .command_buffer_begin = &VulkanDriver::command_buffer_begin,
        .command_buffer_end = &VulkanDriver::command_buffer_end,
        .command_buffer_begin_renderpass = &VulkanDriver::command_buffer_begin_renderpass,
        .command_buffer_end_renderpass = &VulkanDriver::command_buffer_end_renderpass,
        .command_buffer_memory_barrier = &VulkanDriver::command_buffer_memory_barrier,
        .command_buffer_buffer_barrier = &VulkanDriver::command_buffer_buffer_barrier,
        .command_buffer_texture_barrier = &VulkanDriver::command_buffer_texture_barrier,
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
    };
}

void VulkanDriver::initialize(const mem::Allocator &allocator)
{
    data.allocator = allocator;
    
    data.surfaces = FreeList<Surface, Graphics::SurfaceID>::with_allocator(allocator);
    data.devices = FreeList<LogicalDevice, Graphics::DeviceID>::with_allocator(allocator);
    data.swap_chains = FreeList<SwapChain, Graphics::SwapChainID>::with_allocator(allocator);
    data.fences = FreeList<Fence, Graphics::FenceID>::with_allocator(allocator);
    data.semaphores = FreeList<Semaphore, Graphics::SemaphoreID>::with_allocator(allocator);
    data.queues = FreeList<Queue, Graphics::QueueID>::with_allocator(allocator);
    data.buffers = FreeList<Buffer, Graphics::BufferID>::with_allocator(allocator);
    data.textures = FreeList<Texture, Graphics::TextureID>::with_allocator(allocator);
    data.render_targets = FreeList<RenderTarget, Graphics::RenderTargetID>::with_allocator(allocator);
    data.command_pools = FreeList<CommandPool, Graphics::CommandPoolID>::with_allocator(allocator);
    data.command_buffers = FreeList<CommandBuffer, Graphics::CommandBufferID>::with_allocator(get_allocator());
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

    data.surfaces.destroy();
    data.devices.destroy();
    data.swap_chains.destroy();
    data.fences.destroy();
    data.semaphores.destroy();
    data.queues.destroy();
    data.buffers.destroy();
    data.textures.destroy();
    data.render_targets.destroy();
    data.command_pools.destroy();
    data.command_buffers.destroy();

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
    uint32_t queue_count = static_cast<uint32_t>(ArraySize(queue_families));
    if(queue_families[0] == queue_families[1])
    {
        queue_count--;
    }

    for(usize i = 0; i < queue_count; i++)
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

    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
        .pNext = nullptr,
        .dynamicRendering = VK_TRUE,
    };

    VkPhysicalDeviceFeatures2 features =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &dynamic_rendering,
        .features = {},
    };

    VkDeviceCreateInfo device_info =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features,
        .flags = 0,
        .queueCreateInfoCount = queue_count,
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
    ld.vk.vkGetDeviceQueue(ld.vk_device, queue_families[1], 0, &ld.queue.present);

    {
        ld.queue.graphics_queue_id = data.queues.add(Queue());
        Queue& graphics_queue = _get_queue(ld.queue.graphics_queue_id);
        graphics_queue.vk_device = ld.vk_device;
        graphics_queue.vk_queue = ld.queue.graphics;
        graphics_queue.device = device_id;
        graphics_queue.queue_index = ld.queue.graphics_index;
    }
    
    {
        ld.queue.present_queue_id = data.queues.add(Queue());
        Queue& present_queue = _get_queue(ld.queue.present_queue_id);
        present_queue.vk_device = ld.vk_device;
        present_queue.vk_queue = ld.queue.present;
        present_queue.device = device_id;
        present_queue.queue_index = ld.queue.present_index;
    }

    return device_id;
}

void VulkanDriver::device_destroy(Graphics::DeviceID device)
{
    VKFailOn(device.is_valid() == false, "invalid device");
    LogicalDevice& ld = _get_logical_device(device);
    
    data.queues.remove(ld.queue.graphics_queue_id);
    if(ld.queue.graphics_queue_id != ld.queue.present_queue_id)
    {
        data.queues.remove(ld.queue.present_queue_id);
    }
    
    vk.vkDestroyDevice(ld.vk_device, Vulkan::allocation_callbacks());
    
    data.devices.remove(device);
}

Graphics::SwapChainID VulkanDriver::swap_chain_create(const Graphics::SwapChainCreateInfo& ci)
{
    VKFailOn(ci.device.is_valid() == false, "invalid device");
    VKFailOn(ci.surface.is_valid() == false, "invalid surface");
    VKFailOn(ci.present_mode == Graphics::PresentMode::Unknown, "invalid present mode");
    VKFailOn(ci.format == Graphics::SurfaceFormat::Unknown, "invalid surface format");
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

    VkFormat vk_swapchain_format;
    VkColorSpaceKHR vk_swap_chain_color_space;
    _surface_format_to_vk_swapchain_info(ci.format, &vk_swapchain_format, &vk_swap_chain_color_space);
    VkPresentModeKHR vk_present_mode = _present_mode_to_vk_present_mode(ci.present_mode);

    VkSurfaceCapabilitiesKHR capabilities = _surface_get_capabilities(ld.vk_physical_device, surface.vk_surface);
    VkExtent2D vk_swap_chain_extent = _swap_chain_get_vk_extent(ci.size, capabilities);

    VkSwapchainCreateInfoKHR swap_chain_info =
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = surface.vk_surface,
        .minImageCount = ci.image_count,
        .imageFormat = vk_swapchain_format,
        .imageColorSpace = vk_swap_chain_color_space,
        .imageExtent = vk_swap_chain_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = & ld.queue.present_index,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = vk_present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    VkResult result = ld.vk.vkCreateSwapchainKHR(ld.vk_device, &swap_chain_info, Vulkan::allocation_callbacks(), &swap_chain.vk_swapchain);
    VKFailOn(result != VK_SUCCESS, "vkCreateSwapchainKHR({})", Vulkan::result_as_string(result));

    {
        // image
        VkImage vk_images[MaxSwapChainImageCount] = {};
        ld.vk.vkGetSwapchainImagesKHR(ld.vk_device, swap_chain.vk_swapchain, &swap_chain.image_count, nullptr);
        ld.vk.vkGetSwapchainImagesKHR(ld.vk_device, swap_chain.vk_swapchain, &swap_chain.image_count, vk_images);
        for (u32 i = 0; i < swap_chain.image_count; i++)
        {
            swap_chain.images[i].vk_image = vk_images[i];
        }
    }

    {
        VkImageViewCreateInfo image_view_info =
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = VK_NULL_HANDLE,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = vk_swapchain_format,
            .components =
            {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = 
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel  = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        // image views
        for(u32 i = 0; i < swap_chain.image_count; i++)
        {
            image_view_info.image = swap_chain.images[i].vk_image;
            ld.vk.vkCreateImageView(ld.vk_device, &image_view_info, Vulkan::allocation_callbacks(), &swap_chain.images[i].vk_image_view);
        }

        // texture object
        for (u32 i = 0; i < swap_chain.image_count; i++)
        {
            swap_chain.images[i].texture = data.textures.add(Texture());
            Texture& tex = _get_texture(swap_chain.images[i].texture);
            tex.vk_device = swap_chain.vk_device;
            tex.vk_image = swap_chain.images[i].vk_image;
            tex.device = swap_chain.device;
        }
    }

    return sc_id;
}

void VulkanDriver::swap_chain_destroy(Graphics::SwapChainID swap_chain)
{
    VKFailOn(swap_chain.is_valid() == false, "invalid swap chain");

    SwapChain& sc = _get_swap_chain(swap_chain);
    LogicalDevice& ld = _get_logical_device(sc.device);

    for(u32 i = 0; i < sc.image_count; i++)
    {
        ld.vk.vkDestroyImageView(sc.vk_device, sc.images[i].vk_image_view, Vulkan::allocation_callbacks());
        data.textures.remove(sc.images[i].texture);
    }
    
    ld.vk.vkDestroySwapchainKHR(ld.vk_device, sc.vk_swapchain, Vulkan::allocation_callbacks());

    data.swap_chains.remove(swap_chain);
}

Graphics::TextureID VulkanDriver::swap_chain_get_texture(Graphics::SwapChainID swap_chain, u32 image_index)
{
    VKFailOn(swap_chain.is_valid() == false, "invalid swap chain");

    SwapChain& sc = _get_swap_chain(swap_chain);
    VKFailOn(image_index >= sc.image_count, "invalid image index");
    
    return sc.images[image_index].texture;
}

void VulkanDriver::swap_chain_acquire_next_image(Graphics::SwapChainID swap_chain, const Graphics::AcquireInfo& acquire_info, u32* image_index)
{
    VKFailOn(swap_chain.is_valid() == false, "invalid swap chain");
    VKFailOn(
        acquire_info.fence.is_valid() == false && acquire_info.semaphore.is_valid() == false, 
        "semaphore and fence can't be null, at least one is require"
    );

    SwapChain& sc = _get_swap_chain(swap_chain);
    LogicalDevice& ld = _get_logical_device(sc.device);

    VkSemaphore vk_semaphore = VK_NULL_HANDLE;
    if(acquire_info.semaphore.is_valid())
    {
        Semaphore& sem = _get_semaphore(acquire_info.semaphore);
        vk_semaphore = sem.vk_semaphore;
    }

    VkFence vk_fence = VK_NULL_HANDLE;
    if(acquire_info.fence.is_valid())
    {
        Fence& f = _get_fence(acquire_info.fence);
        vk_fence = f.vk_fence;
    }

    ld.vk.vkAcquireNextImageKHR(sc.vk_device, sc.vk_swapchain, acquire_info.timeout, vk_semaphore, vk_fence, image_index);
}

Graphics::FenceID VulkanDriver::fence_create(const Graphics::FenceCreateInfo& ci)
{
    VKFailOn(ci.device.is_valid() == false, "invalid device");

    LogicalDevice& ld = _get_logical_device(ci.device);
    
    Graphics::FenceID fence_id = data.fences.add(Fence());
    Fence& fence = _get_fence(fence_id);

    fence.vk_device = ld.vk_device;
    fence.device = ci.device;

    VkFenceCreateInfo fence_info =
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = ci.signaled == true ? VK_FENCE_CREATE_SIGNALED_BIT : VkFenceCreateFlags(0),
    };
    
    VkResult result = ld.vk.vkCreateFence(ld.vk_device, &fence_info, Vulkan::allocation_callbacks(), &fence.vk_fence);
    VKFailOn(result != VK_SUCCESS, "vkCreateFence({})", Vulkan::result_as_string(result));
    fence.device = ci.device;

    return fence_id;
}

void VulkanDriver::fence_destroy(Graphics::FenceID fence)
{
    VKFailOn(fence.is_valid() == false, "invalid fence");

    Fence& f = _get_fence(fence);
    LogicalDevice& ld = _get_logical_device(f.device);

    ld.vk.vkDestroyFence(f.vk_device, f.vk_fence, Vulkan::allocation_callbacks());

    data.fences.remove(fence);
}

void VulkanDriver::fence_reset(Slice<Graphics::FenceID> fences)
{
    VKFailOn(fences.len == 0, "at least one fence is expected");

    Graphics::DeviceID first_device = _get_fence(fences[0]).device;
    Slice<VkFence> vk_fences = get_allocator().array<VkFence>(fences.len);
    LogicalDevice& ld = _get_logical_device(first_device);

    for(usize i = 0; i < fences.len; i++)
    {
        Fence& fence = _get_fence(fences[i]);
        VKFailOn(first_device != fence.device, "fences must share the same device");

        vk_fences[i] = fence.vk_fence;
    }

    VkResult result = ld.vk.vkResetFences(ld.vk_device, static_cast<uint32_t>(vk_fences.len), vk_fences.ptr());
    VKFailOn(result != VK_SUCCESS, "vkResetFences({})", Vulkan::result_as_string(result));

    get_allocator().free(mem::to_bytes(vk_fences));
}

void VulkanDriver::fence_wait_for(Slice<Graphics::FenceID> fences, bool wait_for_all, u64 timeout)
{
     VKFailOn(fences.len == 0, "at least one fence is expected");

    Graphics::DeviceID first_device = _get_fence(fences[0]).device;
    Slice<VkFence> vk_fences = get_allocator().array<VkFence>(fences.len);
    LogicalDevice& ld = _get_logical_device(first_device);

    for(usize i = 0; i < fences.len; i++)
    {
        Fence& fence = _get_fence(fences[i]);
        VKFailOn(first_device != fence.device, "fences must share the same device");

        vk_fences[i] = fence.vk_fence;
    }

    VkResult result = ld.vk.vkWaitForFences(
        ld.vk_device, static_cast<uint32_t>(vk_fences.len), vk_fences.ptr(),
        wait_for_all == true ? VK_TRUE : VK_FALSE, timeout
    );
    VKFailOn(result != VK_SUCCESS, "vkWaitForFences({})", Vulkan::result_as_string(result));   

    get_allocator().free(mem::to_bytes(vk_fences));
}

Graphics::SemaphoreID VulkanDriver::semaphore_create(const Graphics::SemaphoreCreateInfo &ci)
{
    VKFailOn(ci.device.is_valid() == false, "invalid device");

    Graphics::SemaphoreID semaphore_id = data.semaphores.add(Semaphore());
    Semaphore& sem = _get_semaphore(semaphore_id);
    LogicalDevice& ld = _get_logical_device(ci.device);

    sem.vk_device = ld.vk_device;
    sem.device = ci.device;

    VkSemaphoreCreateInfo semaphore_info =
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    VkResult result = ld.vk.vkCreateSemaphore(ld.vk_device, &semaphore_info, Vulkan::allocation_callbacks(), &sem.vk_semaphore);
    VKFailOn(result != VK_SUCCESS, "vkCreateSemaphore({})", Vulkan::result_as_string(result));   

    return semaphore_id;
}

void VulkanDriver::semaphore_destroy(Graphics::SemaphoreID semaphore)
{
    VKFailOn(semaphore.is_valid() == false, "invalid semaphore");

    Semaphore& sem = _get_semaphore(semaphore);
    LogicalDevice& ld = _get_logical_device(sem.device);

    ld.vk.vkDestroySemaphore(sem.vk_device, sem.vk_semaphore, Vulkan::allocation_callbacks());
    
    data.semaphores.remove(semaphore);
}

Graphics::QueueID VulkanDriver::queue_create(const Graphics::QueueCreateInfo& ci)
{
    VKFailOn(ci.usage == Graphics::QueueUsage::Unknown, "invalid queue usage");

    LogicalDevice& ld = _get_logical_device(ci.device);

    if(ci.usage == Graphics::QueueUsage::Graphics)
    {
        return ld.queue.graphics_queue_id;
    }
    else if(ci.usage == Graphics::QueueUsage::Present)
    {
        return ld.queue.present_queue_id;
    }

    VKFailOn(true, "invalid queue usage");
    return Graphics::QueueID();
}

void VulkanDriver::queue_destroy(Graphics::QueueID queue)
{
    Unused(queue);
}

void VulkanDriver::queue_execute_command_buffer(Graphics::QueueID queue, const Graphics::QueueExecuteInfo& execute_info)
{
    VKFailOn(queue.is_valid() == false, "invalid queue");

    Queue& q = _get_queue(queue);
    LogicalDevice& ld = _get_logical_device(q.device);

    Slice<VkSemaphore> vk_wait_sem = get_allocator().array<VkSemaphore>(execute_info.wait_semaphores.len);
    for(usize i = 0; i < execute_info.wait_semaphores.len; i++)
    {
        Semaphore& sem = _get_semaphore(execute_info.wait_semaphores[i]);
        vk_wait_sem[i] = sem.vk_semaphore;
    }

    Slice<VkSemaphore> vk_signal_sem = get_allocator().array<VkSemaphore>(execute_info.signal_semaphores.len);
    for(usize i = 0; i < execute_info.signal_semaphores.len; i++)
    {
        Semaphore& sem = _get_semaphore(execute_info.signal_semaphores[i]);
        vk_signal_sem[i] = sem.vk_semaphore;
    }

    Slice<VkCommandBuffer> vk_cmd_buffers = get_allocator().array<VkCommandBuffer>(execute_info.command_buffers.len);
    for(usize i = 0; i < execute_info.command_buffers.len; i++)
    {
        CommandBuffer& cmd_buffer = _get_command_buffer(execute_info.command_buffers[i]);
        vk_cmd_buffers[i] = cmd_buffer.vk_command_buffer;
    }

    VkFence vk_fence = VK_NULL_HANDLE;
    if(execute_info.fence.is_valid())
    {
        Fence& f = _get_fence(execute_info.fence);
        vk_fence = f.vk_fence;
    }

    VkPipelineStageFlags wait_dest_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = static_cast<uint32_t>(vk_wait_sem.len),
        .pWaitSemaphores = vk_wait_sem.ptr(),
        .pWaitDstStageMask = &wait_dest_mask,
        .commandBufferCount = static_cast<uint32_t>(vk_cmd_buffers.len),
        .pCommandBuffers = vk_cmd_buffers.ptr(),
        .signalSemaphoreCount = static_cast<uint32_t>(vk_signal_sem.len),
        .pSignalSemaphores = vk_signal_sem.ptr(),
    };

    VkResult result = ld.vk.vkQueueSubmit(q.vk_queue, 1, &submit_info, vk_fence);
    VKFailOn(result != VK_SUCCESS, "vkQueueSubmit({})", Vulkan::result_as_string(result));

    get_allocator().free(mem::to_bytes(vk_wait_sem));
    get_allocator().free(mem::to_bytes(vk_signal_sem));
    get_allocator().free(mem::to_bytes(vk_cmd_buffers));
}

void VulkanDriver::queue_present(Graphics::QueueID queue, const Graphics::QueuePresentInfo& present_info)
{
    VKFailOn(queue.is_valid() == false, "invalid queue");

    Queue& q = _get_queue(queue);
    LogicalDevice& ld = _get_logical_device(q.device);

    Slice<VkSemaphore> vk_wait_semaphores = get_allocator().array<VkSemaphore>(present_info.wait_semaphores.len);
    for(usize i = 0; i < present_info.wait_semaphores.len; i++)
    {
        Semaphore& sem = _get_semaphore(present_info.wait_semaphores[i]);
        vk_wait_semaphores[i] = sem.vk_semaphore;
    }

    Slice<VkSwapchainKHR> vk_swapchains = get_allocator().array<VkSwapchainKHR>(present_info.swapchains.len);
    for(usize i = 0; i < present_info.swapchains.len; i++)
    {
        SwapChain& sc = _get_swap_chain(present_info.swapchains[i]);
        vk_swapchains[i] = sc.vk_swapchain;
    }

    VkResult result;
    VkPresentInfoKHR vk_present_info =
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = static_cast<uint32_t>(vk_wait_semaphores.len),
        .pWaitSemaphores = vk_wait_semaphores.ptr(),
        .swapchainCount = static_cast<uint32_t>(vk_swapchains.len),
        .pSwapchains = vk_swapchains.ptr(),
        .pImageIndices = present_info.image_indices.ptr(),
        .pResults = &result,
    };

    VkResult call_result = ld.vk.vkQueuePresentKHR(q.vk_queue, &vk_present_info);
    VKFailOn(call_result != VK_SUCCESS && call_result != VK_SUBOPTIMAL_KHR, "vkQueuePresentKHR({})", Vulkan::result_as_string(call_result));

    get_allocator().free(mem::to_bytes(vk_wait_semaphores));
    get_allocator().free(mem::to_bytes(vk_swapchains));
}

void VulkanDriver::queue_wait_idle(Graphics::QueueID queue)
{
    VKFailOn(queue.is_valid() == false, "invalid queue");

    Queue& q = _get_queue(queue);
    LogicalDevice& ld = _get_logical_device(q.device);

    VkResult result = ld.vk.vkQueueWaitIdle(q.vk_queue);
    VKFailOn(result != VK_SUCCESS, "vkQueueWaitIdle({})", Vulkan::result_as_string(result));
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

Graphics::CommandPoolID VulkanDriver::command_pool_create(const Graphics::CommandPoolCreateInfo& ci)
{
    VKFailOn(ci.device.is_valid() == false, "invalid device");
    VKFailOn(ci.queue.is_valid() == false, "invalid queue");

    LogicalDevice& ld = _get_logical_device(ci.device);
    Queue& queue = _get_queue(ci.queue);
    
    Graphics::CommandPoolID cmd_pool_id = data.command_pools.add(CommandPool());
    CommandPool& cmd_pool = _get_command_pool(cmd_pool_id);

    VkCommandPoolCreateInfo cmd_pool_info =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue.queue_index,
    };

    VkResult result = ld.vk.vkCreateCommandPool(ld.vk_device, &cmd_pool_info, Vulkan::allocation_callbacks(), &cmd_pool.vk_command_pool);
    VKFailOn(result != VK_SUCCESS, "vkCreateCommandPool({})", Vulkan::result_as_string(result));

    cmd_pool.vk_device = ld.vk_device;
    cmd_pool.device = ci.device;

    return cmd_pool_id;
}

void VulkanDriver::command_pool_destroy(Graphics::CommandPoolID command_pool)
{
    VKFailOn(command_pool.is_valid() == false, "invalid command pool");

    CommandPool& cmd_pool = _get_command_pool(command_pool);
    LogicalDevice& ld = _get_logical_device(cmd_pool.device);

    ld.vk.vkDestroyCommandPool(cmd_pool.vk_device, cmd_pool.vk_command_pool, Vulkan::allocation_callbacks());

    data.command_pools.remove(command_pool);
}

Graphics::CommandBufferID VulkanDriver::command_buffer_allocate(const Graphics::CommandBufferAllocateInfo& ci)
{
    VKFailOn(ci.pool.is_valid() == false, "invalid command pool");

    CommandPool& cmd_pool = _get_command_pool(ci.pool);
    LogicalDevice& ld = _get_logical_device(cmd_pool.device);

    Graphics::CommandBufferID cmd_buffer_id = data.command_buffers.add(CommandBuffer());
    CommandBuffer& cmd_buffer = _get_command_buffer(cmd_buffer_id);

    cmd_buffer.vk_device = cmd_pool.vk_device;
    cmd_buffer.vk_command_pool = cmd_pool.vk_command_pool;
    cmd_buffer.device = cmd_pool.device;

    VkCommandBufferAllocateInfo cmd_buffer_info =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = cmd_pool.vk_command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkResult result = ld.vk.vkAllocateCommandBuffers(ld.vk_device, &cmd_buffer_info, &cmd_buffer.vk_command_buffer);
    VKFailOn(result != VK_SUCCESS, "vkAllocateCommandBuffers({})", Vulkan::result_as_string(result));

    return cmd_buffer_id;
}

void VulkanDriver::command_buffer_free(Graphics::CommandBufferID command_buffer)
{
    VKFailOn(command_buffer.is_valid() == false, "invalid command buffer");

    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    ld.vk.vkFreeCommandBuffers(ld.vk_device, cmd_buffer.vk_command_pool, 1, &cmd_buffer.vk_command_buffer);
    
    data.command_buffers.remove(command_buffer);
}

void VulkanDriver::command_buffer_begin(Graphics::CommandBufferID command_buffer)
{
    VKFailOn(command_buffer.is_valid() == false, "invalid command buffer");

    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    VkCommandBufferBeginInfo begin_info =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr,
    };

    VkResult result = ld.vk.vkBeginCommandBuffer(cmd_buffer.vk_command_buffer, &begin_info);
    VKFailOn(result != VK_SUCCESS, "vkBeginCommandBuffer({})", Vulkan::result_as_string(result));
}

void VulkanDriver::command_buffer_end(Graphics::CommandBufferID command_buffer)
{
    VKFailOn(command_buffer.is_valid() == false, "invalid command buffer");

    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    VkResult result = ld.vk.vkEndCommandBuffer(cmd_buffer.vk_command_buffer);
    VKFailOn(result != VK_SUCCESS, "vkEndCommandBuffer({})", Vulkan::result_as_string(result));
}

void VulkanDriver::command_buffer_begin_renderpass(Graphics::CommandBufferID command_buffer, const Graphics::RenderPassBeginInfo& begin_info)
{
    VKFailOn(command_buffer.is_valid() == false, "invalid command buffer");
    VKFailOn(begin_info.swap_chain.is_valid() == false, "invalid swap chain");

    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    SwapChain& sc = _get_swap_chain(begin_info.swap_chain);

    VkClearValue clear_value = {};
    clear_value.color.float32[0] = begin_info.clear_color.r / 255.f;
    clear_value.color.float32[1] = begin_info.clear_color.g / 255.f;
    clear_value.color.float32[2] = begin_info.clear_color.b / 255.f;
    clear_value.color.float32[3] = begin_info.clear_color.a / 255.f;

    VkRect2D render_area =
    {
        .offset = {.x = 0, .y = 0},
        .extent = 
        {
            .width = static_cast<uint32_t>(begin_info.size.width),
            .height = static_cast<uint32_t>(begin_info.size.height)
        },
    };

    VKFailOn(begin_info.image_index >= sc.image_count, "invalid image index");
    VkImageView vk_image_view = sc.images[begin_info.image_index].vk_image_view;

    VkRenderingAttachmentInfo color_attachment =
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = vk_image_view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .resolveMode = VK_RESOLVE_MODE_NONE,
        .resolveImageView = VK_NULL_HANDLE,
        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = clear_value,
    };

    VkRenderingInfoKHR rendering_info =
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .renderArea = render_area,
        .layerCount = 1,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment,
        .pDepthAttachment = nullptr,
        .pStencilAttachment = nullptr,
    };

    ld.vk.vkCmdBeginRenderingKHR(cmd_buffer.vk_command_buffer, &rendering_info);
}

void VulkanDriver::command_buffer_end_renderpass(Graphics::CommandBufferID command_buffer, const Graphics::RenderPassEndInfo& end_info)
{
    VKFailOn(command_buffer.is_valid() == false, "invalid command buffer");
    Unused(end_info);

    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    ld.vk.vkCmdEndRendering(cmd_buffer.vk_command_buffer);
}

void VulkanDriver::command_buffer_memory_barrier(Graphics::CommandBufferID command_buffer, const Graphics::PipelineMemoryBarrier& memory_barrier)
{
    VKFailOn(command_buffer.is_valid() == false, "invalid command buffer");
    Unused(memory_barrier);
}

void VulkanDriver::command_buffer_buffer_barrier(Graphics::CommandBufferID command_buffer, const Graphics::PipelineBufferBarrier& buffer_barrier)
{
    VKFailOn(command_buffer.is_valid() == false, "invalid command buffer");
    Unused(buffer_barrier);
}

void VulkanDriver::command_buffer_texture_barrier(Graphics::CommandBufferID command_buffer, const Graphics::PipelineTextureBarrier& texture_barrier)
{
    VKFailOn(command_buffer.is_valid() == false, "invalid command buffer");

    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    VkPipelineStageFlags vk_src_stages = _vk_get_pipeline_stages(texture_barrier.src_stages);
    VkPipelineStageFlags vk_dest_stages = _vk_get_pipeline_stages(texture_barrier.dest_stages);

    VkImageSubresourceRange vk_subresource_range =
    {
        .aspectMask = _vk_get_aspect_masks(texture_barrier.subresource_range.aspects),
        .baseMipLevel = texture_barrier.subresource_range.base_mip_level,
        .levelCount = texture_barrier.subresource_range.level_count,
        .baseArrayLayer = texture_barrier.subresource_range.base_array_layer,
        .layerCount = texture_barrier.subresource_range.layer_count,
    };

    Texture& tex = _get_texture(texture_barrier.texture);
    VkImage vk_image = tex.vk_image;

    VkImageMemoryBarrier vk_image_barrier =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = _vk_get_access_masks(texture_barrier.src_masks),
        .dstAccessMask = _vk_get_access_masks(texture_barrier.dest_masks),
        .oldLayout = _vk_get_image_layout(texture_barrier.src_layout),
        .newLayout = _vk_get_image_layout(texture_barrier.dest_layout),
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = vk_image,
        .subresourceRange = vk_subresource_range,
    };

    ld.vk.vkCmdPipelineBarrier(
        cmd_buffer.vk_command_buffer, vk_src_stages, vk_dest_stages, 0,
        0, nullptr, 0, nullptr, 1, &vk_image_barrier
    );
}

void VulkanDriver::command_buffer_blit_framebuffer(Graphics::CommandBufferID command_buffer, Graphics::RenderTargetID src_render_target, Graphics::RenderTargetID dst_render_target, Rect2DI src_rect, Rect2DI dst_rect, Graphics::TextureFilter filter)
{
    Unused(command_buffer, src_render_target, dst_render_target, src_rect, dst_rect, filter);
}

void VulkanDriver::command_buffer_bind_vertex_buffers(Graphics::CommandBufferID command_buffer, u32 binding, const Slice<Graphics::BufferID>& buffers, const Slice<u32>& offsets, const Slice<u32>& strides)
{
    Unused(command_buffer, binding, buffers, offsets, strides);
}

void VulkanDriver::command_buffer_bind_index_buffer(Graphics::CommandBufferID command_buffer, Graphics::BufferID index_buffer, u32 offset, Graphics::IndexType index_type)
{
    Unused(command_buffer, index_buffer, offset, index_type);
}

void VulkanDriver::command_buffer_bind_pipeline(Graphics::CommandBufferID command_buffer, Graphics::PipelineID pipeline)
{
    Unused(command_buffer, pipeline);
}

void VulkanDriver::command_buffer_bind_render_target(Graphics::CommandBufferID command_buffer, Graphics::RenderTargetID render_target)
{
    Unused(command_buffer, render_target);
}

void VulkanDriver::command_buffer_set_texture_unit(Graphics::CommandBufferID command_buffer, u32 set, u32 base_slot, const Slice<Graphics::TextureID>& textures)
{
    Unused(command_buffer, set, base_slot, textures);
}

void VulkanDriver::command_buffer_set_uniform(Graphics::CommandBufferID command_buffer, u32 set, u32 base_slot, const Slice<Graphics::BufferID>& buffers)
{
    Unused(command_buffer, set, base_slot, buffers);
}

void VulkanDriver::command_buffer_set_viewport(Graphics::CommandBufferID command_buffer, Rect2DI viewport_rect)
{
    Unused(command_buffer, viewport_rect);
}

void VulkanDriver::command_buffer_clear(Graphics::CommandBufferID command_buffer, Graphics::RenderTargetID render_target, Color clear_color)
{
    Unused(command_buffer, render_target, clear_color);
}

void VulkanDriver::command_buffer_draw(Graphics::CommandBufferID command_buffer, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance)
{
    Unused(command_buffer, vertex_count, instance_count, base_vertex, base_instance);
}

void VulkanDriver::command_buffer_draw_indexed(Graphics::CommandBufferID command_buffer, u32 index_count, u32 instance_count, u32 base_index, u32 base_vertex, u32 base_instance)
{
    Unused(command_buffer, index_count, instance_count, base_index, base_vertex, base_instance);
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

void VulkanDriver::_surface_format_to_vk_swapchain_info(Graphics::SurfaceFormat surface_format, VkFormat* vk_image_format, VkColorSpaceKHR* vk_color_space)
{
    switch(surface_format)
    {
    case Graphics::SurfaceFormat::RGBA8Unorm:
        *vk_image_format = VK_FORMAT_R8G8B8A8_UNORM;
        *vk_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        return;
    case Graphics::SurfaceFormat::RGBA8Srgb:
        *vk_image_format = VK_FORMAT_R8G8B8A8_SRGB;
        *vk_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        return;
    case Graphics::SurfaceFormat::BGRA8Unorm:
        *vk_image_format = VK_FORMAT_B8G8R8A8_UNORM;
        *vk_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        return;
    case Graphics::SurfaceFormat::BGRA8Srgb:
        *vk_image_format = VK_FORMAT_B8G8R8A8_SRGB;
        *vk_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
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
    case Graphics::PresentMode::Immediate:
        return VK_PRESENT_MODE_IMMEDIATE_KHR;
        case Graphics::PresentMode::VSync:
        return VK_PRESENT_MODE_FIFO_KHR;
    default:
        break;
    }

    VKFailOn(true, "invalid present mode");
    return VK_PRESENT_MODE_IMMEDIATE_KHR;
}

VkSurfaceCapabilitiesKHR VulkanDriver::_surface_get_capabilities(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface)
{
    VkSurfaceCapabilitiesKHR capabilities;
    vk.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, vk_surface, &capabilities);
    return capabilities;   
}

VkExtent2D VulkanDriver::_swap_chain_get_vk_extent(const Vector2I& size, const VkSurfaceCapabilitiesKHR& vk_capabilities)
{
    if(vk_capabilities.currentExtent.width != MaxValue<uint32_t>)
    {
        return vk_capabilities.currentExtent;
    }
    
    VkExtent2D vk_extent = {};
    vk_extent.width = math::clamp(static_cast<uint32_t>(size.width), vk_capabilities.minImageExtent.width, vk_capabilities.maxImageExtent.width);
    vk_extent.height = math::clamp(static_cast<uint32_t>(size.height), vk_capabilities.minImageExtent.height, vk_capabilities.maxImageExtent.height);
    return vk_extent;
}

VkPipelineStageFlags VulkanDriver::_vk_get_pipeline_stages(Graphics::PipelineStages stages)
{
    VkPipelineStageFlags vk_flags = 0;

    if(HasValue(stages & Graphics::PipelineStages::Begin))
    {
        vk_flags |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    if(HasValue(stages & Graphics::PipelineStages::VertexInput))
    {
        vk_flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    }
    if (HasValue(stages & Graphics::PipelineStages::VertexShader))
    {
        vk_flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }
    if (HasValue(stages & Graphics::PipelineStages::FragmentShader))
    {
        vk_flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    if(HasValue(stages & Graphics::PipelineStages::RenderOutput))
    {
        vk_flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    if(HasValue(stages & Graphics::PipelineStages::End))
    {
        vk_flags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }

    return vk_flags;
}

VkImageAspectFlags VulkanDriver::_vk_get_aspect_masks(Graphics::TextureAspects aspects)
{
    VkImageAspectFlags vk_aspects = 0;

    if (HasValue(aspects & Graphics::TextureAspects::Color))
    {
        vk_aspects |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (HasValue(aspects & Graphics::TextureAspects::Depth))
    {
        vk_aspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (HasValue(aspects & Graphics::TextureAspects::Stencil))
    {
        vk_aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    return vk_aspects;
}

VkAccessFlags VulkanDriver::_vk_get_access_masks(Graphics::AccessMasks access_masks)
{
    VkAccessFlags vk_access_masks = 0;

    if (HasValue(access_masks & Graphics::AccessMasks::RenderOutputRead))
    {
        vk_access_masks |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    }
    if (HasValue(access_masks & Graphics::AccessMasks::RenderOutputWrite))
    {
        vk_access_masks |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    return vk_access_masks;
}

VkImageLayout VulkanDriver::_vk_get_image_layout(Graphics::TextureLayout texture_layout)
{
    switch (texture_layout)
    {
    case Graphics::TextureLayout::Unknown:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    case Graphics::TextureLayout::RenderOutput:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case Graphics::TextureLayout::Present:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    default:
        break;
    }

    VKFailOn(true, "invalid texture layout");
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

Graphics::DeviceType VulkanDriver::_vk_device_type_to_device_type(VkPhysicalDeviceType vk_dt)
{
    if(vk_dt == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
    {
        return Graphics::DeviceType::IntegratedGPU;
    }
    else if(vk_dt == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        return Graphics::DeviceType::DiscreteGPU;
    }

    VKFailOn(true, "invalid vulkan physical device type");
    return Graphics::DeviceType::Unknown;
}

Graphics::SurfaceFormat VulkanDriver::_vk_surface_format_to_surface_format(VkSurfaceFormatKHR vk_sf)
{
    if(vk_sf.format == VK_FORMAT_R8G8B8A8_UNORM)
    {
        return Graphics::SurfaceFormat::RGBA8Unorm;
    }
    else if(vk_sf.format == VK_FORMAT_R8G8B8A8_SRGB)
    {
        return Graphics::SurfaceFormat::RGBA8Srgb;
    }
    else if(vk_sf.format == VK_FORMAT_B8G8R8A8_UNORM)
    {
        return Graphics::SurfaceFormat::BGRA8Unorm;
    }
    else if(vk_sf.format == VK_FORMAT_B8G8R8A8_SRGB)
    {
        return Graphics::SurfaceFormat::BGRA8Srgb;
    }

    VKFailOn(true, "invalid vulkan surface format");
    return Graphics::SurfaceFormat::Unknown;
}

Graphics::PresentMode VulkanDriver::_vk_present_mode_to_present_mode(VkPresentModeKHR vk_pm)
{
    if(vk_pm == VK_PRESENT_MODE_IMMEDIATE_KHR)
    {
        return Graphics::PresentMode::Immediate;
    }
    else if(vk_pm == VK_PRESENT_MODE_FIFO_KHR)
    {
        return Graphics::PresentMode::VSync;
    }

    VKFailOn(true, "invalid vulkan present mode");
    return Graphics::PresentMode::Unknown;
}

