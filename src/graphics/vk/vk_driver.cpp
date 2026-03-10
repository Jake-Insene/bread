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
        .memory_heap_create = &VulkanDriver::memory_heap_create,
        .memory_heap_destroy = &VulkanDriver::memory_heap_destroy,
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
        .descriptor_set_create = &VulkanDriver::descriptor_set_create,
        .descriptor_set_destroy = &VulkanDriver::descriptor_set_destroy,
        .descriptor_set_update_descriptors = &VulkanDriver::descriptor_set_update_descriptors,
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
        .command_buffer_copy_buffer = &VulkanDriver::command_buffer_copy_buffer,
        .command_buffer_bind_pipeline = &VulkanDriver::command_buffer_bind_pipeline,
        .command_buffer_bind_descriptor_sets = &VulkanDriver::command_buffer_bind_descriptor_sets,
        .command_buffer_bind_vertex_buffers = &VulkanDriver::command_buffer_bind_vertex_buffers,
        .command_buffer_constant_block = &VulkanDriver::command_buffer_constant_block,
        .command_buffer_set_viewports = &VulkanDriver::command_buffer_set_viewports,
        .command_buffer_set_scissors = &VulkanDriver::command_buffer_set_scissors,
        .command_buffer_draw = &VulkanDriver::command_buffer_draw,
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
    data.memory_heaps = FreeList<MemoryHeap, Graphics::MemoryHeapID>::with_allocator(allocator);
    data.buffers = FreeList<Buffer, Graphics::BufferID>::with_allocator(allocator);
    data.textures = FreeList<Texture, Graphics::TextureID>::with_allocator(allocator);
    data.render_targets = FreeList<RenderTarget, Graphics::RenderTargetID>::with_allocator(allocator);
    data.descriptor_sets = FreeList<DescriptorSet, Graphics::DescriptorSetID>::with_allocator(allocator);
    data.pipelines = FreeList<Pipeline, Graphics::PipelineID>::with_allocator(allocator);
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
    data.memory_heaps.destroy();
    data.buffers.destroy();
    data.textures.destroy();
    data.render_targets.destroy();
    data.descriptor_sets.destroy();
    data.pipelines.destroy();
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

    // Checking for required extensions for the driver.
    Vulkan::check_device_extensions(pd.vk_physical_device);

    u32 family_count;
    vk.vkGetPhysicalDeviceQueueFamilyProperties2(pd.vk_physical_device, &family_count, nullptr);

    Slice<VkQueueFamilyProperties2> families = get_allocator().array<VkQueueFamilyProperties2>(family_count);
    for(VkQueueFamilyProperties2& family : families)
    {
        family.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
        family.pNext = nullptr;
    }
    vk.vkGetPhysicalDeviceQueueFamilyProperties2(pd.vk_physical_device, &family_count, families.ptr());    

    // 0->graphics, 1->present
    uint32_t queue_families[] = {MaxValue<uint32_t>, MaxValue<uint32_t>};
    for(usize i = 0; i < families.len; i++)
    {
        VkQueueFamilyProperties2 family = families[i];
        if(family.queueFamilyProperties.queueCount >= 1 && family.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
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

    // Device info
    VkPhysicalDeviceProperties2 vk_physical_properties =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = nullptr,
        .properties = {},
    };
    vk.vkGetPhysicalDeviceProperties2(ld.vk_physical_device, &vk_physical_properties);
    ld.vk_physical_device_properties = vk_physical_properties.properties;

    VkPhysicalDeviceFeatures2 vk_physical_features =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = nullptr,
        .features = {},
    };
    vk.vkGetPhysicalDeviceFeatures2(ld.vk_physical_device, &vk_physical_features);
    ld.vk_physical_device_features = vk_physical_features.features;

    VkPhysicalDeviceMemoryProperties2 vk_physical_memory_properties =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
        .pNext = nullptr,
        .memoryProperties = {},
    };

    vk.vkGetPhysicalDeviceMemoryProperties2(ld.vk_physical_device, &vk_physical_memory_properties);
    ld.vk_physical_device_memory_properties = vk_physical_memory_properties.memoryProperties;

    // Queues
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

    // Global pool
    VkDescriptorPoolSize vk_pool_sizes[] =
    {
        { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1000, },
        { .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1000, },
    };

    VkDescriptorPoolCreateInfo vk_global_pool_info = 
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000,
        .poolSizeCount = static_cast<uint32_t>(ArraySize(vk_pool_sizes)),
        .pPoolSizes = vk_pool_sizes,
    };

    result = ld.vk.vkCreateDescriptorPool(ld.vk_device, &vk_global_pool_info, Vulkan::allocation_callbacks(), &ld.vk_global_descriptor_pool);
    VKFailOn(result != VK_SUCCESS, "vkCreateDescriptorPool({})", Vulkan::result_as_string(result));
    
    return device_id;
}

void VulkanDriver::device_destroy(Graphics::DeviceID device)
{
    VKFailOn(device.is_valid() == false, "invalid device");
    LogicalDevice& ld = _get_logical_device(device);

    ld.vk.vkDestroyDescriptorPool(ld.vk_device, ld.vk_global_descriptor_pool, Vulkan::allocation_callbacks());
    
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
    _vk_get_surface_format(ci.format, &vk_swapchain_format, &vk_swap_chain_color_space);
    VkPresentModeKHR vk_present_mode = _vk_get_present_mode(ci.present_mode);

    VkSurfaceCapabilitiesKHR capabilities = _vk_get_surface_capabilities(ld.vk_physical_device, surface.vk_surface);
    VkExtent2D vk_swap_chain_extent = _vk_get_swap_chain_extent(ci.size, capabilities);

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

    VkResult result = ld.vk.vkAcquireNextImageKHR(sc.vk_device, sc.vk_swapchain, acquire_info.timeout, vk_semaphore, vk_fence, image_index);
    VKFailOn(result != VK_SUCCESS, "vkAcquireNextImageKHR({})", Vulkan::result_as_string(result));
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

Graphics::MemoryHeapID VulkanDriver::memory_heap_create(const Graphics::MemoryHeapCreateInfo& ci)
{
    VKFailOn(ci.device.is_valid() == false, "invalid device");
    VKFailOn(ci.heap_usage == Graphics::HeapUsage::CPUExclusive, "invalid heap usage");
    VKFailOn(ci.heap_size == 0, "invalid heap size");
    VKFailOn(ci.heap_size < Graphics::MinHeapSize, "invalid heap size");

    LogicalDevice& ld = _get_logical_device(ci.device);
    Graphics::MemoryHeapID memory_heap_id = data.memory_heaps.add(MemoryHeap());
    MemoryHeap& heap = _get_memory_heap(memory_heap_id);
    heap.vk_device = ld.vk_device;
    heap.device = ci.device;
    heap.memory_heap = memory_heap_id;

    uint32_t vk_type_index = MaxValue<uint32_t>;
    VkMemoryPropertyFlags vk_memory_flags = _vk_get_memory_properties(ci.heap_usage);

    for(uint32_t i = 0; i < ld.vk_physical_device_memory_properties.memoryTypeCount; i++)
    {
        VkMemoryType mem_type = ld.vk_physical_device_memory_properties.memoryTypes[i];
        VkMemoryPropertyFlags masked = (mem_type.propertyFlags & vk_memory_flags);
        if(masked == mem_type.propertyFlags)
        {
            vk_type_index = i;
        }   
    }

    VkMemoryAllocateInfo vk_allocate_info =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = ci.heap_size,
        .memoryTypeIndex = vk_type_index,
    };

    VkResult result = ld.vk.vkAllocateMemory(ld.vk_device, &vk_allocate_info, Vulkan::allocation_callbacks(), &heap.vk_memory);
    VKFailOn(result != VK_SUCCESS, "vkAllocateMemory({})", Vulkan::result_as_string(result));

    return memory_heap_id;
}

void VulkanDriver::memory_heap_destroy(Graphics::MemoryHeapID memory_heap)
{
    VKFailOn(memory_heap.is_valid() == false, "invalid memory heap");

    MemoryHeap& heap = _get_memory_heap(memory_heap);
    LogicalDevice& ld = _get_logical_device(heap.device);

    ld.vk.vkFreeMemory(heap.vk_device, heap.vk_memory, Vulkan::allocation_callbacks());

    data.memory_heaps.remove(memory_heap);
}

Graphics::BufferID VulkanDriver::buffer_create(const Graphics::BufferCreateInfo& ci)
{
    VKFailOn(ci.device.is_valid() == false, "invalid device");
    VKFailOn(ci.usage == Graphics::BufferUsage(0), "invalid buffer usage");
    VKFailOn(ci.size < Graphics::MinHeapResourceAlignment, "invalid buffer size");
    VKFailOn(ci.memory_heap.is_valid() == false, "invalid memory heap");

    LogicalDevice& ld = _get_logical_device(ci.device);
    Graphics::BufferID buffer_id = data.buffers.add(Buffer());
    Buffer& buffer = _get_buffer(buffer_id);
    MemoryHeap& heap = _get_memory_heap(ci.memory_heap);

    buffer.vk_device = ld.vk_device;
    buffer.device = ci.device;
    buffer.buffer = buffer_id;
    buffer.memory_heap = ci.memory_heap;

    VkBufferCreateInfo vk_buffer_info =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = ci.size,
        .usage = _vk_get_buffer_usage(ci.usage),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };

    VkResult result = ld.vk.vkCreateBuffer(ld.vk_device, &vk_buffer_info, Vulkan::allocation_callbacks(), &buffer.vk_buffer);
    VKFailOn(result != VK_SUCCESS, "vkCreateBuffer({})", Vulkan::result_as_string(result));

    VkBindBufferMemoryInfo vk_bind_info =
    {
        .sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO,
        .pNext = nullptr,
        .buffer = buffer.vk_buffer,
        .memory = heap.vk_memory,
        .memoryOffset = ci.heap_offset,
    };

    result = ld.vk.vkBindBufferMemory2(ld.vk_device, 1, &vk_bind_info);
    VKFailOn(result != VK_SUCCESS, "vkBindBufferMemory2({})", Vulkan::result_as_string(result));

    return buffer_id;
}

void VulkanDriver::buffer_destroy(Graphics::BufferID buffer)
{
    VKFailOn(buffer.is_valid() == false, "invalid buffer");

    Buffer& b = _get_buffer(buffer);
    LogicalDevice& ld = _get_logical_device(b.device);

    ld.vk.vkDestroyBuffer(b.vk_device, b.vk_buffer, Vulkan::allocation_callbacks());

    data.buffers.remove(buffer);
}

Slice<u8> VulkanDriver::buffer_map_memory(Graphics::BufferID buffer, usize offset, usize len)
{
    VKFailOn(buffer.is_valid() == false, "invalid buffer");
    VKFailOn(len == 0, "invalid buffer len");

    Buffer& b = _get_buffer(buffer);
    MemoryHeap& heap = _get_memory_heap(b.memory_heap);
    LogicalDevice& ld = _get_logical_device(b.device);

    void* ptr = nullptr;
    VkMemoryMapInfo vk_map_info =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_MAP_INFO,
        .pNext = nullptr,
        .flags = 0,
        .memory = heap.vk_memory,
        .offset = offset,
        .size = len,
    };

    VkResult result = ld.vk.vkMapMemory2(ld.vk_device, &vk_map_info, &ptr);
    VKFailOn(result != VK_SUCCESS, "vkMapMemory2({})", Vulkan::result_as_string(result));

    return Slice<u8>(reinterpret_cast<u8*>(ptr), len);
}

void VulkanDriver::buffer_unmap_memory(Graphics::BufferID buffer, const Slice<u8>& memory)
{
    VKFailOn(buffer.is_valid() == false, "invalid buffer");
    VKFailOn(memory.ptr() == nullptr, "invalid memory address");

    Buffer& b = _get_buffer(buffer);
    MemoryHeap& heap = _get_memory_heap(b.memory_heap);
    LogicalDevice& ld = _get_logical_device(b.device);

    VkMemoryUnmapInfo vk_unmap_info =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_UNMAP_INFO,
        .pNext = nullptr,
        .flags = 0,
        .memory = heap.vk_memory,
    };

    VkResult result = ld.vk.vkUnmapMemory2(ld.vk_device, &vk_unmap_info);
    VKFailOn(result != VK_SUCCESS, "vkUnmapMemory2({})", Vulkan::result_as_string(result));
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

Graphics::DescriptorSetID VulkanDriver::descriptor_set_create(const Graphics::DescriptorSetCreateInfo& ci)
{
    VKFailOn(ci.device.is_valid() == false, "invalid device");
    VKFailOn(ci.bindings.len == 0, "invalid binding count");

    Graphics::DescriptorSetID descriptor_set_id = data.descriptor_sets.add(DescriptorSet());
    DescriptorSet& set = _get_descriptor_set(descriptor_set_id);
    LogicalDevice& ld = _get_logical_device(ci.device);

    set.vk_device = ld.vk_device;
    set.device = ci.device;
    set.descriptor_set = descriptor_set_id;

    Slice<VkDescriptorSetLayoutBinding> vk_bindings = get_allocator().array<VkDescriptorSetLayoutBinding>(ci.bindings.len);
    for(usize i = 0; i < ci.bindings.len; i++)
    {
        vk_bindings[i] =
        {
            .binding = ci.bindings[i].binding,
            .descriptorType = _vk_get_descriptor_type(ci.bindings[i].type),
            .descriptorCount = ci.bindings[i].count,
            .stageFlags = _vk_get_shader_stage(ci.bindings[i].stages),
            .pImmutableSamplers = nullptr,
        };
    }

    VkDescriptorSetLayoutCreateInfo vk_set_layout_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = static_cast<uint32_t>(vk_bindings.len),
        .pBindings = vk_bindings.ptr(),
    };

    VkResult result = ld.vk.vkCreateDescriptorSetLayout(ld.vk_device, &vk_set_layout_info, Vulkan::allocation_callbacks(), &set.vk_set_layout);
    VKFailOn(result != VK_SUCCESS, "vkCreateDescriptorSetLayout({})", Vulkan::result_as_string(result));

    VkDescriptorSetAllocateInfo vk_allocate_set_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = ld.vk_global_descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &set.vk_set_layout,
    };

    result = ld.vk.vkAllocateDescriptorSets(ld.vk_device, &vk_allocate_set_info, &set.vk_descriptor_set);
    VKFailOn(result != VK_SUCCESS, "vkAllocateDescriptorSets({})", Vulkan::result_as_string(result));

    get_allocator().free(mem::to_bytes(vk_bindings));
    return descriptor_set_id;
}

void VulkanDriver::descriptor_set_destroy(Graphics::DescriptorSetID descriptor_set)
{
    VKFailOn(descriptor_set.is_valid() == false, "invalid device");

    DescriptorSet& set = _get_descriptor_set(descriptor_set);
    LogicalDevice& ld = _get_logical_device(set.device);

    ld.vk.vkDestroyDescriptorSetLayout(set.vk_device, set.vk_set_layout, Vulkan::allocation_callbacks());
    VkResult result = ld.vk.vkFreeDescriptorSets(set.vk_device, ld.vk_global_descriptor_pool, 1, &set.vk_descriptor_set);
    VKFailOn(result != VK_SUCCESS, "vkFreeDescriptorSets({})", Vulkan::result_as_string(result));

    data.descriptor_sets.remove(descriptor_set);
}

void VulkanDriver::descriptor_set_update_descriptors(Graphics::DescriptorSetID descriptor_set, const Graphics::UpdateDescriptorInfo& update_info)
{
    VKFailOn(descriptor_set.is_valid() == false, "invalid device");

    DescriptorSet& set = _get_descriptor_set(descriptor_set);
    LogicalDevice& ld = _get_logical_device(set.device);

    Slice<VkWriteDescriptorSet> vk_write_descriptor = get_allocator().array<VkWriteDescriptorSet>(update_info.write_infos.len);

    usize total_buffer_infos = 0;
    for(usize i = 0; i < update_info.write_infos.len; i++)
    {
        if(update_info.write_infos[i].type == Graphics::DescriptorType::UniformBuffer
            || update_info.write_infos[i].type == Graphics::DescriptorType::StorageBuffer)
        {
            total_buffer_infos += update_info.write_infos[i].count;
        }
    }

    Slice<VkDescriptorBufferInfo> vk_buffer_infos = get_allocator().array<VkDescriptorBufferInfo>(total_buffer_infos);
    usize vk_buffer_infos_index = 0;

    for(usize i = 0; i < update_info.write_infos.len; i++)
    {
        const Graphics::WriteDescriptorInfo& write_info = update_info.write_infos[i];
        vk_write_descriptor[i] =
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = set.vk_descriptor_set,
            .dstBinding = write_info.binding,
            .dstArrayElement = write_info.array_element,
            .descriptorCount = write_info.count,
            .descriptorType = _vk_get_descriptor_type(write_info.type),
            .pImageInfo = nullptr,
            .pBufferInfo = &vk_buffer_infos[vk_buffer_infos_index],
            .pTexelBufferView = nullptr,
        };

        switch(update_info.write_infos[i].type)
        {
        case Graphics::DescriptorType::UniformBuffer:
        case Graphics::DescriptorType::StorageBuffer:
        {
            for(usize buffer_i = 0; i < write_info.buffers.len; i++)
            {
                const Graphics::DescriptorBufferInfo& buffer_info = write_info.buffers[buffer_i];

                vk_buffer_infos[vk_buffer_infos_index] =
                {
                    .buffer = _get_buffer(buffer_info.buffer).vk_buffer,
                    .offset = buffer_info.offset,
                    .range = buffer_info.range,
                };
                vk_buffer_infos_index++;
            }
        }
            break;
        default:
            VKFailOn(true, "invalid descriptor type");
            break;
        }   
    }

    ld.vk.vkUpdateDescriptorSets(
        set.vk_device, static_cast<uint32_t>(vk_write_descriptor.len), vk_write_descriptor.ptr(),
        0, nullptr
    );

    get_allocator().free(mem::to_bytes(vk_buffer_infos));
    get_allocator().free(mem::to_bytes(vk_write_descriptor));
}

Graphics::PipelineID VulkanDriver::pipeline_create(const Graphics::PipelineCreateInfo& ci)
{
    VKFailOn(ci.device.is_valid() == false, "invalid device");
    VKFailOn(ci.bind_point == Graphics::PipelineBindPoint::Unknown, "invalid pipeline bind point");
    VKFailOn(ci.input_assembly.topology == Graphics::PrimitiveTopology::Unknown, "invalid topology");
    VKFailOn(ci.shader_stages.len == 0, "at least one shader stage was expected");

    LogicalDevice& ld = _get_logical_device(ci.device);
    Graphics::PipelineID pipeline_id = data.pipelines.add(Pipeline());
    Pipeline& pipe = _get_pipeline(pipeline_id);
    pipe.vk_device = ld.vk_device;
    pipe.device = ci.device;
    pipe.pipeline = pipeline_id;

    Slice<VkPipelineShaderStageCreateInfo> vk_shader_stages = get_allocator().array<VkPipelineShaderStageCreateInfo>(ci.shader_stages.len);

    for(usize i = 0; i < ci.shader_stages.len; i++)
    {
        vk_shader_stages[i] =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VkShaderStageFlagBits(_vk_get_shader_stage(ci.shader_stages[i].stage)),
            .module = _vk_create_shader_module(ld, ci.shader_stages[i]),
            .pName = "main",
            .pSpecializationInfo = nullptr,
        };
    }

    Slice<VkVertexInputBindingDescription> vk_vertex_bindings = get_allocator().array<VkVertexInputBindingDescription>(ci.vertex_input.bindings.len);
    Slice<VkVertexInputAttributeDescription> vk_vertex_attributes = get_allocator().array<VkVertexInputAttributeDescription>(ci.vertex_input.attributes.len);
    
    for(usize i = 0; i < ci.vertex_input.bindings.len; i++)
    {
        vk_vertex_bindings[i] =
        {
            .binding = ci.vertex_input.bindings[i].binding,
            .stride = ci.vertex_input.bindings[i].stride,
            .inputRate = _vk_get_input_rate(ci.vertex_input.bindings[i].input_rate),
        };
    }

    for(usize i = 0; i < ci.vertex_input.attributes.len; i++)
    {
        vk_vertex_attributes[i] =
        {
            .location = ci.vertex_input.attributes[i].location,
            .binding = ci.vertex_input.attributes[i].binding,
            .format = _vk_get_vertex_format(ci.vertex_input.attributes[i].format),
            .offset = ci.vertex_input.attributes[i].offset,
        };
    }

    VkPipelineVertexInputStateCreateInfo vk_vertex_input_state =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = static_cast<uint32_t>(vk_vertex_bindings.len),
        .pVertexBindingDescriptions = vk_vertex_bindings.ptr(),
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vk_vertex_attributes.len),
        .pVertexAttributeDescriptions = vk_vertex_attributes.ptr(),
    };

    VkPipelineInputAssemblyStateCreateInfo vk_input_assembly_state =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = _vk_get_topology(ci.input_assembly.topology),
        .primitiveRestartEnable = VK_FALSE,
    };

    VkPipelineViewportStateCreateInfo vk_viewport_state =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr,
    };

    VkPipelineRasterizationStateCreateInfo vk_rasterization_state =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = ci.rasterizer_state.depth_clamp_enable ? VK_TRUE : VK_FALSE,
        .rasterizerDiscardEnable = ci.rasterizer_state.rasterizer_discard_enable ? VK_TRUE : VK_FALSE,
        .polygonMode = _vk_get_polygon_mode(ci.rasterizer_state.polygon_mode),
        .cullMode = _vk_get_cull_mode(ci.rasterizer_state.cull_mode),
        .frontFace = _vk_get_front_face(ci.rasterizer_state.front_face),
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0,
        .depthBiasClamp = 0,
        .depthBiasSlopeFactor = 1.f,
        .lineWidth = ci.rasterizer_state.line_width,
    };

    VkPipelineMultisampleStateCreateInfo vk_multisample_state =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = _vk_get_samples(ci.multisample_state.sample_count),
        .sampleShadingEnable = ci.multisample_state.sample_shading_enable ? VK_TRUE : VK_FALSE,
        .minSampleShading = ci.multisample_state.min_sample_shading,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = ci.multisample_state.alpha_to_coverage_enable ? VK_TRUE : VK_FALSE,
        .alphaToOneEnable = ci.multisample_state.alpha_one_enable ? VK_TRUE : VK_FALSE,
    };

    VkPipelineDepthStencilStateCreateInfo vk_depth_stencil_state =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthTestEnable = ci.depth_stencil_state.depth_test_enable ? VK_TRUE : VK_FALSE,
        .depthWriteEnable = ci.depth_stencil_state.depth_write_enable ? VK_TRUE : VK_FALSE,
        .depthCompareOp = ci.depth_stencil_state.depth_test_enable ? VK_COMPARE_OP_LESS : VK_COMPARE_OP_ALWAYS,
        .depthBoundsTestEnable = ci.depth_stencil_state.depth_bounds_test_enable ? VK_TRUE : VK_FALSE,
        .stencilTestEnable = ci.depth_stencil_state.stencil_test_enable ? VK_TRUE : VK_FALSE,
        .front = {},
        .back = {},
        .minDepthBounds = ci.depth_stencil_state.min_depth_bounds,
        .maxDepthBounds = ci.depth_stencil_state.max_depth_bounds,
    };

    VkPipelineColorBlendAttachmentState vk_color_blend_attachment =
    {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT
                        | VK_COLOR_COMPONENT_G_BIT
                        | VK_COLOR_COMPONENT_B_BIT
                        | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo vk_color_blend_state =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &vk_color_blend_attachment,
        .blendConstants = {},
    };

    VkDynamicState vk_dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo vk_dynamic_state =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = static_cast<uint32_t>(ArraySize(vk_dynamic_states)),
        .pDynamicStates = vk_dynamic_states,
    };

    VkFormat vk_attachment_format;
    VkColorSpaceKHR vk_color_space;
    _vk_get_surface_format(ci.surface_format, &vk_attachment_format, &vk_color_space);

    VkPipelineRenderingCreateInfo vk_rendering_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = nullptr,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &vk_attachment_format,
        .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
    };

    // Creating the layout

    Slice<VkPushConstantRange> vk_push_ranges = get_allocator().array<VkPushConstantRange>(ci.pipeline_layout.constant_blocks.len);
    for(usize i = 0; i < ci.pipeline_layout.constant_blocks.len; i++)
    {
        vk_push_ranges[i] =
        {
            .stageFlags = VkShaderStageFlags(_vk_get_shader_stage(ci.pipeline_layout.constant_blocks[i].stages)),
            .offset = ci.pipeline_layout.constant_blocks[i].offset,
            .size = ci.pipeline_layout.constant_blocks[i].size,
        };
    }

    pipe.vk_set_layouts = get_allocator().array<VkDescriptorSetLayout>(ci.pipeline_layout.sets.len);
    for(usize i = 0; i < ci.pipeline_layout.sets.len; i++)
    {
        pipe.vk_set_layouts[i] = _vk_create_set_layout(ld, ci.pipeline_layout.sets[i]);
    }

    VkPipelineLayoutCreateInfo vk_pipeline_layout_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = static_cast<uint32_t>(pipe.vk_set_layouts.len),
        .pSetLayouts = pipe.vk_set_layouts.ptr(),
        .pushConstantRangeCount = static_cast<uint32_t>(vk_push_ranges.len),
        .pPushConstantRanges = vk_push_ranges.ptr(),
    };

    VkResult result = ld.vk.vkCreatePipelineLayout(ld.vk_device, &vk_pipeline_layout_info, Vulkan::allocation_callbacks(), &pipe.vk_pipeline_layout);
    VKFailOn(result != VK_SUCCESS, "vkCreatePipelineLayout({})", Vulkan::result_as_string(result));
    
    VkGraphicsPipelineCreateInfo vk_graphics_pipeline_info =
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &vk_rendering_info,
        .flags = 0,
        .stageCount = static_cast<uint32_t>(vk_shader_stages.len),
        .pStages = vk_shader_stages.ptr(),
        .pVertexInputState = &vk_vertex_input_state,
        .pInputAssemblyState = &vk_input_assembly_state,
        .pTessellationState = nullptr,
        .pViewportState = &vk_viewport_state,
        .pRasterizationState = &vk_rasterization_state,
        .pMultisampleState = &vk_multisample_state,
        .pDepthStencilState = &vk_depth_stencil_state, // TODO: implement depth stencil
        .pColorBlendState = &vk_color_blend_state,
        .pDynamicState = &vk_dynamic_state,
        .layout = pipe.vk_pipeline_layout,
        .renderPass = VK_NULL_HANDLE,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
    };

    result = ld.vk.vkCreateGraphicsPipelines(
        ld.vk_device, VK_NULL_HANDLE, 1, &vk_graphics_pipeline_info,
        Vulkan::allocation_callbacks(), &pipe.vk_pipeline
    );
    VKFailOn(result != VK_SUCCESS, "vkCreateGraphicsPipelines({})", Vulkan::result_as_string(result));

    // destroy shader modules
    for(usize i = 0; i < ci.shader_stages.len; i++)
    {
        ld.vk.vkDestroyShaderModule(ld.vk_device, vk_shader_stages[i].module, Vulkan::allocation_callbacks());
    }

    // deallocating vk structs
    get_allocator().free(mem::to_bytes(vk_shader_stages));
    get_allocator().free(mem::to_bytes(vk_vertex_bindings));
    get_allocator().free(mem::to_bytes(vk_vertex_attributes));
    get_allocator().free(mem::to_bytes(vk_push_ranges));   
    return pipeline_id;
}

void VulkanDriver::pipeline_destroy(Graphics::PipelineID pipeline)
{
    VKFailOn(pipeline.is_valid() == false, "invalid pipeline");

    Pipeline& pipe = _get_pipeline(pipeline);
    LogicalDevice& ld = _get_logical_device(pipe.device);

    ld.vk.vkDestroyPipeline(pipe.vk_device, pipe.vk_pipeline, Vulkan::allocation_callbacks());
    ld.vk.vkDestroyPipelineLayout(pipe.vk_device, pipe.vk_pipeline_layout, Vulkan::allocation_callbacks());

    for(usize i = 0; i < pipe.vk_set_layouts.len; i++)
    {
        ld.vk.vkDestroyDescriptorSetLayout(pipe.vk_device, pipe.vk_set_layouts[i], Vulkan::allocation_callbacks());
    }
    get_allocator().free(mem::to_bytes(pipe.vk_set_layouts));   

    data.pipelines.remove(pipeline);
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

void VulkanDriver::command_buffer_copy_buffer(Graphics::CommandBufferID command_buffer, const Graphics::BufferCopyInfo& copy_info)
{
    VKFailOn(command_buffer.is_valid() == false, "invalid command buffer");
    VKFailOn(copy_info.source_buffer.is_valid() == false, "invalid source buffer");
    VKFailOn(copy_info.destination_buffer.is_valid() == false, "invalid destination buffer");

    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    Buffer& src_buffer = _get_buffer(copy_info.source_buffer);
    Buffer& dest_buffer = _get_buffer(copy_info.destination_buffer);

    Slice<VkBufferCopy> vk_regions = get_allocator().array<VkBufferCopy>(copy_info.copy_regions.len);
    for(usize i = 0; i < copy_info.copy_regions.len; i++)
    {
        vk_regions[i] =
        {
            .srcOffset = copy_info.copy_regions[i].source_offset,
            .dstOffset = copy_info.copy_regions[i].destination_offset,
            .size = copy_info.copy_regions[i].size,
        };
    }

    ld.vk.vkCmdCopyBuffer(
        cmd_buffer.vk_command_buffer, src_buffer.vk_buffer, dest_buffer.vk_buffer,
        static_cast<uint32_t>(vk_regions.len), vk_regions.ptr()

    );

    get_allocator().free(mem::to_bytes(vk_regions));
}

void VulkanDriver::command_buffer_bind_pipeline(Graphics::CommandBufferID command_buffer, Graphics::PipelineBindPoint bind_point, Graphics::PipelineID pipeline)
{
    VKFailOn(command_buffer.is_valid() == false, "invalid command buffer");
    VKFailOn(bind_point == Graphics::PipelineBindPoint::Unknown, "invalid bind point");
    VKFailOn(pipeline.is_valid() == false, "invalid pipeline");

    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    cmd_buffer.last_binded_pipeline = pipeline;

    Pipeline& pipe = _get_pipeline(pipeline);

    ld.vk.vkCmdBindPipeline(cmd_buffer.vk_command_buffer, _vk_get_bind_point(bind_point), pipe.vk_pipeline);
}

void VulkanDriver::command_buffer_bind_descriptor_sets(Graphics::CommandBufferID command_buffer, Graphics::PipelineBindPoint bind_point, u32 base_set, const Slice<Graphics::DescriptorSetID>& descriptor_sets)
{
    VKFailOn(command_buffer.is_valid() == false, "invalid command buffer");
    VKFailOn(bind_point == Graphics::PipelineBindPoint::Unknown, "invalid bind point");
    VKFailOn(descriptor_sets.len == 0, "invalid descriptor set count");

    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    Pipeline& current_pipe = _get_pipeline(cmd_buffer.last_binded_pipeline);

    Slice<VkDescriptorSet> vk_descriptor_sets = get_allocator().array<VkDescriptorSet>(descriptor_sets.len);
    for(usize i = 0; i < descriptor_sets.len; i++)
    {
        vk_descriptor_sets[i] = _get_descriptor_set(descriptor_sets[i]).vk_descriptor_set;
    }

    ld.vk.vkCmdBindDescriptorSets(
        cmd_buffer.vk_command_buffer, _vk_get_bind_point(bind_point), current_pipe.vk_pipeline_layout,
        base_set, static_cast<uint32_t>(vk_descriptor_sets.len), vk_descriptor_sets.ptr(), 0, nullptr 
    );

    get_allocator().free(mem::to_bytes(vk_descriptor_sets));
}

void VulkanDriver::command_buffer_bind_vertex_buffers(Graphics::CommandBufferID command_buffer, u32 base_binding, const Slice<Graphics::BufferID>& buffers, const Slice<usize>& offsets)
{
    VKFailOn(command_buffer.is_valid() == false, "invalid command buffer");
    VKFailOn(buffers.len != offsets.len, "inconsistent buffers and offsets count");

    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    Slice<VkBuffer> vk_buffers = get_allocator().array<VkBuffer>(buffers.len);
    for(usize i = 0; i < buffers.len; i++)
    {
        vk_buffers[i] = _get_buffer(buffers[i]).vk_buffer;
    }

    ld.vk.vkCmdBindVertexBuffers(
        cmd_buffer.vk_command_buffer, base_binding,
        static_cast<uint32_t>(vk_buffers.len), vk_buffers.ptr(), offsets.ptr()
    );

    get_allocator().free(mem::to_bytes(vk_buffers));
}

void VulkanDriver::command_buffer_constant_block(Graphics::CommandBufferID command_buffer, Graphics::PipelineID pipeline, Graphics::ShaderStage stages, u32 offset, u32 size, MemoryAddress block_address)
{   
    VKFailOn(command_buffer.is_valid() == false, "invalid command buffer");
    VKFailOn(stages == Graphics::ShaderStage(0), "invalid shader stage");
    VKFailOn(size == 0, "invalid block size");

    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    Pipeline& pipe = _get_pipeline(pipeline);

    ld.vk.vkCmdPushConstants(
        cmd_buffer.vk_command_buffer, pipe.vk_pipeline_layout, _vk_get_shader_stage(stages),
        offset, size, reinterpret_cast<void*>(block_address)
    );
}

void VulkanDriver::command_buffer_set_viewports(Graphics::CommandBufferID command_buffer, u32 base_viewport, const Slice<Graphics::Viewport>& viewports)
{
    VKFailOn(command_buffer.is_valid() == false, "invalid command buffer");

    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    Slice<VkViewport> vk_viewports = get_allocator().array<VkViewport>(viewports.len);
    for(usize i = 0; i < viewports.len; i++)
    {
        vk_viewports[i] =
        {
            .x = viewports[i].x,
            .y = viewports[i].y + viewports[i].height,
            .width = viewports[i].width,
            .height = -viewports[i].height,
            .minDepth = viewports[i].min_depth,
            .maxDepth = viewports[i].max_depth,
        };
    }

    ld.vk.vkCmdSetViewport(cmd_buffer.vk_command_buffer, base_viewport, static_cast<uint32_t>(vk_viewports.len), vk_viewports.ptr());
    get_allocator().free(mem::to_bytes(vk_viewports));
}

void VulkanDriver::command_buffer_set_scissors(Graphics::CommandBufferID command_buffer, u32 base_scissor, const Slice<Graphics::Scissor>& scissors)
{
    VKFailOn(command_buffer.is_valid() == false, "invalid command buffer");

    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    Slice<VkRect2D> vk_scissors = get_allocator().array<VkRect2D>(scissors.len);
    for(usize i = 0; i < scissors.len; i++)
    {
        vk_scissors[i] =
        {
            .offset = { .x = scissors[i].x, .y = scissors[i].y },
            .extent = { .width = scissors[i].width, .height = scissors[i].height },
        };
    }

    ld.vk.vkCmdSetScissor(cmd_buffer.vk_command_buffer, base_scissor, static_cast<uint32_t>(vk_scissors.len), vk_scissors.ptr());
    get_allocator().free(mem::to_bytes(vk_scissors));
}

void VulkanDriver::command_buffer_draw(Graphics::CommandBufferID command_buffer, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance)
{
    VKFailOn(command_buffer.is_valid() == false, "invalid command buffer");

    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    ld.vk.vkCmdDraw(cmd_buffer.vk_command_buffer, vertex_count, instance_count, base_vertex, base_instance);
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

void VulkanDriver::_vk_get_surface_format(Graphics::SurfaceFormat surface_format, VkFormat* vk_image_format, VkColorSpaceKHR* vk_color_space)
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

VkPresentModeKHR VulkanDriver::_vk_get_present_mode(Graphics::PresentMode present_mode)
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

VkSurfaceCapabilitiesKHR VulkanDriver::_vk_get_surface_capabilities(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface)
{
    VkSurfaceCapabilitiesKHR capabilities;
    vk.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, vk_surface, &capabilities);
    return capabilities;   
}

VkExtent2D VulkanDriver::_vk_get_swap_chain_extent(const Vector2I& size, const VkSurfaceCapabilitiesKHR& vk_capabilities)
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

VkMemoryPropertyFlags VulkanDriver::_vk_get_memory_properties(Graphics::HeapUsage heap_usage)
{
    switch(heap_usage)
    {
    case Graphics::HeapUsage::CPUExclusive:
        return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    case Graphics::HeapUsage::GPUExclusive:
        return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    case Graphics::HeapUsage::CPUGPUCoherent:
        return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    default:
        break;
    }

    VKFailOn(true, "invalid heap usage");
    return VkMemoryPropertyFlags(0);
}

VkBufferUsageFlags VulkanDriver::_vk_get_buffer_usage(Graphics::BufferUsage buffer_usage)
{
    VkBufferUsageFlags vk_flags = 0;

    if(HasValue(buffer_usage & Graphics::BufferUsage::VertexBuffer))
    {
        vk_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if(HasValue(buffer_usage & Graphics::BufferUsage::IndexBuffer))
    {
        vk_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if(HasValue(buffer_usage & Graphics::BufferUsage::UniformBuffer))
    {
        vk_flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if(HasValue(buffer_usage & Graphics::BufferUsage::TransferSource))
    {
        vk_flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if(HasValue(buffer_usage & Graphics::BufferUsage::TransferDestination))
    {
        vk_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    
    return vk_flags;
}

VkDescriptorType VulkanDriver::_vk_get_descriptor_type(Graphics::DescriptorType descriptor_type)
{
    switch(descriptor_type)
    {
    case Graphics::DescriptorType::UniformBuffer:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case Graphics::DescriptorType::StorageBuffer:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    default:
        break;
    }

    VKFailOn(true, "invalid descriptor type");
    return VkDescriptorType(0);
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

VkShaderStageFlags VulkanDriver::_vk_get_shader_stage(Graphics::ShaderStage shader_stage)
{
    VkShaderStageFlags vk_flags = 0;
    
    if(HasValue(shader_stage & Graphics::ShaderStage::Vertex))
    {
        vk_flags |= VK_SHADER_STAGE_VERTEX_BIT;
    }

    if(HasValue(shader_stage & Graphics::ShaderStage::Fragment))
    {
        vk_flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    return vk_flags;
}

VkVertexInputRate VulkanDriver::_vk_get_input_rate(Graphics::InputRate input_rate)
{
    switch(input_rate)
    {
    case Graphics::InputRate::Vertex:
        return VK_VERTEX_INPUT_RATE_VERTEX;
    case Graphics::InputRate::Instance:
        return VK_VERTEX_INPUT_RATE_INSTANCE;
    default:
        break;
    };

    VKFailOn(true, "invalid input rate");
    return VkVertexInputRate();
}

VkFormat VulkanDriver::_vk_get_vertex_format(Graphics::VertexFormat vertex_format)
{
    switch(vertex_format)
    {
    case Graphics::VertexFormat::RGBA32Float:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case Graphics::VertexFormat::RGB32Float:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case Graphics::VertexFormat::RG32Float:
        return VK_FORMAT_R32G32_SFLOAT;
    case Graphics::VertexFormat::R32Float:
        return VK_FORMAT_R32_SFLOAT;
    default:
        break;
    };

    VKFailOn(true, "invalid vertex format");
    return VkFormat();
}

VkPrimitiveTopology VulkanDriver::_vk_get_topology(Graphics::PrimitiveTopology primitive_topology)
{
    switch(primitive_topology)
    {
    case Graphics::PrimitiveTopology::TriangleList:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case Graphics::PrimitiveTopology::LineList:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    default:
        break;
    };

    VKFailOn(true, "invalid primitive topology");
    return VkPrimitiveTopology();
}

VkPolygonMode VulkanDriver::_vk_get_polygon_mode(Graphics::PolygonMode polygon_mode)
{
    switch(polygon_mode)
    {
    case Graphics::PolygonMode::Fill:
        return VK_POLYGON_MODE_FILL;
    case Graphics::PolygonMode::Line:
        return VK_POLYGON_MODE_LINE;
    case Graphics::PolygonMode::Point:
        return VK_POLYGON_MODE_POINT;
    default:
        break;
    };

    VKFailOn(true, "invalid polygon mode");
    return VkPolygonMode();
}

VkCullModeFlags VulkanDriver::_vk_get_cull_mode(Graphics::CullMode cull_mode)
{
    switch(cull_mode)
    {
    case Graphics::CullMode::Front:
        return VK_CULL_MODE_FRONT_BIT;
    case Graphics::CullMode::Back:
        return VK_CULL_MODE_BACK_BIT;
    case Graphics::CullMode::FrontAndBack:
        return VK_CULL_MODE_FRONT_AND_BACK;
    default:
        break;
    };

    VKFailOn(true, "invalid cull mode");
    return VkCullModeFlags();
}

VkFrontFace VulkanDriver::_vk_get_front_face(Graphics::FrontFace front_face)
{
    switch(front_face)
    {
    case Graphics::FrontFace::CounterClockWise:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    case Graphics::FrontFace::ClockWise:
        return VK_FRONT_FACE_CLOCKWISE;
    default:
        break;
    };

    VKFailOn(true, "invalid front face");
    return VkFrontFace();
}

VkSampleCountFlagBits VulkanDriver::_vk_get_samples(Graphics::SampleCount sample_count)
{
    switch(sample_count)
    {
    case Graphics::SampleCount::Sample1:
        return VK_SAMPLE_COUNT_1_BIT;
    case Graphics::SampleCount::Sample2:
        return VK_SAMPLE_COUNT_2_BIT;
    case Graphics::SampleCount::Sample4:
        return VK_SAMPLE_COUNT_4_BIT;
    case Graphics::SampleCount::Sample8:
        return VK_SAMPLE_COUNT_8_BIT;
    case Graphics::SampleCount::Sample16:
        return VK_SAMPLE_COUNT_16_BIT;
    case Graphics::SampleCount::Sample32:
        return VK_SAMPLE_COUNT_32_BIT;
    default:
        break;
    }
    
    VKFailOn(true, "invalid sample count");
    return VkSampleCountFlagBits();
}

VkPipelineBindPoint VulkanDriver::_vk_get_bind_point(Graphics::PipelineBindPoint bind_point)
{
    switch(bind_point)
    {
    case Graphics::PipelineBindPoint::Graphics:
        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    default:
        break;
    }

    VKFailOn(true, "invalid bind point");
    return VkPipelineBindPoint();
}

VkShaderModule VulkanDriver::_vk_create_shader_module(LogicalDevice& ld, const Graphics::ShaderStageInfo& shader_stage_info)
{
    VkShaderModule vk_module;

    VkShaderModuleCreateInfo vk_shader_module_info =
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = static_cast<size_t>(shader_stage_info.code.len),
        .pCode = reinterpret_cast<const uint32_t*>(shader_stage_info.code.ptr()),
    };

    VkResult result = ld.vk.vkCreateShaderModule(ld.vk_device, &vk_shader_module_info, Vulkan::allocation_callbacks(), &vk_module);
    VKFailOn(result != VK_SUCCESS, "vkCreateShaderModule({})", Vulkan::result_as_string(result));

    return vk_module;
}

VkDescriptorSetLayout VulkanDriver::_vk_create_set_layout(LogicalDevice& ld, const Graphics::PipelineDescriptorSet& set_info)
{
    VkDescriptorSetLayout vk_set_layout;
    
    Slice<VkDescriptorSetLayoutBinding> vk_bindings = get_allocator().array<VkDescriptorSetLayoutBinding>(set_info.bindings.len);
    for(usize i = 0; i < set_info.bindings.len; i++)
    {
        vk_bindings[i] = 
        {
            .binding = set_info.bindings[i].binding,
            .descriptorType = _vk_get_descriptor_type(set_info.bindings[i].type),
            .descriptorCount = set_info.bindings[i].count,
            .stageFlags = _vk_get_shader_stage(set_info.bindings[i].stages),
            .pImmutableSamplers = nullptr,
        };
    }

    VkDescriptorSetLayoutCreateInfo vk_set_layout_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = static_cast<uint32_t>(vk_bindings.len),
        .pBindings = vk_bindings.ptr(),
    };

    VkResult result = ld.vk.vkCreateDescriptorSetLayout(ld.vk_device, &vk_set_layout_info, Vulkan::allocation_callbacks(), &vk_set_layout);
    VKFailOn(result != VK_SUCCESS, "vkCreateDescriptorSetLayout({})", Vulkan::result_as_string(result));

    get_allocator().free(mem::to_bytes(vk_bindings));
    return vk_set_layout;
}

Graphics::DeviceType VulkanDriver::_vk_device_type_to_device_type(VkPhysicalDeviceType vk_device_type)
{
    switch(vk_device_type)
    {
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return Graphics::DeviceType::IntegratedGPU;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return Graphics::DeviceType::DiscreteGPU;
    default:
        break;
    }

    VKFailOn(true, "invalid vulkan physical device type");
    return Graphics::DeviceType::Unknown;
}

Graphics::SurfaceFormat VulkanDriver::_vk_surface_format_to_surface_format(VkSurfaceFormatKHR vk_surface_format)
{
    switch(vk_surface_format.format)
    {
    case VK_FORMAT_R8G8B8A8_UNORM:
        return Graphics::SurfaceFormat::RGBA8Unorm;
    case VK_FORMAT_R8G8B8A8_SRGB:
        return Graphics::SurfaceFormat::RGBA8Srgb;
    case VK_FORMAT_B8G8R8A8_UNORM:
        return Graphics::SurfaceFormat::BGRA8Unorm;
    case VK_FORMAT_B8G8R8A8_SRGB:
        return Graphics::SurfaceFormat::BGRA8Srgb;
    default:
        break;
    }

    VKFailOn(true, "invalid vulkan surface format");
    return Graphics::SurfaceFormat::Unknown;
}

Graphics::PresentMode VulkanDriver::_vk_present_mode_to_present_mode(VkPresentModeKHR vk_present_mode)
{
    switch(vk_present_mode)
    {
    case VK_PRESENT_MODE_IMMEDIATE_KHR:
        return Graphics::PresentMode::Immediate;
    case VK_PRESENT_MODE_FIFO_KHR:
        return Graphics::PresentMode::VSync;
    default:
        break;
    }

    VKFailOn(true, "invalid vulkan present mode");
    return Graphics::PresentMode::Unknown;
}

