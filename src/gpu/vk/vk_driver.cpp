#include "gpu/vk/vk_driver.h"

#include "core/templates.h"
#include "display/display.h"
#include "gpu/gpu.h"
#include "gpu/vk/vk_utils.h"
#include "math/funcs.h"
#include "os/os.h"



InternalGPU::GPUAdapter VulkanDriver::get_adapter()
{
    return InternalGPU::GPUAdapter
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
        .swap_chain_get_image_count = &VulkanDriver::swap_chain_get_image_count,
        .swap_chain_get_texture = &VulkanDriver::swap_chain_get_texture,
        .swap_chain_acquire_next_image = &VulkanDriver::swap_chain_acquire_next_image,
        .fence_create = &VulkanDriver::fence_create,
        .fence_destroy = &VulkanDriver::fence_destroy,
        .fence_get_state = &VulkanDriver::fence_get_state,
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
        .memory_heap_map = &VulkanDriver::memory_heap_map,
        .memory_heap_unmap = &VulkanDriver::memory_heap_unmap,
        .buffer_create = &VulkanDriver::buffer_create,
        .buffer_destroy = &VulkanDriver::buffer_destroy,
        .sampler_create = &VulkanDriver::sampler_create,
        .sampler_destroy = &VulkanDriver::sampler_destroy,
        .texture_create = &VulkanDriver::texture_create,
        .texture_destroy = &VulkanDriver::texture_destroy,
        .descriptor_set_layout_create = &VulkanDriver::descriptor_set_layout_create,
        .descriptor_set_layout_destroy = &VulkanDriver::descriptor_set_layout_destroy,
        .descriptor_pool_create = &VulkanDriver::descriptor_pool_create,
        .descriptor_pool_destroy = &VulkanDriver::descriptor_pool_destroy,
        .descriptor_set_allocate = &VulkanDriver::descriptor_set_allocate,
        .descriptor_set_free = &VulkanDriver::descriptor_set_free,
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
        .command_buffer_copy_buffer_to_texture = &VulkanDriver::command_buffer_copy_buffer_to_texture,
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
    VKDebugInfo("Initializing Vulkan Driver...");
    data.allocator = allocator;

    data.tmp_allocator.init(OS::map_memory(1024*1024, OS::MapReadWrite));
    
    data.surfaces = FreeList<Surface, GPU::SurfaceID>::with_allocator(allocator);
    data.devices = FreeList<LogicalDevice, GPU::DeviceID>::with_allocator(allocator);
    data.swap_chains = FreeList<SwapChain, GPU::SwapChainID>::with_allocator(allocator);
    data.fences = FreeList<Fence, GPU::FenceID>::with_allocator(allocator);
    data.semaphores = FreeList<Semaphore, GPU::SemaphoreID>::with_allocator(allocator);
    data.queues = FreeList<Queue, GPU::QueueID>::with_allocator(allocator);
    data.memory_heaps = FreeList<MemoryHeap, GPU::MemoryHeapID>::with_allocator(allocator);
    data.buffers = FreeList<Buffer, GPU::BufferID>::with_allocator(allocator);
    data.samplers = FreeList<Sampler, GPU::SamplerID>::with_allocator(allocator);
    data.textures = FreeList<Texture, GPU::TextureID>::with_allocator(allocator);
    data.descriptor_set_layouts = FreeList<DescriptorSetLayout, GPU::DescriptorSetLayoutID>::with_allocator(allocator);
    data.descriptor_pools = FreeList<DescriptorPool, GPU::DescriptorPoolID>::with_allocator(allocator);
    data.descriptor_sets = FreeList<DescriptorSet, GPU::DescriptorSetID>::with_allocator(allocator);
    data.pipelines = FreeList<Pipeline, GPU::PipelineID>::with_allocator(allocator);
    data.command_pools = FreeList<CommandPool, GPU::CommandPoolID>::with_allocator(allocator);
    data.command_buffers = FreeList<CommandBuffer, GPU::CommandBufferID>::with_allocator(allocator);

#if defined(BREAD_ANDROID)
    data.vk_lib = OS::load_library("libvulkan.so");
#else
    data.vk_lib = OS::load_library("vulkan-1.dll");
#endif

    Vulkan::load_core_procs(data.vk_lib);

    data.info.api_version = Vulkan::get_api_version();
    VKFailOn(data.info.api_version < VK_API_VERSION_1_1, "vulkan 1.1 was expected");
    VKDebugInfo(
        "Vulkan API Version: {}.{}.{}",
        VK_API_VERSION_MAJOR(data.info.api_version),
        VK_API_VERSION_MINOR(data.info.api_version),
        VK_API_VERSION_PATCH(data.info.api_version)
    );

    data.instance = Vulkan::create_instance();
    Vulkan::load_instance_procs(data.instance);

    // VK_ext_debug_util
#if defined(BREAD_SHOW_DEBUG_INFO) && defined(BREAD_WIN32)
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

#if defined(BREAD_WIN32)
    data.dummy_surface = Vulkan::create_surface(data.instance, 0);
#elif defined(BREAD_ANDROID)
    data.dummy_surface = VK_NULL_HANDLE;
#endif
}

void VulkanDriver::shutdown()
{
    OS::unmap_memory(data.tmp_allocator.sp);

    vk.vkDestroySurfaceKHR(data.instance, data.dummy_surface, Vulkan::allocation_callbacks());

    data.surfaces.destroy();
    data.devices.destroy();
    data.swap_chains.destroy();
    data.fences.destroy();
    data.semaphores.destroy();
    data.queues.destroy();
    data.memory_heaps.destroy();
    data.buffers.destroy();
    data.samplers.destroy();
    data.textures.destroy();
    data.descriptor_set_layouts.destroy();
    data.descriptor_pools.destroy();
    data.descriptor_sets.destroy();
    data.pipelines.destroy();
    data.command_pools.destroy();
    data.command_buffers.destroy();

    get_allocator().free(mem::to_bytes(data.physical_device_ids));
    get_allocator().free(mem::to_bytes(data.physical_devices));

#if defined(BREAD_SHOW_DEBUG_INFO) && defined(BREAD_WIN32)
    vk.vkDestroyDebugUtilsMessengerEXT(
        data.instance, data.messenger, Vulkan::allocation_callbacks()
    );
#endif

    vk.vkDestroyInstance(data.instance, Vulkan::allocation_callbacks());

    OS::unload_library(data.vk_lib);
}

Slice<GPU::PhysicalDeviceID> VulkanDriver::physical_devices_enumerate()
{
    return data.physical_device_ids;
}

GPU::PhysicalDeviceInfo VulkanDriver::physical_device_get_info(GPU::PhysicalDeviceID physical_device)
{
    VKFailOn(physical_device.integer() >= data.physical_devices.len);
    PhysicalDevice& pd = data.physical_devices[physical_device.integer()];
    return pd.info;
}

GPU::SurfaceID VulkanDriver::surface_create(const GPU::SurfaceCreateInfo &ci)
{
    GPU::SurfaceID surface_id = data.surfaces.add(Surface());
    Surface& surface = _get_surface(surface_id);
#if defined(BREAD_WIN32)
    surface.window_native_handle = ci.window_native_handle;
    surface.vk_surface = Vulkan::create_surface(data.instance, ci.window_native_handle);
#elif defined(BREAD_ANDROID)
    if(data.dummy_surface == VK_NULL_HANDLE)
    {
        data.dummy_surface = Vulkan::create_surface(data.instance, Display::window_get_native_handle(Display::WindowID()));
    }
    Unused(ci);
    surface.vk_surface = data.dummy_surface;
#endif
    return surface_id;
}

void VulkanDriver::surface_destroy(GPU::SurfaceID surface)
{
#if defined(BREAD_WIN32)
    Surface& s = _get_surface(surface);
    vk.vkDestroySurfaceKHR(data.instance, s.vk_surface, Vulkan::allocation_callbacks());

#elif defined(BREAD_ANDROID)
    Unused(surface);
#endif
    data.surfaces.remove(surface);
}

GPU::DeviceID VulkanDriver::device_create(const GPU::DeviceCreateInfo& ci)
{
    VKFailOn(
        ci.physical_device.integer() >= data.physical_devices.len, 
        "invalid physical device"
    );

    PhysicalDevice& pd = data.physical_devices[ci.physical_device.integer()];
    mem::Allocator allocator = acquire_tmp_allocator();
    
    GPU::DeviceID device_id = data.devices.add(LogicalDevice());
    LogicalDevice& ld = _get_logical_device(device_id);
    ld.vk_physical_device = pd.vk_physical_device;

    u32 property_count;
    vk.vkEnumerateDeviceExtensionProperties(pd.vk_physical_device, nullptr, &property_count, nullptr);
    
    Slice<VkExtensionProperties> device_extensions = allocator.array<VkExtensionProperties>(property_count);
    vk.vkEnumerateDeviceExtensionProperties(pd.vk_physical_device, nullptr, &property_count, device_extensions.ptr());
    
    VKDebugInfo("Device Extensions");
    for(VkExtensionProperties& extension : device_extensions)
    {
        VKDebugInfo("{}", Vulkan::vulkan_string_to_sv(extension.extensionName));
    }

    // Checking for required extensions and features use by the driver.
    Vulkan::AdditionalExtensionSupport add_ext = Vulkan::check_device_extensions(pd.vk_physical_device);
    Vulkan::check_device_features(pd.vk_physical_device);

    u32 vk_family_count;
    vk.vkGetPhysicalDeviceQueueFamilyProperties2(pd.vk_physical_device, &vk_family_count, nullptr);

    Slice<VkQueueFamilyProperties2> vk_families = allocator.array<VkQueueFamilyProperties2>(vk_family_count);
    for(VkQueueFamilyProperties2& vk_family : vk_families)
    {
        vk_family.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
        vk_family.pNext = nullptr;
    }
    vk.vkGetPhysicalDeviceQueueFamilyProperties2(pd.vk_physical_device, &vk_family_count, vk_families.ptr());    

    // 0->graphics, 1->present
    uint32_t vk_graphics_index = MaxValue<uint32_t>;
    uint32_t vk_compute_index = MaxValue<uint32_t>;
    uint32_t vk_copy_index = MaxValue<uint32_t>;
    uint32_t vk_present_index = MaxValue<uint32_t>;

    for(usize i = 0; i < vk_families.len; i++)
    {
        VkQueueFamilyProperties2 family = vk_families[i];
        bool has_graphics = HasValue(family.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT);
        bool has_compute = HasValue(family.queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT);
        bool has_copy = HasValue(family.queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT);

        if(vk_graphics_index == MaxValue<uint32_t> && has_graphics)
        {
            // graphics
            vk_graphics_index = static_cast<uint32_t>(i);
        }
        if(vk_compute_index == MaxValue<uint32_t> && has_compute && !has_graphics)
        {
            // exclusive compute
            vk_compute_index = static_cast<uint32_t>(i);
        }
        if(vk_copy_index == MaxValue<uint32_t> && has_copy && !has_graphics)
        {
            // exclusive copy
            vk_copy_index = static_cast<uint32_t>(i);
        }

        VkBool32 supported = false;
        vk.vkGetPhysicalDeviceSurfaceSupportKHR(pd.vk_physical_device, static_cast<uint32_t>(i), data.dummy_surface, &supported);
        if(vk_present_index == MaxValue<uint32_t> && supported)
        {
            vk_present_index = static_cast<uint32_t>(i);
        }
    }
    VKFailOn(vk_graphics_index == MaxValue<uint32_t> || vk_present_index == MaxValue<uint32_t>,
        "graphics and present is required");

    // Default to graphics queue, graphics and present are expected to be.
    if(vk_compute_index == MaxValue<uint32_t>
        && vk_families[vk_graphics_index].queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)
    {
        vk_compute_index = vk_graphics_index;
    }

    if(vk_copy_index == MaxValue<uint32_t>)
    {
        // Prefer the compute queue.
        if(vk_compute_index != vk_graphics_index)
        {
            vk_copy_index = vk_compute_index;
        }
        else
        {
            vk_copy_index = vk_compute_index;
        }
    }
    
    static constexpr usize VkFamilyCount = 4; // Graphics, Compute, Copy, Present

    uint32_t vk_family_indices[] =
    {
        vk_graphics_index, vk_compute_index, vk_copy_index, vk_present_index
    };
    ArrayIterator<uint32_t> iterator = {.base = vk_family_indices, .extent = ArraySize(vk_family_indices) };

    uint32_t uniques[VkFamilyCount] = {};
    uint32_t unique_count = 0;

    (void)iterator.for_each(
        [&](uint32_t v)
        {
            bool finded = false;;
            for(usize i = 0; i < unique_count; i++)
            {
                if(uniques[i] == v && v != MaxValue<uint32_t>)
                {
                    finded = true;
                    break;
                }
            }

            if(!finded)
            {
                uniques[unique_count] = v;
                unique_count++;
            }
        }
    );
    
    VkDeviceQueueCreateInfo queue_infos[VkFamilyCount] = {};
    for(usize i = 0; i < unique_count; i++)
    {
        f32 priority = 1.F;
        queue_infos[i] =
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = uniques[i],
            .queueCount = 1,
            .pQueuePriorities = &priority,
        };
    }

    uint32_t extension_count = 0;
    const char** extensions = Vulkan::get_device_extensions(pd.vk_physical_device, add_ext, allocator, &extension_count);
    VkDeviceCreateInfo vk_device_info =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = Vulkan::get_device_features(add_ext, allocator),
        .flags = 0,
        .queueCreateInfoCount = unique_count,
        .pQueueCreateInfos = queue_infos,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = extension_count,
        .ppEnabledExtensionNames = extensions,
        .pEnabledFeatures = nullptr,
    };

    VkResult result = vk.vkCreateDevice(pd.vk_physical_device, &vk_device_info, Vulkan::allocation_callbacks(), &ld.vk_device);
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

    ld.additional_extension_support = add_ext;

    // Queues
    GPU::QueueUsage queue_usages[VkFamilyCount] =
    {
        GPU::QueueUsage::Graphics,
        GPU::QueueUsage::Compute,
        GPU::QueueUsage::Copy,
        GPU::QueueUsage::Present,
    };

    ld.families = get_allocator().array<LogicalDevice::QueueFamily>(unique_count);
    for(usize i = 0; i < ld.families.len; i++)
    {
        LogicalDevice::QueueFamily& family = ld.families[i];

        family.usage = queue_usages[i];
        family.vk_family_index = vk_family_indices[i];

        family.vk_queues = get_allocator().array<VkQueue>(1);
        ld.vk.vkGetDeviceQueue(ld.vk_device, family.vk_family_index, 0, family.vk_queues.ptr());
    }

    ld.device_queues = get_allocator().array<LogicalDevice::DeviceQueue>(VkFamilyCount);
    for(usize i = 0; i < ld.device_queues.len; i++)
    {
        for(usize family_i = 0; family_i < ld.families.len; family_i++)
        {
            if(vk_family_indices[i] == ld.families[family_i].vk_family_index)
            {
                ld.device_queues[i].family_index = family_i;
                continue;
            }
        }
    }

    ld.render_pass_cache = HashMap<VkDriverRenderPassKey, RenderPassCache>::with_allocator(get_allocator());

    ld.device = device_id;

    return device_id;
}

void VulkanDriver::device_destroy(GPU::DeviceID device)
{
    LogicalDevice& ld = _get_logical_device(device);
    
    for(usize i = 0; i < ld.families.len; i++)
    {
        LogicalDevice::QueueFamily& queue_family = ld.families[i];

        get_allocator().free(mem::to_bytes(queue_family.vk_queues));
    }
    get_allocator().free(mem::to_bytes(ld.device_queues));
    get_allocator().free(mem::to_bytes(ld.families));
    
    for(RenderPassEntry& it : ld.render_pass_cache.iter())
    {
        for(FramebufferEntry framebuffer : it.second.vk_framebuffers_cache.iter())
        {
            ld.vk.vkDestroyFramebuffer(
                ld.vk_device, framebuffer.second,
                Vulkan::allocation_callbacks()
            );
        }
        it.second.vk_framebuffers_cache.destroy();
        ld.vk.vkDestroyRenderPass(ld.vk_device, it.second.vk_render_pass, Vulkan::allocation_callbacks());
    }
    ld.render_pass_cache.destroy();
    
    vk.vkDestroyDevice(ld.vk_device, Vulkan::allocation_callbacks());
    data.devices.remove(device);
}

GPU::SwapChainID VulkanDriver::swap_chain_create(const GPU::SwapChainCreateInfo& ci)
{
    LogicalDevice& ld = _get_logical_device(ci.device);
    mem::Allocator allocator = acquire_tmp_allocator();

    GPU::SwapChainID sc_id = data.swap_chains.add(SwapChain());
    SwapChain& swap_chain = _get_swap_chain(sc_id);
    Surface& surface = _get_surface(ci.surface);

    // Creating the surface
    swap_chain.vk_device = ld.vk_device;
    swap_chain.vk_surface = surface.vk_surface;
    swap_chain.surface = ci.surface;
    swap_chain.device = ci.device;
    swap_chain.image_count = 0;

    VkFormat vk_swapchain_format;
    VkColorSpaceKHR vk_swap_chain_color_space;
    _vk_get_surface_format(ci.format, &vk_swapchain_format, &vk_swap_chain_color_space);
    VkPresentModeKHR vk_present_mode = VkUtils::_vk_get_present_mode(ci.present_mode);

    VkSurfaceCapabilitiesKHR capabilities = _vk_get_surface_capabilities(ld.vk_physical_device, surface.vk_surface);
    VkExtent2D vk_swap_chain_extent = _vk_get_swap_chain_extent(ci.size, capabilities);

    VkSwapchainCreateInfoKHR swap_chain_info =
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = surface.vk_surface,
        .minImageCount = ci.min_image_count,
        .imageFormat = vk_swapchain_format,
        .imageColorSpace = vk_swap_chain_color_space,
        .imageExtent = vk_swap_chain_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = & ld.families[ld.device_queues[3].family_index].vk_family_index,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = vk_present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    VkResult result = ld.vk.vkCreateSwapchainKHR(ld.vk_device, &swap_chain_info, Vulkan::allocation_callbacks(), &swap_chain.vk_swapchain);
    VKFailOn(result != VK_SUCCESS, "vkCreateSwapchainKHR({})", Vulkan::result_as_string(result));

    {
        // images
        ld.vk.vkGetSwapchainImagesKHR(ld.vk_device, swap_chain.vk_swapchain, &swap_chain.image_count, nullptr);

        Slice<VkImage> vk_images = allocator.array<VkImage>(swap_chain.image_count);
        swap_chain.images = get_allocator().array<SwapChainImage>(swap_chain.image_count);
        ld.vk.vkGetSwapchainImagesKHR(ld.vk_device, swap_chain.vk_swapchain, &swap_chain.image_count, vk_images.ptr());
        for (u32 i = 0; i < swap_chain.image_count; i++)
        {
            swap_chain.images[i].vk_image = vk_images[i];
        }
    }

    {
        // image views
        VkImageViewCreateInfo vk_image_view_info =
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

        for(u32 i = 0; i < swap_chain.image_count; i++)
        {
            vk_image_view_info.image = swap_chain.images[i].vk_image;
            ld.vk.vkCreateImageView(ld.vk_device, &vk_image_view_info, Vulkan::allocation_callbacks(), &swap_chain.images[i].vk_image_view);
        }

        // texture object
        for (u32 i = 0; i < swap_chain.image_count; i++)
        {
            swap_chain.images[i].texture = data.textures.add(Texture());
            Texture& tex = _get_texture(swap_chain.images[i].texture);
            tex.vk_device = swap_chain.vk_device;
            tex.vk_image = swap_chain.images[i].vk_image;
            tex.vk_image_view = swap_chain.images[i].vk_image_view;
            tex.vk_format = VkUtils::_vk_get_texture_format(ci.format);
		    tex.format = ci.format;
            tex.extent = Vector3U(vk_swap_chain_extent.width, vk_swap_chain_extent.height, 1);
            tex.device = swap_chain.device;
        }
    }

    return sc_id;
}

void VulkanDriver::swap_chain_destroy(GPU::SwapChainID swap_chain)
{
    SwapChain& sc = _get_swap_chain(swap_chain);
    LogicalDevice& ld = _get_logical_device(sc.device);

    for(u32 i = 0; i < sc.image_count; i++)
    {
        ld.vk.vkDestroyImageView(sc.vk_device, sc.images[i].vk_image_view, Vulkan::allocation_callbacks());

        Texture& texture = _get_texture(sc.images[i].texture);

        for(RenderPassEntry& render_pass_entry : ld.render_pass_cache.iter())
        {
            if(render_pass_entry.second.vk_framebuffers_cache.has(texture.vk_image_view))
            {
                ld.vk.vkDestroyFramebuffer(
                    ld.vk_device, render_pass_entry.second.vk_framebuffers_cache.get(texture.vk_image_view),
                    Vulkan::allocation_callbacks()
                );
                render_pass_entry.second.vk_framebuffers_cache.remove(texture.vk_image_view);
            }
        }
     
        data.textures.remove(sc.images[i].texture);
    }
    get_allocator().free(mem::to_bytes(sc.images));
    
    ld.vk.vkDestroySwapchainKHR(ld.vk_device, sc.vk_swapchain, Vulkan::allocation_callbacks());

    data.swap_chains.remove(swap_chain);
}

u32 VulkanDriver::swap_chain_get_image_count(GPU::SwapChainID swap_chain)
{
    SwapChain& sc = _get_swap_chain(swap_chain);
    return sc.image_count; 
}

GPU::TextureID VulkanDriver::swap_chain_get_texture(GPU::SwapChainID swap_chain, u32 image_index)
{
    SwapChain& sc = _get_swap_chain(swap_chain);
    return sc.images[image_index].texture;
}

GPU::AcquireResult VulkanDriver::swap_chain_acquire_next_image(GPU::SwapChainID swap_chain, const GPU::AcquireInfo& acquire_info, u32* image_index)
{
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
    VKFailOn(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR && result != VK_ERROR_OUT_OF_DATE_KHR, "vkAcquireNextImageKHR({})", Vulkan::result_as_string(result));

    if(result == VK_SUBOPTIMAL_KHR)
    {
        return GPU::AcquireResult::Suboptimal;
    }
    else if(result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        return GPU::AcquireResult::OutOfDate;
    }

    return GPU::AcquireResult::Acquired;
}

GPU::FenceID VulkanDriver::fence_create(const GPU::FenceCreateInfo& ci)
{
    LogicalDevice& ld = _get_logical_device(ci.device);
    
    GPU::FenceID fence_id = data.fences.add(Fence());
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

void VulkanDriver::fence_destroy(GPU::FenceID fence)
{
    Fence& f = _get_fence(fence);
    LogicalDevice& ld = _get_logical_device(f.device);

    ld.vk.vkDestroyFence(f.vk_device, f.vk_fence, Vulkan::allocation_callbacks());

    data.fences.remove(fence);
}

bool VulkanDriver::fence_get_state(GPU::FenceID fence)
{
    Fence& f = _get_fence(fence);
    LogicalDevice& ld = _get_logical_device(f.device);

    VkResult result = ld.vk.vkGetFenceStatus(ld.vk_device, f.vk_fence);

    return result == VK_SUCCESS ? true : false;
}

void VulkanDriver::fence_reset(Slice<GPU::FenceID> fences)
{
    GPU::DeviceID first_device = _get_fence(fences[0]).device;
    LogicalDevice& ld = _get_logical_device(first_device);
    mem::Allocator allocator = acquire_tmp_allocator();

    Slice<VkFence> vk_fences = allocator.array<VkFence>(fences.len);

    for(usize i = 0; i < fences.len; i++)
    {
        Fence& fence = _get_fence(fences[i]);
        VKFailOn(first_device != fence.device, "fences must share the same device");

        vk_fences[i] = fence.vk_fence;
    }

    VkResult result = ld.vk.vkResetFences(ld.vk_device, static_cast<uint32_t>(vk_fences.len), vk_fences.ptr());
    VKFailOn(result != VK_SUCCESS, "vkResetFences({})", Vulkan::result_as_string(result));
}

void VulkanDriver::fence_wait_for(Slice<GPU::FenceID> fences, bool wait_for_all, u64 timeout)
{
    GPU::DeviceID first_device = _get_fence(fences[0]).device;
    LogicalDevice& ld = _get_logical_device(first_device);
    mem::Allocator allocator = acquire_tmp_allocator();
    
    Slice<VkFence> vk_fences = allocator.array<VkFence>(fences.len);
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
}

GPU::SemaphoreID VulkanDriver::semaphore_create(const GPU::SemaphoreCreateInfo &ci)
{
    GPU::SemaphoreID semaphore_id = data.semaphores.add(Semaphore());
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

void VulkanDriver::semaphore_destroy(GPU::SemaphoreID semaphore)
{
    Semaphore& sem = _get_semaphore(semaphore);
    LogicalDevice& ld = _get_logical_device(sem.device);

    ld.vk.vkDestroySemaphore(sem.vk_device, sem.vk_semaphore, Vulkan::allocation_callbacks());
    
    data.semaphores.remove(semaphore);
}

GPU::QueueID VulkanDriver::queue_create(const GPU::QueueCreateInfo& ci)
{
    LogicalDevice& ld = _get_logical_device(ci.device);

    usize device_queue_index = usize(ci.usage) - 1;
    LogicalDevice::QueueFamily& family = ld.families[ld.device_queues[device_queue_index].family_index];
    GPU::QueueID queue_id = data.queues.add(Queue());
    Queue& q = _get_queue(queue_id);

    q.vk_device = ld.vk_device;
    q.vk_queue = family.vk_queues[0];
    q.device_queue_index = device_queue_index;
    q.device = ld.device;
    q.queue = queue_id;

    return queue_id;
}

void VulkanDriver::queue_destroy(GPU::QueueID queue)
{
    data.queues.remove(queue);
}

void VulkanDriver::queue_execute_command_buffer(GPU::QueueID queue, const GPU::QueueExecuteInfo& execute_info)
{
    Queue& q = _get_queue(queue);
    LogicalDevice& ld = _get_logical_device(q.device);
    mem::Allocator allocator = acquire_tmp_allocator();

    Slice<VkSemaphore> vk_wait_sem = allocator.array<VkSemaphore>(execute_info.wait_semaphores.len);
    Slice<VkPipelineStageFlags> vk_wait_stages = allocator.array<VkPipelineStageFlags>(execute_info.wait_stages.len);
    for(usize i = 0; i < execute_info.wait_semaphores.len; i++)
    {
        Semaphore& sem = _get_semaphore(execute_info.wait_semaphores[i]);
        vk_wait_sem[i] = sem.vk_semaphore;
        vk_wait_stages[i] = VkUtils::_vk_get_pipeline_stages(execute_info.wait_stages[i]);
    }

    Slice<VkSemaphore> vk_signal_sem = allocator.array<VkSemaphore>(execute_info.signal_semaphores.len);
    for(usize i = 0; i < execute_info.signal_semaphores.len; i++)
    {
        Semaphore& sem = _get_semaphore(execute_info.signal_semaphores[i]);
        vk_signal_sem[i] = sem.vk_semaphore;
    }

    Slice<VkCommandBuffer> vk_cmd_buffers = allocator.array<VkCommandBuffer>(execute_info.command_buffers.len);
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

    VkSubmitInfo submit_info =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = static_cast<uint32_t>(vk_wait_sem.len),
        .pWaitSemaphores = vk_wait_sem.ptr(),
        .pWaitDstStageMask = vk_wait_stages.ptr(),
        .commandBufferCount = static_cast<uint32_t>(vk_cmd_buffers.len),
        .pCommandBuffers = vk_cmd_buffers.ptr(),
        .signalSemaphoreCount = static_cast<uint32_t>(vk_signal_sem.len),
        .pSignalSemaphores = vk_signal_sem.ptr(),
    };

    VkResult result = ld.vk.vkQueueSubmit(q.vk_queue, 1, &submit_info, vk_fence);
    VKFailOn(result != VK_SUCCESS, "vkQueueSubmit({})", Vulkan::result_as_string(result));
}

GPU::AcquireResult VulkanDriver::queue_present(GPU::QueueID queue, const GPU::QueuePresentInfo& present_info)
{
    Queue& q = _get_queue(queue);
    LogicalDevice& ld = _get_logical_device(q.device);
    mem::Allocator allocator = acquire_tmp_allocator();

    Slice<VkSemaphore> vk_wait_semaphores = allocator.array<VkSemaphore>(present_info.wait_semaphores.len);
    for(usize i = 0; i < present_info.wait_semaphores.len; i++)
    {
        Semaphore& sem = _get_semaphore(present_info.wait_semaphores[i]);
        vk_wait_semaphores[i] = sem.vk_semaphore;
    }

    Slice<VkSwapchainKHR> vk_swapchains = allocator.array<VkSwapchainKHR>(present_info.swapchains.len);
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
    VKFailOn(call_result != VK_SUCCESS && call_result != VK_SUBOPTIMAL_KHR && call_result != VK_ERROR_OUT_OF_DATE_KHR, "vkQueuePresentKHR({})", Vulkan::result_as_string(call_result));

    if(call_result == VK_SUBOPTIMAL_KHR)
    {
        return GPU::AcquireResult::Suboptimal;
    }
    else if(call_result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        return GPU::AcquireResult::OutOfDate;
    }

    return GPU::AcquireResult::Acquired;
}

void VulkanDriver::queue_wait_idle(GPU::QueueID queue)
{
    Queue& q = _get_queue(queue);
    LogicalDevice& ld = _get_logical_device(q.device);

    VkResult result = ld.vk.vkQueueWaitIdle(q.vk_queue);
    VKFailOn(result != VK_SUCCESS, "vkQueueWaitIdle({})", Vulkan::result_as_string(result));
}

GPU::MemoryHeapID VulkanDriver::memory_heap_create(const GPU::MemoryHeapCreateInfo& ci)
{
    LogicalDevice& ld = _get_logical_device(ci.device);
    GPU::MemoryHeapID memory_heap_id = data.memory_heaps.add(MemoryHeap());
    MemoryHeap& heap = _get_memory_heap(memory_heap_id);
    heap.vk_device = ld.vk_device;
    heap.device = ci.device;
    heap.memory_heap = memory_heap_id;

    uint32_t vk_type_index = MaxValue<uint32_t>;
    VkMemoryPropertyFlags vk_memory_flags = VkUtils::_vk_get_memory_properties(ci.heap_usage);

    for(uint32_t i = 0; i < ld.vk_physical_device_memory_properties.memoryTypeCount; i++)
    {
        VkMemoryType mem_type = ld.vk_physical_device_memory_properties.memoryTypes[i];
        if((mem_type.propertyFlags & vk_memory_flags) == vk_memory_flags)
        {
            vk_type_index = i;
            break;
        }
    }

    if(vk_type_index == MaxValue<uint32_t> && ci.heap_usage == GPU::HeapUsage::CPUGPUCoherent)
    {
        vk_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        for(uint32_t i = 0; i < ld.vk_physical_device_memory_properties.memoryTypeCount; i++)
        {
            VkMemoryType mem_type = ld.vk_physical_device_memory_properties.memoryTypes[i];
            if((mem_type.propertyFlags & vk_memory_flags) == vk_memory_flags)
            {
                vk_type_index = i;
                break;
            }
        }
    }

    VkMemoryAllocateInfo vk_allocate_info =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = ci.heap_size,
        .memoryTypeIndex = vk_type_index,
    };

    heap.vk_type_index = vk_type_index;
    heap.vk_memory_flags = vk_memory_flags;

    VkResult result = ld.vk.vkAllocateMemory(ld.vk_device, &vk_allocate_info, Vulkan::allocation_callbacks(), &heap.vk_memory);
    VKFailOn(result != VK_SUCCESS, "vkAllocateMemory({})", Vulkan::result_as_string(result));

    return memory_heap_id;
}

void VulkanDriver::memory_heap_destroy(GPU::MemoryHeapID memory_heap)
{
    MemoryHeap& heap = _get_memory_heap(memory_heap);
    LogicalDevice& ld = _get_logical_device(heap.device);

    ld.vk.vkFreeMemory(heap.vk_device, heap.vk_memory, Vulkan::allocation_callbacks());

    data.memory_heaps.remove(memory_heap);
}

Slice<u8> VulkanDriver::memory_heap_map(GPU::MemoryHeapID memory_heap, usize offset, usize len)
{
    MemoryHeap& heap = _get_memory_heap(memory_heap);
    LogicalDevice& ld = _get_logical_device(heap.device);

    void* ptr = nullptr;

    VkResult result = ld.vk.vkMapMemory(ld.vk_device, heap.vk_memory, offset, len, 0, &ptr);
    VKFailOn(result != VK_SUCCESS, "vkMapMemory({})", Vulkan::result_as_string(result));

    return Slice<u8>(reinterpret_cast<u8*>(ptr), len);
}

void VulkanDriver::memory_heap_unmap(GPU::MemoryHeapID memory_heap, const Slice<u8>& memory)
{
    Unused(memory);
    MemoryHeap& heap = _get_memory_heap(memory_heap);
    LogicalDevice& ld = _get_logical_device(heap.device);

    ld.vk.vkUnmapMemory(ld.vk_device, heap.vk_memory);
}

GPU::BufferID VulkanDriver::buffer_create(const GPU::BufferCreateInfo& ci)
{
    LogicalDevice& ld = _get_logical_device(ci.device);
    GPU::BufferID buffer_id = data.buffers.add(Buffer());
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
        .usage = VkUtils::_vk_get_buffer_usage(ci.usage),
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

void VulkanDriver::buffer_destroy(GPU::BufferID buffer)
{
    Buffer& b = _get_buffer(buffer);
    LogicalDevice& ld = _get_logical_device(b.device);

    ld.vk.vkDestroyBuffer(b.vk_device, b.vk_buffer, Vulkan::allocation_callbacks());

    data.buffers.remove(buffer);
}

GPU::SamplerID VulkanDriver::sampler_create(const GPU::SamplerCreateInfo& ci)
{
    GPU::SamplerID sampler_id = data.samplers.add(Sampler());
    Sampler& sam = _get_sampler(sampler_id);
    LogicalDevice& ld = _get_logical_device(ci.device);

    sam.vk_device = ld.vk_device;
    sam.device = ci.device;
    sam.sampler = sampler_id;

    VkSamplerCreateInfo vk_sampler_info =
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = VkUtils::_vk_get_filter(ci.mag_filter),
        .minFilter = VkUtils::_vk_get_filter(ci.min_filter),
        .mipmapMode = VkUtils::_vk_get_mipmap_mode(ci.mipmap_mode),
        .addressModeU = VkUtils::_vk_get_address_mode(ci.address_mode_u),
        .addressModeV = VkUtils::_vk_get_address_mode(ci.address_mode_v),
        .addressModeW = VkUtils::_vk_get_address_mode(ci.address_mode_w),
        .mipLodBias = ci.mip_lod_bias,
        .anisotropyEnable = ci.anisotropy_enable,
        .maxAnisotropy = ci.max_anisotropy,
        .compareEnable = ci.compare_enable,
        .compareOp = VkUtils::_vk_get_compare_op(ci.compare_op),
        .minLod = ci.min_lod,
        .maxLod = ci.max_lod,
        .borderColor = {},
        .unnormalizedCoordinates = VK_FALSE,
    };

    ld.vk.vkCreateSampler(ld.vk_device, &vk_sampler_info, Vulkan::allocation_callbacks(), &sam.vk_sampler);

    return sampler_id;
}

void VulkanDriver::sampler_destroy(GPU::SamplerID sampler)
{
    Sampler& sam = _get_sampler(sampler);
    LogicalDevice& ld = _get_logical_device(sam.device);

    ld.vk.vkDestroySampler(sam.vk_device, sam.vk_sampler, Vulkan::allocation_callbacks());

    data.samplers.remove(sampler);
}

GPU::TextureID VulkanDriver::texture_create(const GPU::TextureCreateInfo& ci)
{
    GPU::TextureID texture_id = data.textures.add(Texture());
    Texture& tex = _get_texture(texture_id);
    LogicalDevice& ld = _get_logical_device(ci.device);
    tex.vk_device = ld.vk_device;
    tex.vk_format = VkUtils::_vk_get_texture_format(ci.format);
    tex.format = ci.format;
    tex.extent = ci.extent;
    tex.device = ci.device;
    tex.texture = texture_id;

    VkImageCreateInfo vk_image_info =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VkUtils::_vk_get_image_type(ci.type),
        .format = VkUtils::_vk_get_texture_format(ci.format),
        .extent =
        {
            .width = ci.extent.x,
            .height = ci.extent.y,
            .depth = ci.extent.z,
        },
        .mipLevels = ci.mip_levels,
        .arrayLayers = ci.array_levels,
        .samples = VkUtils::_vk_get_samples(ci.sample_count),
        .tiling = VkUtils::_vk_get_tiling(ci.tiling),
        .usage = VkUtils::_vk_get_texture_usage(ci.usage),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VkUtils::_vk_get_image_layout(ci.initial_layout),
    };
    
    VkResult result = ld.vk.vkCreateImage(ld.vk_device, &vk_image_info, Vulkan::allocation_callbacks(), &tex.vk_image);
    VKFailOn(result != VK_SUCCESS, "vkCreateImage({})", Vulkan::result_as_string(result));

    MemoryHeap& heap = _get_memory_heap(ci.memory_heap);

    VkBindImageMemoryInfo vk_bind_info =
    {
        .sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO,
        .pNext = nullptr,
        .image = tex.vk_image,
        .memory = heap.vk_memory,
        .memoryOffset = ci.heap_offset,
    };
    
    ld.vk.vkBindImageMemory2(ld.vk_device, 1, &vk_bind_info);

    // The view needs the memory first
    VkImageViewCreateInfo vk_view_info =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = tex.vk_image,
        .viewType = VkUtils::_vk_get_image_view_type(ci.type),
        .format = VkUtils::_vk_get_texture_format(ci.format),
        .components = {},
        .subresourceRange =
        {
            // TODO: check if format is depth or stencil
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = ci.mip_levels,
            .baseArrayLayer = 0,
            .layerCount = ci.array_levels,
        },
    };

    result = ld.vk.vkCreateImageView(ld.vk_device, &vk_view_info, Vulkan::allocation_callbacks(), &tex.vk_image_view);
    VKFailOn(result != VK_SUCCESS, "vkCreateImageView({})", Vulkan::result_as_string(result));

    return texture_id;
}

void VulkanDriver::texture_destroy(GPU::TextureID texture)
{
    Texture& tex = _get_texture(texture);
    LogicalDevice& ld = _get_logical_device(tex.device);

    ld.vk.vkDestroyImage(tex.vk_device, tex.vk_image, Vulkan::allocation_callbacks());
    ld.vk.vkDestroyImageView(tex.vk_device, tex.vk_image_view, Vulkan::allocation_callbacks());

    data.textures.remove(texture);
}

GPU::DescriptorSetLayoutID VulkanDriver::descriptor_set_layout_create(const GPU::DescriptorSetLayoutCreateInfo& ci)
{
    GPU::DescriptorSetLayoutID descriptor_set_layout_id = data.descriptor_set_layouts.add(DescriptorSetLayout());
    DescriptorSetLayout& layout = _get_descriptor_set_layout(descriptor_set_layout_id);
    LogicalDevice& ld = _get_logical_device(ci.device);
    mem::Allocator allocator = acquire_tmp_allocator();

    layout.vk_device = ld.vk_device;
    layout.device = ci.device;
    layout.descriptor_set_layout = descriptor_set_layout_id;

    Slice<VkDescriptorSetLayoutBinding> vk_bindings = allocator.array<VkDescriptorSetLayoutBinding>(ci.bindings.len);
    for(usize i = 0; i < ci.bindings.len; i++)
    {
        vk_bindings[i] =
        {
            .binding = ci.bindings[i].binding,
            .descriptorType = VkUtils::_vk_get_descriptor_type(ci.bindings[i].type),
            .descriptorCount = ci.bindings[i].count,
            .stageFlags = VkUtils::_vk_get_shader_stage(ci.bindings[i].stages),
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

    VkResult result = ld.vk.vkCreateDescriptorSetLayout(ld.vk_device, &vk_set_layout_info, Vulkan::allocation_callbacks(), &layout.vk_set_layout);
    VKFailOn(result != VK_SUCCESS, "vkCreateDescriptorSetLayout({})", Vulkan::result_as_string(result));
 
    return descriptor_set_layout_id;
}

void VulkanDriver::descriptor_set_layout_destroy(GPU::DescriptorSetLayoutID descriptor_set_layout)
{
    DescriptorSetLayout& layout = _get_descriptor_set_layout(descriptor_set_layout);
    LogicalDevice& ld = _get_logical_device(layout.device);

    ld.vk.vkDestroyDescriptorSetLayout(layout.vk_device, layout.vk_set_layout, Vulkan::allocation_callbacks());

    data.descriptor_set_layouts.remove(descriptor_set_layout);
}

GPU::DescriptorPoolID VulkanDriver::descriptor_pool_create(const GPU::DescriptorPoolCreateInfo& ci)
{
    GPU::DescriptorPoolID descriptor_pool_id = data.descriptor_pools.add(DescriptorPool());
    DescriptorPool& descriptor_pool = data.descriptor_pools.get(descriptor_pool_id);
    LogicalDevice& ld = _get_logical_device(ci.device);
    mem::Allocator allocator = acquire_tmp_allocator();

    descriptor_pool.vk_device = ld.vk_device;
    descriptor_pool.device = ci.device;
    descriptor_pool.descriptor_pool = descriptor_pool_id;

    Slice<VkDescriptorPoolSize> vk_pool_sizes = allocator.array<VkDescriptorPoolSize>(ci.sizes.len);
    for(usize i = 0; i < vk_pool_sizes.len; i++)
    {
        vk_pool_sizes[i] =
        {
            .type = VkUtils::_vk_get_descriptor_type(ci.sizes[i].type),
            .descriptorCount = ci.sizes[i].count,
        };
    }

    VkDescriptorPoolCreateInfo vk_global_pool_info = 
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = ci.max_sets,
        .poolSizeCount = static_cast<uint32_t>(vk_pool_sizes.len),
        .pPoolSizes = vk_pool_sizes.ptr(),
    };

    VkResult result = ld.vk.vkCreateDescriptorPool(ld.vk_device, &vk_global_pool_info, Vulkan::allocation_callbacks(), &descriptor_pool.vk_descriptor_pool);
    VKFailOn(result != VK_SUCCESS, "vkCreateDescriptorPool({})", Vulkan::result_as_string(result));

    return descriptor_pool_id;
}

void VulkanDriver::descriptor_pool_destroy(GPU::DescriptorPoolID descriptor_pool)
{
    DescriptorPool& pool = data.descriptor_pools.get(descriptor_pool);
    LogicalDevice& ld = _get_logical_device(pool.device);

    ld.vk.vkDestroyDescriptorPool(pool.vk_device, pool.vk_descriptor_pool, Vulkan::allocation_callbacks());

    data.descriptor_pools.remove(descriptor_pool);
}

GPU::DescriptorSetID VulkanDriver::descriptor_set_allocate(const GPU::DescriptorSetAllocateInfo& ci)
{
    GPU::DescriptorSetID descriptor_set_id = data.descriptor_sets.add(DescriptorSet());
    DescriptorSet& set = _get_descriptor_set(descriptor_set_id);
    DescriptorPool& pool = _get_descriptor_pool(ci.pool);
    DescriptorSetLayout& layout = _get_descriptor_set_layout(ci.set_layout);
    LogicalDevice& ld = _get_logical_device(ci.device);

    set.vk_device = ld.vk_device;
    set.vk_descriptor_pool = pool.vk_descriptor_pool;
    set.device = ci.device;
    set.descriptor_set = descriptor_set_id;
    set.descriptor_pool = ci.pool;

    VkDescriptorSetAllocateInfo vk_allocate_set_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = pool.vk_descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &layout.vk_set_layout,
    };

    VkResult result = ld.vk.vkAllocateDescriptorSets(ld.vk_device, &vk_allocate_set_info, &set.vk_descriptor_set);
    VKFailOn(result != VK_SUCCESS, "vkAllocateDescriptorSets({})", Vulkan::result_as_string(result));

    return descriptor_set_id;
}

void VulkanDriver::descriptor_set_free(GPU::DescriptorSetID descriptor_set)
{
    DescriptorSet& set = _get_descriptor_set(descriptor_set);
    LogicalDevice& ld = _get_logical_device(set.device);

    VkResult result = ld.vk.vkFreeDescriptorSets(set.vk_device, set.vk_descriptor_pool, 1, &set.vk_descriptor_set);
    VKFailOn(result != VK_SUCCESS, "vkFreeDescriptorSets({})", Vulkan::result_as_string(result));

    data.descriptor_sets.remove(descriptor_set);
}

void VulkanDriver::descriptor_set_update_descriptors(GPU::DescriptorSetID descriptor_set, const GPU::UpdateDescriptorInfo& update_info)
{
    DescriptorSet& set = _get_descriptor_set(descriptor_set);
    LogicalDevice& ld = _get_logical_device(set.device);
    mem::Allocator allocator = acquire_tmp_allocator();

    Slice<VkWriteDescriptorSet> vk_write_descriptor = allocator.array<VkWriteDescriptorSet>(update_info.write_infos.len);

    usize total_buffer_infos = 0;
    usize total_image_infos = 0;
    for(usize i = 0; i < update_info.write_infos.len; i++)
    {
        if(update_info.write_infos[i].type == GPU::DescriptorType::UniformBuffer
            || update_info.write_infos[i].type == GPU::DescriptorType::StorageBuffer)
        {
            total_buffer_infos += update_info.write_infos[i].count;
        }
        else if(update_info.write_infos[i].type == GPU::DescriptorType::CombinedTextureSampler)
        {
            total_image_infos += update_info.write_infos[i].count;
        }
    }

    Slice<VkDescriptorBufferInfo> vk_buffer_infos = allocator.array<VkDescriptorBufferInfo>(total_buffer_infos);
    Slice<VkDescriptorImageInfo> vk_image_infos = allocator.array<VkDescriptorImageInfo>(total_image_infos);
    usize vk_buffer_infos_index = 0;
    usize vk_image_infos_index = 0;

    for(usize i = 0; i < update_info.write_infos.len; i++)
    {
        const GPU::WriteDescriptorInfo& write_info = update_info.write_infos[i];
        vk_write_descriptor[i] =
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = set.vk_descriptor_set,
            .dstBinding = write_info.binding,
            .dstArrayElement = write_info.array_element,
            .descriptorCount = write_info.count,
            .descriptorType = VkUtils::_vk_get_descriptor_type(write_info.type),
            .pImageInfo = nullptr,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr,
        };

        switch(update_info.write_infos[i].type)
        {
        case GPU::DescriptorType::UniformBuffer:
        case GPU::DescriptorType::StorageBuffer:
        {
            vk_write_descriptor[i].pBufferInfo = &vk_buffer_infos[vk_buffer_infos_index];
            for(usize buffer_i = 0; buffer_i < write_info.buffers.len; buffer_i++)
            {
                const GPU::DescriptorBufferInfo& buffer_info = write_info.buffers[buffer_i];

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
        case GPU::DescriptorType::CombinedTextureSampler:
        {
            vk_write_descriptor[i].pImageInfo = &vk_image_infos[vk_image_infos_index];
            for(usize texture_i = 0; texture_i < write_info.textures.len; texture_i++)
            {
                const GPU::DescriptorTextureInfo& texture_info = write_info.textures[texture_i];

                vk_image_infos[vk_image_infos_index] =
                {
                    .sampler = _get_sampler(texture_info.sampler).vk_sampler,
                    .imageView = _get_texture(texture_info.texture).vk_image_view,
                    .imageLayout = VkUtils::_vk_get_image_layout(texture_info.layout),
                };
                vk_image_infos_index++;
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
}

GPU::PipelineID VulkanDriver::pipeline_create(const GPU::PipelineCreateInfo& ci)
{
    LogicalDevice& ld = _get_logical_device(ci.device);
    mem::Allocator allocator = acquire_tmp_allocator();

    GPU::PipelineID pipeline_id = data.pipelines.add(Pipeline());
    Pipeline& pipe = _get_pipeline(pipeline_id);
    pipe.vk_device = ld.vk_device;
    pipe.device = ci.device;
    pipe.pipeline = pipeline_id;

    Slice<VkPipelineShaderStageCreateInfo> vk_shader_stages = allocator.array<VkPipelineShaderStageCreateInfo>(ci.shader_stages.len);

    for(usize i = 0; i < ci.shader_stages.len; i++)
    {
        Slice<char> null_terminated = allocator.array<char>(ci.shader_stages[i].name.len + 1);
        mem::copy(null_terminated, ci.shader_stages[i].name);
        null_terminated[null_terminated.len - 1] = '\0';

        vk_shader_stages[i] =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VkShaderStageFlagBits(VkUtils::_vk_get_shader_stage(ci.shader_stages[i].stage)),
            .module = _vk_create_shader_module(ld, ci.shader_stages[i]),
            .pName = null_terminated.ptr(),
            .pSpecializationInfo = nullptr,
        };
    }

    Slice<VkVertexInputBindingDescription> vk_vertex_bindings = allocator.array<VkVertexInputBindingDescription>(ci.vertex_input.bindings.len);
    Slice<VkVertexInputAttributeDescription> vk_vertex_attributes = allocator.array<VkVertexInputAttributeDescription>(ci.vertex_input.attributes.len);
    
    for(usize i = 0; i < ci.vertex_input.bindings.len; i++)
    {
        vk_vertex_bindings[i] =
        {
            .binding = ci.vertex_input.bindings[i].binding,
            .stride = ci.vertex_input.bindings[i].stride,
            .inputRate = VkUtils::_vk_get_input_rate(ci.vertex_input.bindings[i].input_rate),
        };
    }

    for(usize i = 0; i < ci.vertex_input.attributes.len; i++)
    {
        vk_vertex_attributes[i] =
        {
            .location = ci.vertex_input.attributes[i].location,
            .binding = ci.vertex_input.attributes[i].binding,
            .format = VkUtils::_vk_get_vertex_format(ci.vertex_input.attributes[i].format),
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
        .topology = VkUtils::_vk_get_topology(ci.input_assembly.topology),
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
        .polygonMode = VkUtils::_vk_get_polygon_mode(ci.rasterizer_state.polygon_mode),
        .cullMode = VkUtils::_vk_get_cull_mode(ci.rasterizer_state.cull_mode),
        .frontFace = VkUtils::_vk_get_front_face(ci.rasterizer_state.front_face),
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0,
        .depthBiasClamp = 0,
        .depthBiasSlopeFactor = 1.F,
        .lineWidth = ci.rasterizer_state.line_width,
    };

    VkPipelineMultisampleStateCreateInfo vk_multisample_state =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VkUtils::_vk_get_samples(ci.multisample_state.sample_count),
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

    // Creating the layout

    Slice<VkPushConstantRange> vk_push_ranges = allocator.array<VkPushConstantRange>(ci.pipeline_layout.constant_blocks.len);
    for(usize i = 0; i < ci.pipeline_layout.constant_blocks.len; i++)
    {
        vk_push_ranges[i] =
        {
            .stageFlags = VkUtils::_vk_get_shader_stage(ci.pipeline_layout.constant_blocks[i].stages),
            .offset = ci.pipeline_layout.constant_blocks[i].offset,
            .size = ci.pipeline_layout.constant_blocks[i].size,
        };
    }

    Slice<VkDescriptorSetLayout> vk_set_layouts = allocator.array<VkDescriptorSetLayout>(ci.pipeline_layout.set_layouts.len);
    for(usize i = 0; i < ci.pipeline_layout.set_layouts.len; i++)
    {
        vk_set_layouts[i] = _get_descriptor_set_layout(ci.pipeline_layout.set_layouts[i]).vk_set_layout;
    }

    VkPipelineLayoutCreateInfo vk_pipeline_layout_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = static_cast<uint32_t>(vk_set_layouts.len),
        .pSetLayouts = vk_set_layouts.ptr(),
        .pushConstantRangeCount = static_cast<uint32_t>(vk_push_ranges.len),
        .pPushConstantRanges = vk_push_ranges.ptr(),
    };

    VkResult result = ld.vk.vkCreatePipelineLayout(ld.vk_device, &vk_pipeline_layout_info, Vulkan::allocation_callbacks(), &pipe.vk_pipeline_layout);
    VKFailOn(result != VK_SUCCESS, "vkCreatePipelineLayout({})", Vulkan::result_as_string(result));

    Slice<VkFormat> vk_color_attachment_formats = allocator.array<VkFormat>(ci.rendering_info.render_attachments.len);
    for(usize i = 0; i < vk_color_attachment_formats.len; i++)
    {
        vk_color_attachment_formats[i] = VkUtils::_vk_get_texture_format(ci.rendering_info.render_attachments[i]);
    }

    VkPipelineRenderingCreateInfoKHR vk_pipeline_rendering_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .pNext = nullptr,
        .viewMask = 0,
        .colorAttachmentCount = static_cast<uint32_t>(vk_color_attachment_formats.len),
        .pColorAttachmentFormats = vk_color_attachment_formats.ptr(),
        .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
    };

    RenderPassCache* render_pass = nullptr;
    if(!ld.additional_extension_support.has_dynamic_rendering)
    {
        render_pass = &_get_render_pass_for_pipeline(ld, ci.rendering_info);
    }
    
    VkGraphicsPipelineCreateInfo vk_graphics_pipeline_info =
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = ld.additional_extension_support.has_dynamic_rendering ? &vk_pipeline_rendering_info : nullptr,
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
        .renderPass = ld.additional_extension_support.has_dynamic_rendering ? nullptr : render_pass->vk_render_pass,
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
    return pipeline_id;
}

void VulkanDriver::pipeline_destroy(GPU::PipelineID pipeline)
{
    Pipeline& pipe = _get_pipeline(pipeline);
    LogicalDevice& ld = _get_logical_device(pipe.device);

    ld.vk.vkDestroyPipeline(pipe.vk_device, pipe.vk_pipeline, Vulkan::allocation_callbacks());
    ld.vk.vkDestroyPipelineLayout(pipe.vk_device, pipe.vk_pipeline_layout, Vulkan::allocation_callbacks()); 

    data.pipelines.remove(pipeline);
}

GPU::CommandPoolID VulkanDriver::command_pool_create(const GPU::CommandPoolCreateInfo& ci)
{
    LogicalDevice& ld = _get_logical_device(ci.device);
    Queue& queue = _get_queue(ci.queue);
    
    GPU::CommandPoolID cmd_pool_id = data.command_pools.add(CommandPool());
    CommandPool& cmd_pool = _get_command_pool(cmd_pool_id);

    LogicalDevice::QueueFamily& family = ld.families[ld.device_queues[queue.device_queue_index].family_index];
    VkCommandPoolCreateInfo cmd_pool_info =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = family.vk_family_index,
    };

    VkResult result = ld.vk.vkCreateCommandPool(ld.vk_device, &cmd_pool_info, Vulkan::allocation_callbacks(), &cmd_pool.vk_command_pool);
    VKFailOn(result != VK_SUCCESS, "vkCreateCommandPool({})", Vulkan::result_as_string(result));

    cmd_pool.vk_device = ld.vk_device;
    cmd_pool.device = ci.device;

    return cmd_pool_id;
}

void VulkanDriver::command_pool_destroy(GPU::CommandPoolID command_pool)
{
    CommandPool& cmd_pool = _get_command_pool(command_pool);
    LogicalDevice& ld = _get_logical_device(cmd_pool.device);

    ld.vk.vkDestroyCommandPool(cmd_pool.vk_device, cmd_pool.vk_command_pool, Vulkan::allocation_callbacks());

    data.command_pools.remove(command_pool);
}

GPU::CommandBufferID VulkanDriver::command_buffer_allocate(const GPU::CommandBufferAllocateInfo& ci)
{
    CommandPool& cmd_pool = _get_command_pool(ci.pool);
    LogicalDevice& ld = _get_logical_device(cmd_pool.device);

    GPU::CommandBufferID cmd_buffer_id = data.command_buffers.add(CommandBuffer());
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

void VulkanDriver::command_buffer_free(GPU::CommandBufferID command_buffer)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    ld.vk.vkFreeCommandBuffers(ld.vk_device, cmd_buffer.vk_command_pool, 1, &cmd_buffer.vk_command_buffer);
    
    data.command_buffers.remove(command_buffer);
}

void VulkanDriver::command_buffer_begin(GPU::CommandBufferID command_buffer)
{
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

void VulkanDriver::command_buffer_end(GPU::CommandBufferID command_buffer)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    VkResult result = ld.vk.vkEndCommandBuffer(cmd_buffer.vk_command_buffer);
    VKFailOn(result != VK_SUCCESS, "vkEndCommandBuffer({})", Vulkan::result_as_string(result));
}

void VulkanDriver::command_buffer_begin_renderpass(GPU::CommandBufferID command_buffer, const GPU::RenderPassBeginInfo& begin_info)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    Texture& tex = _get_texture(begin_info.render_attachment.image);

    VkClearValue vk_clear_value = {};
    vk_clear_value.color.float32[0] = begin_info.render_attachment.clear_color.r;
    vk_clear_value.color.float32[1] = begin_info.render_attachment.clear_color.g;
    vk_clear_value.color.float32[2] = begin_info.render_attachment.clear_color.b;
    vk_clear_value.color.float32[3] = begin_info.render_attachment.clear_color.a;

    VkRect2D vk_render_area =
    {
        .offset =
        {
            .x = begin_info.offset.x,
            .y = begin_info.offset.y,
        },
        .extent = 
        {
            .width = begin_info.extent.width,
            .height = begin_info.extent.height
        },
    };

    if(ld.additional_extension_support.has_dynamic_rendering)
    {
        VkImageView vk_resolve_view = VK_NULL_HANDLE;
        VkImageLayout vk_resolve_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        if(begin_info.render_attachment.resolve_image.is_valid())
        {
            vk_resolve_view = _get_texture(begin_info.render_attachment.resolve_image).vk_image_view;
            vk_resolve_layout = VkUtils::_vk_get_image_layout(begin_info.render_attachment.resolve_layout);
        }

        VkRenderingAttachmentInfoKHR vk_color_attachment =
        {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .pNext = nullptr,
            .imageView = tex.vk_image_view,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = vk_resolve_view,
            .resolveImageLayout = vk_resolve_layout,
            .loadOp = VkUtils::_vk_get_load_op(begin_info.render_attachment.load_op),
            .storeOp = VkUtils::_vk_get_store_op(begin_info.render_attachment.store_op),
            .clearValue = vk_clear_value,
        };

        VkRenderingInfoKHR vk_rendering_info =
        {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .renderArea = vk_render_area,
            .layerCount = 1,
            .viewMask = 0,
            .colorAttachmentCount = 1,
            .pColorAttachments = &vk_color_attachment,
            .pDepthAttachment = nullptr,
            .pStencilAttachment = nullptr,
        };

        ld.vk.vkCmdBeginRenderingKHR(cmd_buffer.vk_command_buffer, &vk_rendering_info);
        return;
    }

    VkRenderPass vk_render_pass;
    VkFramebuffer vk_framebuffer;
    _get_render_pass_and_framebuffer_for(ld, tex, begin_info, &vk_render_pass, &vk_framebuffer);

    VkRenderPassBeginInfo vk_begin_info =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = vk_render_pass,
        .framebuffer = vk_framebuffer,
        .renderArea = vk_render_area,
        .clearValueCount = 1,
        .pClearValues = &vk_clear_value,
    };

    ld.vk.vkCmdBeginRenderPass(cmd_buffer.vk_command_buffer, &vk_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanDriver::command_buffer_end_renderpass(GPU::CommandBufferID command_buffer, const GPU::RenderPassEndInfo& end_info)
{
    Unused(end_info);

    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    if(ld.additional_extension_support.has_dynamic_rendering)
    {
        ld.vk.vkCmdEndRenderingKHR(cmd_buffer.vk_command_buffer);
        return;
    }

    ld.vk.vkCmdEndRenderPass(cmd_buffer.vk_command_buffer);
}

void VulkanDriver::command_buffer_memory_barrier(GPU::CommandBufferID command_buffer, const GPU::PipelineMemoryBarrier& memory_barrier)
{
    Unused(command_buffer, memory_barrier);
}

void VulkanDriver::command_buffer_buffer_barrier(GPU::CommandBufferID command_buffer, const GPU::PipelineBufferBarrier& buffer_barrier)
{
    Unused(command_buffer, buffer_barrier);
}

void VulkanDriver::command_buffer_texture_barrier(GPU::CommandBufferID command_buffer, const GPU::PipelineTextureBarrier& texture_barrier)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    VkPipelineStageFlags vk_src_stages = VkUtils::_vk_get_pipeline_stages(texture_barrier.src_stages);
    VkPipelineStageFlags vk_dest_stages = VkUtils::_vk_get_pipeline_stages(texture_barrier.dest_stages);

    VkImageSubresourceRange vk_subresource_range =
    {
        .aspectMask = VkUtils::_vk_get_aspect_masks(texture_barrier.subresource_range.aspect),
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
        .srcAccessMask = VkUtils::_vk_get_access_masks(texture_barrier.src_masks),
        .dstAccessMask = VkUtils::_vk_get_access_masks(texture_barrier.dest_masks),
        .oldLayout = VkUtils::_vk_get_image_layout(texture_barrier.src_layout),
        .newLayout = VkUtils::_vk_get_image_layout(texture_barrier.dest_layout),
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

void VulkanDriver::command_buffer_copy_buffer_to_texture(GPU::CommandBufferID command_buffer, const GPU::CopyBufferToTextureInfo& copy_info)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    Buffer& src_buffer = _get_buffer(copy_info.source_buffer);
    Texture& dest_texture = _get_texture(copy_info.destination_texture);

    VkImageSubresourceLayers vk_subresource_layer =
    {
        .aspectMask = VkUtils::_vk_get_aspect_masks(copy_info.subresource_layer.aspect),
        .mipLevel = copy_info.subresource_layer.mip_level,
        .baseArrayLayer = copy_info.subresource_layer.base_array_layer,
        .layerCount = copy_info.subresource_layer.layer_count,
    };

    VkBufferImageCopy vk_buffer_image_copy =
    {
        .bufferOffset = copy_info.source_offset,
        .bufferRowLength = copy_info.row_length,
        .bufferImageHeight = copy_info.image_height,
        .imageSubresource = vk_subresource_layer,
        .imageOffset =
        {
            .x = copy_info.offset.x,
            .y = copy_info.offset.y,
            .z = copy_info.offset.z,
        },
        .imageExtent =
        {
            .width = copy_info.extent.x,
            .height = copy_info.extent.y,
            .depth = copy_info.extent.z,
        },
    };

    ld.vk.vkCmdCopyBufferToImage(
        cmd_buffer.vk_command_buffer, src_buffer.vk_buffer, dest_texture.vk_image,
        VkUtils::_vk_get_image_layout(copy_info.destination_layout), 1, &vk_buffer_image_copy
    );
}
    
void VulkanDriver::command_buffer_copy_buffer(GPU::CommandBufferID command_buffer, const GPU::BufferCopyInfo& copy_info)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    mem::Allocator allocator = acquire_tmp_allocator();

    Buffer& src_buffer = _get_buffer(copy_info.source_buffer);
    Buffer& dest_buffer = _get_buffer(copy_info.destination_buffer);

    Slice<VkBufferCopy> vk_regions = allocator.array<VkBufferCopy>(copy_info.copy_regions.len);
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
}

void VulkanDriver::command_buffer_bind_pipeline(GPU::CommandBufferID command_buffer, GPU::PipelineBindPoint bind_point, GPU::PipelineID pipeline)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    Pipeline& pipe = _get_pipeline(pipeline);

    ld.vk.vkCmdBindPipeline(cmd_buffer.vk_command_buffer, VkUtils::_vk_get_bind_point(bind_point), pipe.vk_pipeline);
}

void VulkanDriver::command_buffer_bind_descriptor_sets(GPU::CommandBufferID command_buffer, GPU::PipelineBindPoint bind_point, GPU::PipelineID pipeline, u32 base_set, const Slice<GPU::DescriptorSetID>& descriptor_sets)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    Pipeline& pipe = _get_pipeline(pipeline);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    mem::Allocator allocator = acquire_tmp_allocator();

    Slice<VkDescriptorSet> vk_descriptor_sets = allocator.array<VkDescriptorSet>(descriptor_sets.len);
    for(usize i = 0; i < descriptor_sets.len; i++)
    {
        vk_descriptor_sets[i] = _get_descriptor_set(descriptor_sets[i]).vk_descriptor_set;
    }

    ld.vk.vkCmdBindDescriptorSets(
        cmd_buffer.vk_command_buffer, VkUtils::_vk_get_bind_point(bind_point), pipe.vk_pipeline_layout,
        base_set, static_cast<uint32_t>(vk_descriptor_sets.len), vk_descriptor_sets.ptr(), 0, nullptr 
    );
}

void VulkanDriver::command_buffer_bind_vertex_buffers(GPU::CommandBufferID command_buffer, u32 base_binding, const Slice<GPU::BufferID>& buffers, const Slice<usize>& offsets)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    mem::Allocator allocator = acquire_tmp_allocator();

    Slice<VkBuffer> vk_buffers = allocator.array<VkBuffer>(buffers.len);
    for(usize i = 0; i < buffers.len; i++)
    {
        vk_buffers[i] = _get_buffer(buffers[i]).vk_buffer;
    }

    ld.vk.vkCmdBindVertexBuffers(
        cmd_buffer.vk_command_buffer, base_binding,
        static_cast<uint32_t>(vk_buffers.len), vk_buffers.ptr(), reinterpret_cast<const VkDeviceSize*>(offsets.ptr())
    );

    allocator.free(mem::to_bytes(vk_buffers));
}

void VulkanDriver::command_buffer_constant_block(GPU::CommandBufferID command_buffer, GPU::PipelineID pipeline, GPU::ShaderStage stages, u32 offset, u32 size, MemoryAddress block_address)
{   
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    Pipeline& pipe = _get_pipeline(pipeline);

    ld.vk.vkCmdPushConstants(
        cmd_buffer.vk_command_buffer, pipe.vk_pipeline_layout, VkUtils::_vk_get_shader_stage(stages),
        offset, size, reinterpret_cast<void*>(block_address)
    );
}

void VulkanDriver::command_buffer_set_viewports(GPU::CommandBufferID command_buffer, u32 base_viewport, const Slice<GPU::Viewport>& viewports)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    mem::Allocator allocator = acquire_tmp_allocator();

    Slice<VkViewport> vk_viewports = allocator.array<VkViewport>(viewports.len);
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
}

void VulkanDriver::command_buffer_set_scissors(GPU::CommandBufferID command_buffer, u32 base_scissor, const Slice<GPU::Scissor>& scissors)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    mem::Allocator allocator = acquire_tmp_allocator();

    Slice<VkRect2D> vk_scissors = allocator.array<VkRect2D>(scissors.len);
    for(usize i = 0; i < scissors.len; i++)
    {
        vk_scissors[i] =
        {
            .offset = { .x = scissors[i].x, .y = scissors[i].y },
            .extent = { .width = scissors[i].width, .height = scissors[i].height },
        };
    }

    ld.vk.vkCmdSetScissor(cmd_buffer.vk_command_buffer, base_scissor, static_cast<uint32_t>(vk_scissors.len), vk_scissors.ptr());
}

void VulkanDriver::command_buffer_draw(GPU::CommandBufferID command_buffer, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance)
{
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

    mem::Allocator allocator = acquire_tmp_allocator();

    data.physical_devices = get_allocator().array<PhysicalDevice>(physical_device_count);
    Slice<VkPhysicalDevice> vk_physical_devices = allocator.array<VkPhysicalDevice>(physical_device_count);
    vk.vkEnumeratePhysicalDevices(data.instance, &physical_device_count, vk_physical_devices.ptr());

    data.physical_device_ids = get_allocator().array<GPU::PhysicalDeviceID>(data.physical_devices.len);
    for(usize i = 0; i < data.physical_device_ids.len; i++)
    {
        data.physical_device_ids[i] = GPU::PhysicalDeviceID(static_cast<u32>(i));
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
}

void VulkanDriver::_vk_get_surface_format(GPU::TextureFormat surface_format, VkFormat* vk_image_format, VkColorSpaceKHR* vk_color_space)
{
    switch(surface_format)
    {
    case GPU::TextureFormat::RGBA8Unorm:
        *vk_image_format = VK_FORMAT_R8G8B8A8_UNORM;
        *vk_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        return;
    case GPU::TextureFormat::RGBA8Srgb:
        *vk_image_format = VK_FORMAT_R8G8B8A8_SRGB;
        *vk_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        return;
    case GPU::TextureFormat::BGRA8Unorm:
        *vk_image_format = VK_FORMAT_B8G8R8A8_UNORM;
        *vk_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        return;
    case GPU::TextureFormat::BGRA8Srgb:
        *vk_image_format = VK_FORMAT_B8G8R8A8_SRGB;
        *vk_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        return;
    default:
        break;
    }

    VKFailOn(true, "invalid surface format");
}

VkSurfaceCapabilitiesKHR VulkanDriver::_vk_get_surface_capabilities(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface)
{
    VkSurfaceCapabilitiesKHR capabilities;
    vk.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, vk_surface, &capabilities);
    return capabilities;   
}

VkExtent2D VulkanDriver::_vk_get_swap_chain_extent(const Vector2U& size, const VkSurfaceCapabilitiesKHR& vk_capabilities)
{
    if(vk_capabilities.currentExtent.width != MaxValue<uint32_t>)
    {
        return vk_capabilities.currentExtent;
    }
    
    VkExtent2D vk_extent = {};
    vk_extent.width = math::clamp(size.width, vk_capabilities.minImageExtent.width, vk_capabilities.maxImageExtent.width);
    vk_extent.height = math::clamp(size.height, vk_capabilities.minImageExtent.height, vk_capabilities.maxImageExtent.height);
    return vk_extent;
}

VkShaderModule VulkanDriver::_vk_create_shader_module(LogicalDevice& ld, const GPU::ShaderStageInfo& shader_stage_info)
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

VulkanDriver::RenderPassCache& VulkanDriver::_get_render_pass_for(LogicalDevice& ld, Texture& texture, const GPU::RenderPassBeginInfo& begin_info)
{
    VkDriverRenderPassKey render_pass_key =
    {
        .format = texture.format,
        .load_op = begin_info.render_attachment.load_op,
        .store_op = begin_info.render_attachment.store_op,
    };

    if(ld.render_pass_cache.has(render_pass_key))
    {
        return ld.render_pass_cache.get(render_pass_key);
    }

    VkAttachmentDescription vk_attachment_info =
    {
        .flags = 0,
        .format = texture.vk_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VkUtils::_vk_get_load_op(begin_info.render_attachment.load_op),
        .storeOp = VkUtils::_vk_get_store_op(begin_info.render_attachment.store_op),
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference vk_color_attachment =
    {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription vk_subpass =
    {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &vk_color_attachment,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr,
    };

    VkRenderPassCreateInfo vk_render_pass_info =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = 1,
        .pAttachments = &vk_attachment_info,
        .subpassCount = 1,
        .pSubpasses = &vk_subpass,
        .dependencyCount = 0,
        .pDependencies = nullptr,
    };

    RenderPassCache& render_pass_cache = ld.render_pass_cache.insert(render_pass_key, RenderPassCache());
    render_pass_cache.vk_framebuffers_cache = HashMap<VkImageView, VkFramebuffer>::with_size(get_allocator(), 3);
    render_pass_cache.device = ld.device;
    ld.vk.vkCreateRenderPass(ld.vk_device, &vk_render_pass_info, Vulkan::allocation_callbacks(), &render_pass_cache.vk_render_pass);
    
    return render_pass_cache;
}

VulkanDriver::RenderPassCache& VulkanDriver::_get_render_pass_for_pipeline(LogicalDevice& ld,
    const GPU::RenderingInfo& pipeline_rendering_info)
{
    VkDriverRenderPassKey render_pass_key =
    {
        .format = pipeline_rendering_info.render_attachments[0],
        .load_op = GPU::LoadOp::Load,
        .store_op = GPU::StoreOp::Store,
    };
    if(ld.render_pass_cache.has(render_pass_key))
    {
        return ld.render_pass_cache.get(render_pass_key);
    }

    VkAttachmentDescription vk_attachment_info =
    {
        .flags = 0,
        .format = VkUtils::_vk_get_texture_format(pipeline_rendering_info.render_attachments[0]),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VkUtils::_vk_get_load_op(render_pass_key.load_op),
        .storeOp = VkUtils::_vk_get_store_op(render_pass_key.store_op),
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference vk_color_attachment =
    {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription vk_subpass =
    {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &vk_color_attachment,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr,
    };

    VkRenderPassCreateInfo vk_render_pass_info =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = 1,
        .pAttachments = &vk_attachment_info,
        .subpassCount = 1,
        .pSubpasses = &vk_subpass,
        .dependencyCount = 0,
        .pDependencies = nullptr,
    };

    RenderPassCache& render_pass_cache = ld.render_pass_cache.insert(render_pass_key, RenderPassCache());
    render_pass_cache.vk_framebuffers_cache = HashMap<VkImageView, VkFramebuffer>::with_size(get_allocator(), 3);
    render_pass_cache.device = ld.device;
    ld.vk.vkCreateRenderPass(ld.vk_device, &vk_render_pass_info, Vulkan::allocation_callbacks(), &render_pass_cache.vk_render_pass);
    
    return render_pass_cache;
}

void VulkanDriver::_get_render_pass_and_framebuffer_for(LogicalDevice& ld, Texture& texture, const GPU::RenderPassBeginInfo& begin_info,
    VkRenderPass* vk_render_pass, VkFramebuffer* vk_framebuffer)
{
    RenderPassCache& render_pass_cache = _get_render_pass_for(ld, texture, begin_info);

    if(render_pass_cache.vk_framebuffers_cache.has(texture.vk_image_view))
    {
        *vk_framebuffer = render_pass_cache.vk_framebuffers_cache.get(texture.vk_image_view);
    }
    else
    {
        VkFramebufferCreateInfo vk_framebuffer_info =
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = render_pass_cache.vk_render_pass,
            .attachmentCount = 1,
            .pAttachments = &texture.vk_image_view,
            .width = texture.extent.x,
            .height = texture.extent.y,
            .layers = texture.extent.z,
        };

        ld.vk.vkCreateFramebuffer(ld.vk_device, &vk_framebuffer_info, Vulkan::allocation_callbacks(), vk_framebuffer);
        render_pass_cache.vk_framebuffers_cache.insert(texture.vk_image_view, *vk_framebuffer);
    }

    *vk_render_pass = render_pass_cache.vk_render_pass;
}

GPU::DeviceType VulkanDriver::_vk_device_type_to_device_type(VkPhysicalDeviceType vk_device_type)
{
    switch(vk_device_type)
    {
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return GPU::DeviceType::IntegratedGPU;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return GPU::DeviceType::DiscreteGPU;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return GPU::DeviceType::Cpu;
    default:
        break;
    }

    VKFailOn(true, "invalid vulkan physical device type");
    return GPU::DeviceType::Unknown;
}

GPU::PresentMode VulkanDriver::_vk_present_mode_to_present_mode(VkPresentModeKHR vk_present_mode)
{
    switch(vk_present_mode)
    {
    case VK_PRESENT_MODE_IMMEDIATE_KHR:
        return GPU::PresentMode::Immediate;
    case VK_PRESENT_MODE_FIFO_KHR:
        return GPU::PresentMode::VSync;
    default:
        break;
    }

    VKFailOn(true, "invalid vulkan present mode");
    return GPU::PresentMode::Unknown;
}

