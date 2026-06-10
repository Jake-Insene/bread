#include "gpu/vk/vk_adapter.h"

#include "core/templates.h"
#include "display/display.h"
#include "gpu/gpu.h"
#include "gpu/vk/vk_utils.h"
#include "math/funcs.h"
#include "os/os.h"


VkDriverRenderPassKey VkDriverRenderPassKey::from_rendering_info(const GPU::RenderingInfo& rendering_info)
{
    VkDriverRenderPassKey key = {};

    for(usize i = 0; i < rendering_info.render_attachment_formats.len; i++)
    {
        key.render_attachments[i].attachment.format = static_cast<u32>(rendering_info.render_attachment_formats[i]);
        key.render_attachments[i].attachment.layout = 0;
        key.render_attachments[i].attachment.load_op = 0;
        key.render_attachments[i].attachment.store_op = 0;
    }
    if(rendering_info.depth_attachment_format != GPU::TextureFormat::Unknown)
    {
        key.depth_attachment.attachment.format = static_cast<u32>(rendering_info.depth_attachment_format);
        key.depth_attachment.attachment.layout = 0;
        key.depth_attachment.attachment.load_op = 0;
        key.depth_attachment.attachment.store_op = 0;
    }
    if(rendering_info.stencil_attachment_format != GPU::TextureFormat::Unknown)
    {
        key.stencil_attachment.attachment.format = static_cast<u32>(rendering_info.stencil_attachment_format);
        key.stencil_attachment.attachment.layout = 0;
        key.stencil_attachment.attachment.load_op = 0;
        key.stencil_attachment.attachment.store_op = 0;
    }

    return key;
}

VkDriverRenderPassKey VkDriverRenderPassKey::from_render_pass_begin_info(VulkanAdapter* adapter, const GPU::RenderPassBeginInfo& begin_info)
{
	VkDriverRenderPassKey key = {};

	for(usize i = 0; i < begin_info.render_attachments.len; i++)
	{
        key.render_attachments[i].attachment.format = static_cast<u32>(
            adapter->_get_texture_view(begin_info.render_attachments[i].texture_view).format
        );
        key.render_attachments[i].attachment.layout = static_cast<u32>(begin_info.render_attachments[i].layout);
        key.render_attachments[i].attachment.load_op = static_cast<u32>(begin_info.render_attachments[i].load_op);
        key.render_attachments[i].attachment.store_op = static_cast<u32>(begin_info.render_attachments[i].store_op);
	}

    if(begin_info.depth_attachment.texture_view.is_valid())
    {
        key.depth_attachment.attachment.format = static_cast<u32>(adapter->_get_texture_view(begin_info.depth_attachment.texture_view).format);
		key.depth_attachment.attachment.layout = static_cast<u32>(begin_info.depth_attachment.layout);
		key.depth_attachment.attachment.load_op = static_cast<u32>(begin_info.depth_attachment.load_op);
		key.depth_attachment.attachment.store_op = static_cast<u32>(begin_info.depth_attachment.store_op);
    }
    if(begin_info.stencil_attachment.texture_view.is_valid())
    {
        key.stencil_attachment.attachment.format = static_cast<u32>(adapter->_get_texture_view(begin_info.stencil_attachment.texture_view).format);
		key.stencil_attachment.attachment.layout = static_cast<u32>(begin_info.stencil_attachment.layout);
		key.stencil_attachment.attachment.load_op = static_cast<u32>(begin_info.stencil_attachment.load_op);
		key.stencil_attachment.attachment.store_op = static_cast<u32>(begin_info.stencil_attachment.store_op);
    }

	return key;
}

void VulkanAdapter::initialize(Mem::Allocator* _allocator)
{
    VKDebugInfo("Initializing Vulkan Driver...");
    
    internal_allocator = _allocator;

    tmp_allocator.init(OS::map_memory(1024*1024, OS::ReadWrite));

    surfaces = FreeList<Surface, GPU::SurfaceID>::with_allocator(get_allocator());
    devices = FreeList<LogicalDevice, GPU::DeviceID>::with_allocator(get_allocator());
    swap_chains = FreeList<SwapChain, GPU::SwapChainID>::with_allocator(get_allocator());
    fences = FreeList<Fence, GPU::FenceID>::with_allocator(get_allocator());
    semaphores = FreeList<Semaphore, GPU::SemaphoreID>::with_allocator(get_allocator());
    queues = Array<Queue>::with_allocator(get_allocator());
    memory_heaps = FreeList<MemoryHeap, GPU::MemoryHeapID>::with_allocator(get_allocator());
    buffers = FreeList<Buffer, GPU::BufferID>::with_allocator(get_allocator());
    samplers = FreeList<Sampler, GPU::SamplerID>::with_allocator(get_allocator());
    textures = FreeList<Texture, GPU::TextureID>::with_allocator(get_allocator());
    texture_views = FreeList<TextureView, GPU::TextureViewID>::with_allocator(get_allocator());
    descriptor_set_layouts = FreeList<DescriptorSetLayout, GPU::DescriptorSetLayoutID>::with_allocator(get_allocator());
    descriptor_pools = FreeList<DescriptorPool, GPU::DescriptorPoolID>::with_allocator(get_allocator());
    descriptor_sets = FreeList<DescriptorSet, GPU::DescriptorSetID>::with_allocator(get_allocator());
    pipeline_layouts = FreeList<PipelineLayout, GPU::PipelineLayoutID>::with_allocator(get_allocator());
    pipelines = FreeList<Pipeline, GPU::PipelineID>::with_allocator(get_allocator());
    command_pools = FreeList<CommandPool, GPU::CommandPoolID>::with_allocator(get_allocator());
    command_buffers = FreeList<CommandBuffer, GPU::CommandBufferID>::with_allocator(get_allocator());

#if defined(BREAD_ANDROID)
    vk_lib = OS::load_library("libvulkan.so");
#else
    vk_lib = OS::load_library("vulkan-1.dll");
#endif

    Vulkan::load_core_procs(vk_lib);

    info.api_version = Vulkan::get_api_version();
    VKFailOn(info.api_version < VK_API_VERSION_1_1, "vulkan 1.1 was expected");
    VKDebugInfo(
        "Vulkan API Version: {}.{}.{}",
        VK_API_VERSION_MAJOR(info.api_version),
        VK_API_VERSION_MINOR(info.api_version),
        VK_API_VERSION_PATCH(info.api_version)
    );

    instance = Vulkan::create_instance(this);
    Vulkan::load_instance_procs(instance);

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
            instance, &debug_messenger_info, Vulkan::allocation_callbacks(this), &messenger
        );
        VKFailOn(result != VK_SUCCESS, "vkCreateDebugUtilsMessengerEXT({})", Vulkan::result_as_string(result));
    }
#endif

    // Getting physical devices
    _get_physical_devices();

#if defined(BREAD_WIN32)
    dummy_surface = Vulkan::create_surface(this, instance, 0);
#elif defined(BREAD_ANDROID)
    dummy_surface = VK_NULL_HANDLE;
#endif
}

void VulkanAdapter::shutdown()
{
    vk.vkDestroySurfaceKHR(instance, dummy_surface, Vulkan::allocation_callbacks(this));

    surfaces.destroy();
    devices.destroy();
    swap_chains.destroy();
    fences.destroy();
    semaphores.destroy();
    queues.destroy();
    memory_heaps.destroy();
    buffers.destroy();
    samplers.destroy();
    textures.destroy();
    texture_views.destroy();
    descriptor_set_layouts.destroy();
    descriptor_pools.destroy();
    descriptor_sets.destroy();
    pipeline_layouts.destroy();
    pipelines.destroy();
    command_pools.destroy();
    command_buffers.destroy();

    get_allocator()->free(Mem::to_bytes(physical_device_ids));
    get_allocator()->free(Mem::to_bytes(physical_devices));

#if defined(BREAD_SHOW_DEBUG_INFO) && defined(BREAD_WIN32)
    vk.vkDestroyDebugUtilsMessengerEXT(
        instance, messenger, Vulkan::allocation_callbacks(this)
    );
#endif

    vk.vkDestroyInstance(instance, Vulkan::allocation_callbacks(this));

    OS::unload_library(vk_lib);

    OS::unmap_memory(tmp_allocator.sp);
}

Slice<GPU::PhysicalDeviceID> VulkanAdapter::physical_devices_enumerate()
{
    return physical_device_ids;
}

GPU::PhysicalDeviceInfo VulkanAdapter::physical_device_get_info(GPU::PhysicalDeviceID physical_device)
{
    VKFailOn(physical_device.integer() >= physical_devices.len);
    PhysicalDevice& pd = physical_devices[physical_device.integer()];
    return pd.info;
}

GPU::SurfaceID VulkanAdapter::surface_create(const GPU::SurfaceCreateInfo &ci)
{
    GPU::SurfaceID surface_id = surfaces.add(Surface());
    Surface& surface = _get_surface(surface_id);
#if defined(BREAD_WIN32)
    surface.window_native_handle = ci.window_native_handle;
    surface.vk_surface = Vulkan::create_surface(this, instance, ci.window_native_handle);
#elif defined(BREAD_ANDROID)
    if(dummy_surface == VK_NULL_HANDLE)
    {
        dummy_surface = Vulkan::create_surface(instance, Display::window_get_native_handle(Display::WindowID()));
    }
    Unused(ci);
    surface.vk_surface = dummy_surface;
#endif
    return surface_id;
}

void VulkanAdapter::surface_destroy(GPU::SurfaceID surface)
{
#if defined(BREAD_WIN32)
    Surface& s = _get_surface(surface);
    vk.vkDestroySurfaceKHR(instance, s.vk_surface, Vulkan::allocation_callbacks(this));

#elif defined(BREAD_ANDROID)
    Unused(surface);
#endif
    surfaces.remove(surface);
}

GPU::DeviceID VulkanAdapter::device_create(GPU::PhysicalDeviceID physical_device, const GPU::DeviceCreateInfo& ci)
{
    VKFailOn(
        physical_device.integer() >= physical_devices.len, 
        "invalid physical device"
    );
    Unused(ci);

    PhysicalDevice& pd = physical_devices[physical_device.integer()];
    Mem::Allocator* allocator = acquire_tmp_allocator();
    
    GPU::DeviceID device_id = devices.add(LogicalDevice());
    LogicalDevice& ld = _get_logical_device(device_id);
    ld.vk_physical_device = pd.vk_physical_device;

    u32 property_count;
    vk.vkEnumerateDeviceExtensionProperties(pd.vk_physical_device, nullptr, &property_count, nullptr);
    
    Slice<VkExtensionProperties> device_extensions = allocator->array<VkExtensionProperties>(property_count);
    vk.vkEnumerateDeviceExtensionProperties(pd.vk_physical_device, nullptr, &property_count, device_extensions.ptr());
    
    VKDebugInfo("Device Extensions");
    for(VkExtensionProperties& extension : device_extensions)
    {
        VKDebugInfo("{}", Vulkan::vulkan_string_to_sv(extension.extensionName));
    }

    // Checking for required extensions and features use by the driver.
    Vulkan::AdditionalExtensionSupport add_ext = Vulkan::check_device_extensions(allocator, pd.vk_physical_device);
    Vulkan::check_device_features(pd.vk_physical_device);

    u32 vk_family_count;
    vk.vkGetPhysicalDeviceQueueFamilyProperties2(pd.vk_physical_device, &vk_family_count, nullptr);

    Slice<VkQueueFamilyProperties2> vk_families = allocator->array<VkQueueFamilyProperties2>(vk_family_count);
    Slice<u32> vk_acquired = allocator->array<u32>(vk_family_count);
    for(VkQueueFamilyProperties2& vk_family : vk_families)
    {
        vk_family.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
        vk_family.pNext = nullptr;
    }
    vk.vkGetPhysicalDeviceQueueFamilyProperties2(pd.vk_physical_device, &vk_family_count, vk_families.ptr());    

    // 0->graphics, 1->compute, 2->copy, 3->present
    uint32_t vk_graphics_index = MaxValue<uint32_t>;
    uint32_t vk_compute_index = MaxValue<uint32_t>;
    uint32_t vk_copy_index = MaxValue<uint32_t>;
    uint32_t vk_present_index = MaxValue<uint32_t>;

    Array<VkDeviceQueueCreateInfo> vk_queue_infos = Array<VkDeviceQueueCreateInfo>::with_allocator(allocator);
    f32 priority = 1.F;

    vk_graphics_index = _get_queue_family_for(vk_families, vk_acquired, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
    VKFailOn(vk_graphics_index == MaxValue<uint32_t>, "couldn't find the graphics queue");
    vk_acquired[vk_graphics_index] = 1;
    (void)vk_queue_infos.add(
        VkDeviceQueueCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = vk_graphics_index,
            .queueCount = vk_families[vk_graphics_index].queueFamilyProperties.queueCount,
            .pQueuePriorities = &priority,
        }
    );

    vk_compute_index = _get_queue_family_for(vk_families, vk_acquired, VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
    if(vk_compute_index != MaxValue<uint32_t>)
    {
        vk_acquired[vk_compute_index] = 1;

        (void)vk_queue_infos.add(
            VkDeviceQueueCreateInfo
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = vk_compute_index,
                .queueCount = vk_families[vk_compute_index].queueFamilyProperties.queueCount,
                .pQueuePriorities = &priority,
            }
        );
    }

    vk_copy_index = _get_queue_family_for(vk_families, vk_acquired, VK_QUEUE_TRANSFER_BIT);
    if(vk_copy_index != MaxValue<uint32_t>)
    {
        vk_acquired[vk_copy_index] = 1;

        (void)vk_queue_infos.add(
            VkDeviceQueueCreateInfo
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = vk_copy_index,
                .queueCount = vk_families[vk_copy_index].queueFamilyProperties.queueCount,
                .pQueuePriorities = &priority,
            }
        );
    }

    vk_present_index = _get_queue_family_for_present(ld.vk_physical_device, dummy_surface, vk_families);
    VKFailOn(vk_graphics_index == MaxValue<uint32_t> || vk_present_index == MaxValue<uint32_t>,
        "graphics and present queues are required");
    if(vk_present_index != vk_graphics_index && vk_present_index != vk_compute_index && vk_present_index != vk_copy_index)
    {
        (void)vk_queue_infos.add(
            VkDeviceQueueCreateInfo
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = vk_present_index,
                .queueCount = vk_families[vk_present_index].queueFamilyProperties.queueCount,
                .pQueuePriorities = &priority,
            }
        );
    }

    uint32_t extension_count = 0;
    const char** extensions = Vulkan::get_device_extensions(allocator, pd.vk_physical_device, add_ext, &extension_count);
    VkDeviceCreateInfo vk_device_info =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = Vulkan::get_device_features(allocator, add_ext),
        .flags = 0,
        .queueCreateInfoCount = static_cast<uint32_t>(vk_queue_infos.count),
        .pQueueCreateInfos = vk_queue_infos.slice().ptr(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = extension_count,
        .ppEnabledExtensionNames = extensions,
        .pEnabledFeatures = nullptr,
    };

    VkResult result = vk.vkCreateDevice(pd.vk_physical_device, &vk_device_info, Vulkan::allocation_callbacks(this), &ld.vk_device);
    VKFailOn(result != VK_SUCCESS, "vkCreateDevice({})", Vulkan::result_as_string(result));
    vk_queue_infos.destroy();
    
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
    const uint32_t vk_family_indices[MaxQueueFamilyCount] =
    {
        vk_graphics_index,
        vk_compute_index,
        vk_copy_index,
        vk_present_index,
    };

    for(usize i = 0; i < MaxQueueFamilyCount; i++)
    {
        LogicalDevice::QueueFamily& family = ld.families[i];

        family.vk_family_index = vk_family_indices[i];
        if(vk_family_indices[i] == MaxValue<uint32_t>)
        {
            continue;
        }

        family.vk_queues = get_allocator()->array<VkQueue>(vk_families[vk_family_indices[i]].queueFamilyProperties.queueCount);
        family.queue_ids = get_allocator()->array<GPU::QueueID>(vk_families[vk_family_indices[i]].queueFamilyProperties.queueCount);
        for(uint32_t queue_i = 0; queue_i < family.vk_queues.len; queue_i++)
        {
            ld.vk.vkGetDeviceQueue(ld.vk_device, family.vk_family_index, queue_i, &family.vk_queues[queue_i]);

            Queue& q = queues.add(
                Queue
                {
                    .vk_device = ld.vk_device,
                    .vk_queue = family.vk_queues[queue_i],
                    .vk_family_index = family.vk_family_index,
                    .queue_index = queue_i,
                    .device = device_id,
                    .queue = GPU::QueueID::invalid(),
                }
            );
            q.queue = GPU::QueueID(queues.count - 1);
            family.queue_ids[queue_i] = GPU::QueueID(queues.count - 1);
        }
    }

    ld.render_pass_cache = HashMap<VkDriverRenderPassKey, RenderPassCache>::with_allocator(get_allocator());

    ld.device = device_id;

    ld.feature_level = FeatureLevel::Level0;
    if(ld.additional_extension_support.has_dynamic_rendering)
    {
        ld.feature_level = FeatureLevel::Level1;
    }

    return device_id;
}

void VulkanAdapter::device_destroy(GPU::DeviceID device)
{
    LogicalDevice& ld = _get_logical_device(device);
    
    for(usize i = 0; i < MaxQueueFamilyCount; i++)
    {
        LogicalDevice::QueueFamily& queue_family = ld.families[i];

        if(queue_family.vk_queues.len == 0)
        {
            continue;
        }

        get_allocator()->free(Mem::to_bytes(queue_family.vk_queues));
        get_allocator()->free(Mem::to_bytes(queue_family.queue_ids));
    }
    
    for(RenderPassEntry& it : ld.render_pass_cache.iter())
    {
        ld.vk.vkDestroyFramebuffer(
            ld.vk_device, it.second.vk_framebuffer,
            Vulkan::allocation_callbacks(this)
        );
        ld.vk.vkDestroyRenderPass(ld.vk_device, it.second.vk_render_pass, Vulkan::allocation_callbacks(this));
    }
    ld.render_pass_cache.destroy();
    
    vk.vkDestroyDevice(ld.vk_device, Vulkan::allocation_callbacks(this));
    devices.remove(device);
}

GPU::SwapChainID VulkanAdapter::swap_chain_create(GPU::DeviceID device, const GPU::SwapChainCreateInfo& ci)
{
    LogicalDevice& ld = _get_logical_device(device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    GPU::SwapChainID sc_id = swap_chains.add(SwapChain());
    SwapChain& swap_chain = _get_swap_chain(sc_id);
    Surface& surface = _get_surface(ci.surface);

    // Creating the surface
    swap_chain.vk_device = ld.vk_device;
    swap_chain.vk_surface = surface.vk_surface;
    swap_chain.surface = ci.surface;
    swap_chain.device = device;
    swap_chain.image_count = 0;

    VkFormat vk_swapchain_format;
    VkColorSpaceKHR vk_swap_chain_color_space;
    _vk_get_surface_format(ci.format, &vk_swapchain_format, &vk_swap_chain_color_space);
    VkPresentModeKHR vk_present_mode = VkUtils::_vk_get_present_mode(ci.present_mode);

    VkSurfaceCapabilitiesKHR capabilities = _vk_get_surface_capabilities(ld.vk_physical_device, surface.vk_surface);
    VkExtent2D vk_swap_chain_extent = _vk_get_swap_chain_extent(ci.size, capabilities);

    VkSwapchainCreateInfoKHR vk_swap_chain_info =
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
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = vk_present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    VkResult result = ld.vk.vkCreateSwapchainKHR(ld.vk_device, &vk_swap_chain_info, Vulkan::allocation_callbacks(this), &swap_chain.vk_swapchain);
    VKFailOn(result != VK_SUCCESS, "vkCreateSwapchainKHR({})", Vulkan::result_as_string(result));

    {
        // images
        ld.vk.vkGetSwapchainImagesKHR(ld.vk_device, swap_chain.vk_swapchain, &swap_chain.image_count, nullptr);

        Slice<VkImage> vk_images = allocator->array<VkImage>(swap_chain.image_count);
        ld.vk.vkGetSwapchainImagesKHR(ld.vk_device, swap_chain.vk_swapchain, &swap_chain.image_count, vk_images.ptr());
        
        swap_chain.images = get_allocator()->array<SwapChainImage>(swap_chain.image_count);
        for (u32 i = 0; i < swap_chain.image_count; i++)
        {
            swap_chain.images[i].vk_image = vk_images[i];

            swap_chain.images[i].texture = textures.add(Texture());
            Texture& tex = _get_texture(swap_chain.images[i].texture);
            tex.vk_device = swap_chain.vk_device;
            tex.vk_image = swap_chain.images[i].vk_image;
		    tex.format = ci.format;
		    tex.extent = Vector3U(vk_swap_chain_extent.width, vk_swap_chain_extent.height, 1);
            tex.device = swap_chain.device;
        }
    }

    {
        // texture object
        for (u32 i = 0; i < swap_chain.image_count; i++)
        {
            swap_chain.images[i].texture_view = texture_view_create(device, 
                GPU::TextureViewCreateInfo::create(GPU::TextureViewType::Texture2D,
                    ci.format, swap_chain.images[i].texture, GPU::ComponentMapping::identity(),
                    GPU::TextureSubresourceRange::color(0, 1, 0, 1))
            );
        }
    }

    return sc_id;
}

void VulkanAdapter::swap_chain_destroy(GPU::SwapChainID swap_chain)
{
    SwapChain& sc = _get_swap_chain(swap_chain);
    LogicalDevice& ld = _get_logical_device(sc.device);

    for(u32 i = 0; i < sc.image_count; i++)
    {
        texture_view_destroy(sc.images[i].texture_view);
        
        textures.remove(sc.images[i].texture);
    }
    get_allocator()->free(Mem::to_bytes(sc.images));
    
    ld.vk.vkDestroySwapchainKHR(ld.vk_device, sc.vk_swapchain, Vulkan::allocation_callbacks(this));

    swap_chains.remove(swap_chain);
}

u32 VulkanAdapter::swap_chain_get_image_count(GPU::SwapChainID swap_chain)
{
    SwapChain& sc = _get_swap_chain(swap_chain);
    return sc.image_count; 
}

GPU::TextureID VulkanAdapter::swap_chain_get_image(GPU::SwapChainID swap_chain, u32 image_index)
{
    SwapChain& sc = _get_swap_chain(swap_chain);
    return sc.images[image_index].texture;
}

GPU::TextureViewID VulkanAdapter::swap_chain_get_image_view(GPU::SwapChainID swap_chain, u32 image_index)
{
    SwapChain& sc = _get_swap_chain(swap_chain);
    return sc.images[image_index].texture_view;
}

GPU::AcquireResult VulkanAdapter::swap_chain_acquire_next_image(GPU::SwapChainID swap_chain, const GPU::AcquireInfo& acquire_info, u32* image_index)
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
    if(result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        return GPU::AcquireResult::OutOfDate;
    }

    return GPU::AcquireResult::Acquired;
}

GPU::FenceID VulkanAdapter::fence_create(GPU::DeviceID device, const GPU::FenceCreateInfo& ci)
{
    LogicalDevice& ld = _get_logical_device(device);
    
    GPU::FenceID fence_id = fences.add(Fence());
    Fence& fence = _get_fence(fence_id);

    fence.vk_device = ld.vk_device;
    fence.device = device;

    VkFenceCreateInfo fence_info =
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = ci.signaled ? VK_FENCE_CREATE_SIGNALED_BIT : VkFenceCreateFlags(0),
    };
    
    VkResult result = ld.vk.vkCreateFence(ld.vk_device, &fence_info, Vulkan::allocation_callbacks(this), &fence.vk_fence);
    VKFailOn(result != VK_SUCCESS, "vkCreateFence({})", Vulkan::result_as_string(result));
    fence.device = device;

    return fence_id;
}

void VulkanAdapter::fence_destroy(GPU::FenceID fence)
{
    Fence& f = _get_fence(fence);
    LogicalDevice& ld = _get_logical_device(f.device);

    ld.vk.vkDestroyFence(f.vk_device, f.vk_fence, Vulkan::allocation_callbacks(this));

    fences.remove(fence);
}

bool VulkanAdapter::fence_get_state(GPU::FenceID fence)
{
    Fence& f = _get_fence(fence);
    LogicalDevice& ld = _get_logical_device(f.device);

    VkResult result = ld.vk.vkGetFenceStatus(ld.vk_device, f.vk_fence);

    return result == VK_SUCCESS;
}

void VulkanAdapter::fence_reset(Slice<GPU::FenceID> fences)
{
    GPU::DeviceID first_device = _get_fence(fences[0]).device;
    LogicalDevice& ld = _get_logical_device(first_device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    Slice<VkFence> vk_fences = allocator->array<VkFence>(fences.len);

    for(usize i = 0; i < fences.len; i++)
    {
        Fence& fence = _get_fence(fences[i]);
        VKFailOn(first_device != fence.device, "fences must share the same device");

        vk_fences[i] = fence.vk_fence;
    }

    VkResult result = ld.vk.vkResetFences(ld.vk_device, static_cast<uint32_t>(vk_fences.len), vk_fences.ptr());
    VKFailOn(result != VK_SUCCESS, "vkResetFences({})", Vulkan::result_as_string(result));
}

void VulkanAdapter::fence_wait_for(Slice<GPU::FenceID> fences, bool wait_for_all, u64 timeout)
{
    GPU::DeviceID first_device = _get_fence(fences[0]).device;
    LogicalDevice& ld = _get_logical_device(first_device);
    Mem::Allocator* allocator = acquire_tmp_allocator();
    
    Slice<VkFence> vk_fences = allocator->array<VkFence>(fences.len);
    for(usize i = 0; i < fences.len; i++)
    {
        Fence& fence = _get_fence(fences[i]);
        VKFailOn(first_device != fence.device, "fences must share the same device");

        vk_fences[i] = fence.vk_fence;
    }

    VkResult result = ld.vk.vkWaitForFences(
        ld.vk_device, static_cast<uint32_t>(vk_fences.len), vk_fences.ptr(),
        wait_for_all ? VK_TRUE : VK_FALSE, timeout
    );
    VKFailOn(result != VK_SUCCESS, "vkWaitForFences({})", Vulkan::result_as_string(result));   
}

GPU::SemaphoreID VulkanAdapter::semaphore_create(GPU::DeviceID device, const GPU::SemaphoreCreateInfo &ci)
{
    Unused(ci);
    
    GPU::SemaphoreID semaphore_id = semaphores.add(Semaphore());
    Semaphore& sem = _get_semaphore(semaphore_id);
    LogicalDevice& ld = _get_logical_device(device);

    sem.vk_device = ld.vk_device;
    sem.device = device;

    VkSemaphoreCreateInfo semaphore_info =
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    VkResult result = ld.vk.vkCreateSemaphore(ld.vk_device, &semaphore_info, Vulkan::allocation_callbacks(this), &sem.vk_semaphore);
    VKFailOn(result != VK_SUCCESS, "vkCreateSemaphore({})", Vulkan::result_as_string(result));   

    return semaphore_id;
}

void VulkanAdapter::semaphore_destroy(GPU::SemaphoreID semaphore)
{
    Semaphore& sem = _get_semaphore(semaphore);
    LogicalDevice& ld = _get_logical_device(sem.device);

    ld.vk.vkDestroySemaphore(sem.vk_device, sem.vk_semaphore, Vulkan::allocation_callbacks(this));
    
    semaphores.remove(semaphore);
}

u32 VulkanAdapter::queue_get_count(GPU::DeviceID device, const GPU::QueueGetCountInfo& gci)
{
    LogicalDevice& ld = _get_logical_device(device);

    LogicalDevice::QueueFamily& family = ld.families[usize(gci.usage) - 1];
    return family.queue_ids.len;
}

GPU::QueueID VulkanAdapter::queue_get(GPU::DeviceID device, const GPU::QueueGetInfo& gi)
{
    LogicalDevice& ld = _get_logical_device(device);

    LogicalDevice::QueueFamily& family = ld.families[usize(gi.usage) - 1];
    return family.queue_ids[gi.index];
}

void VulkanAdapter::queue_execute_command_buffer(GPU::QueueID queue, const GPU::QueueExecuteInfo& execute_info)
{
    Queue& q = _get_queue(queue);
    LogicalDevice& ld = _get_logical_device(q.device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    Slice<VkSemaphore> vk_wait_sem = allocator->array<VkSemaphore>(execute_info.wait_semaphores.len);
    Slice<VkPipelineStageFlags> vk_wait_stages = allocator->array<VkPipelineStageFlags>(execute_info.wait_stages.len);
    for(usize i = 0; i < execute_info.wait_semaphores.len; i++)
    {
        Semaphore& sem = _get_semaphore(execute_info.wait_semaphores[i]);
        vk_wait_sem[i] = sem.vk_semaphore;
        vk_wait_stages[i] = VkUtils::_vk_get_pipeline_stages(execute_info.wait_stages[i]);
    }

    Slice<VkSemaphore> vk_signal_sem = allocator->array<VkSemaphore>(execute_info.signal_semaphores.len);
    for(usize i = 0; i < execute_info.signal_semaphores.len; i++)
    {
        Semaphore& sem = _get_semaphore(execute_info.signal_semaphores[i]);
        vk_signal_sem[i] = sem.vk_semaphore;
    }

    Slice<VkCommandBuffer> vk_cmd_buffers = allocator->array<VkCommandBuffer>(execute_info.command_buffers.len);
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

GPU::AcquireResult VulkanAdapter::queue_present(GPU::QueueID queue, const GPU::QueuePresentInfo& present_info)
{
    Queue& q = _get_queue(queue);
    LogicalDevice& ld = _get_logical_device(q.device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    Slice<VkSemaphore> vk_wait_semaphores = allocator->array<VkSemaphore>(present_info.wait_semaphores.len);
    for(usize i = 0; i < present_info.wait_semaphores.len; i++)
    {
        Semaphore& sem = _get_semaphore(present_info.wait_semaphores[i]);
        vk_wait_semaphores[i] = sem.vk_semaphore;
    }

    Slice<VkSwapchainKHR> vk_swapchains = allocator->array<VkSwapchainKHR>(present_info.swapchains.len);
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
    if(call_result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        return GPU::AcquireResult::OutOfDate;
    }

    return GPU::AcquireResult::Acquired;
}

void VulkanAdapter::queue_wait_idle(GPU::QueueID queue)
{
    Queue& q = _get_queue(queue);
    LogicalDevice& ld = _get_logical_device(q.device);

    VkResult result = ld.vk.vkQueueWaitIdle(q.vk_queue);
    VKFailOn(result != VK_SUCCESS, "vkQueueWaitIdle({})", Vulkan::result_as_string(result));
}

GPU::MemoryHeapID VulkanAdapter::memory_heap_create(GPU::DeviceID device, const GPU::MemoryHeapCreateInfo& ci)
{
    LogicalDevice& ld = _get_logical_device(device);
    GPU::MemoryHeapID memory_heap_id = memory_heaps.add(MemoryHeap());
    MemoryHeap& heap = _get_memory_heap(memory_heap_id);
    heap.vk_device = ld.vk_device;
    heap.device = device;
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
        vk_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
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

    VkResult result = ld.vk.vkAllocateMemory(ld.vk_device, &vk_allocate_info, Vulkan::allocation_callbacks(this), &heap.vk_memory);
    VKFailOn(result != VK_SUCCESS, "vkAllocateMemory({})", Vulkan::result_as_string(result));

    return memory_heap_id;
}

void VulkanAdapter::memory_heap_destroy(GPU::MemoryHeapID memory_heap)
{
    MemoryHeap& heap = _get_memory_heap(memory_heap);
    LogicalDevice& ld = _get_logical_device(heap.device);

    ld.vk.vkFreeMemory(heap.vk_device, heap.vk_memory, Vulkan::allocation_callbacks(this));

    memory_heaps.remove(memory_heap);
}

Slice<u8> VulkanAdapter::memory_heap_map(GPU::MemoryHeapID memory_heap, usize offset, usize len)
{
    MemoryHeap& heap = _get_memory_heap(memory_heap);
    LogicalDevice& ld = _get_logical_device(heap.device);

    void* ptr = nullptr;

    VkResult result = ld.vk.vkMapMemory(ld.vk_device, heap.vk_memory, offset, len, 0, &ptr);
    VKFailOn(result != VK_SUCCESS, "vkMapMemory({})", Vulkan::result_as_string(result));

    return Slice<u8>(reinterpret_cast<u8*>(ptr), len);
}

void VulkanAdapter::memory_heap_unmap(GPU::MemoryHeapID memory_heap, const Slice<u8>& memory)
{
    Unused(memory);
    MemoryHeap& heap = _get_memory_heap(memory_heap);
    LogicalDevice& ld = _get_logical_device(heap.device);

    ld.vk.vkUnmapMemory(ld.vk_device, heap.vk_memory);
}

GPU::BufferID VulkanAdapter::buffer_create(GPU::DeviceID device, const GPU::BufferCreateInfo& ci)
{
    LogicalDevice& ld = _get_logical_device(device);
    GPU::BufferID buffer_id = buffers.add(Buffer());
    Buffer& buffer = _get_buffer(buffer_id);

    buffer.vk_device = ld.vk_device;
    buffer.device = device;
    buffer.buffer = buffer_id;

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

    VkResult result = ld.vk.vkCreateBuffer(ld.vk_device, &vk_buffer_info, Vulkan::allocation_callbacks(this), &buffer.vk_buffer);
    VKFailOn(result != VK_SUCCESS, "vkCreateBuffer({})", Vulkan::result_as_string(result));

    return buffer_id;
}

void VulkanAdapter::buffer_destroy(GPU::BufferID buffer)
{
    Buffer& b = _get_buffer(buffer);
    LogicalDevice& ld = _get_logical_device(b.device);

    ld.vk.vkDestroyBuffer(b.vk_device, b.vk_buffer, Vulkan::allocation_callbacks(this));

    buffers.remove(buffer);
}

GPU::MemoryRequirements VulkanAdapter::buffer_get_memory_requirements(GPU::BufferID buffer)
{
    Buffer& b = _get_buffer(buffer);
    LogicalDevice& ld = _get_logical_device(b.device);

    VkBufferMemoryRequirementsInfo2 vk_buffer_req =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
        .pNext = nullptr,
        .buffer = b.vk_buffer,
    };

    VkMemoryRequirements2 vk_memory_requirements =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
        .pNext = nullptr,
        .memoryRequirements = {},
    };
    
    ld.vk.vkGetBufferMemoryRequirements2(ld.vk_device, &vk_buffer_req, &vk_memory_requirements);

    GPU::MemoryRequirements requirements =
    {
        .size = vk_memory_requirements.memoryRequirements.size,
        .alignment = vk_memory_requirements.memoryRequirements.alignment,
        .heap_usage = GPU::HeapUsage::Unknown,
    };

    for(uint32_t i = 0; i < ld.vk_physical_device_memory_properties.memoryTypeCount; i++)
    {
        if(!HasValue((1 << i) & vk_memory_requirements.memoryRequirements.memoryTypeBits))
        {
            continue;
        }

        requirements.heap_usage = _vk_memory_property_to_heap_usage(ld.vk_physical_device_memory_properties.memoryTypes[i].propertyFlags);
        break;
    }

    VKFailOn(requirements.heap_usage == GPU::HeapUsage::Unknown, "invalid heap usage");

    return requirements;
}

void VulkanAdapter::buffer_bind_memory_heap(GPU::BufferID buffer, const GPU::BindMemoryInfo& bind_info)
{
    Buffer& b = _get_buffer(buffer);
    LogicalDevice& ld = _get_logical_device(b.device);
    MemoryHeap& heap = _get_memory_heap(bind_info.memory_heap);

    VkBindBufferMemoryInfo vk_bind_info =
    {
        .sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO,
        .pNext = nullptr,
        .buffer = b.vk_buffer,
        .memory = heap.vk_memory,
        .memoryOffset = bind_info.heap_offset,
    };

    VkResult result = ld.vk.vkBindBufferMemory2(ld.vk_device, 1, &vk_bind_info);
    VKFailOn(result != VK_SUCCESS, "vkBindBufferMemory2({})", Vulkan::result_as_string(result));
}

GPU::SamplerID VulkanAdapter::sampler_create(GPU::DeviceID device, const GPU::SamplerCreateInfo& ci)
{
    GPU::SamplerID sampler_id = samplers.add(Sampler());
    Sampler& sam = _get_sampler(sampler_id);
    LogicalDevice& ld = _get_logical_device(device);

    sam.vk_device = ld.vk_device;
    sam.device = device;
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

    ld.vk.vkCreateSampler(ld.vk_device, &vk_sampler_info, Vulkan::allocation_callbacks(this), &sam.vk_sampler);

    return sampler_id;
}

void VulkanAdapter::sampler_destroy(GPU::SamplerID sampler)
{
    Sampler& sam = _get_sampler(sampler);
    LogicalDevice& ld = _get_logical_device(sam.device);

    ld.vk.vkDestroySampler(sam.vk_device, sam.vk_sampler, Vulkan::allocation_callbacks(this));

    samplers.remove(sampler);
}

GPU::TextureID VulkanAdapter::texture_create(GPU::DeviceID device, const GPU::TextureCreateInfo& ci)
{
    GPU::TextureID texture_id = textures.add(Texture());
    Texture& tex = _get_texture(texture_id);
    LogicalDevice& ld = _get_logical_device(device);
    tex.vk_device = ld.vk_device;
    tex.format = ci.format;
    tex.extent = ci.extent;
    tex.device = device;
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
    
    VkResult result = ld.vk.vkCreateImage(ld.vk_device, &vk_image_info, Vulkan::allocation_callbacks(this), &tex.vk_image);
    VKFailOn(result != VK_SUCCESS, "vkCreateImage({})", Vulkan::result_as_string(result));

    return texture_id;
}

void VulkanAdapter::texture_destroy(GPU::TextureID texture)
{
    Texture& tex = _get_texture(texture);
    LogicalDevice& ld = _get_logical_device(tex.device);

    ld.vk.vkDestroyImage(tex.vk_device, tex.vk_image, Vulkan::allocation_callbacks(this));

    textures.remove(texture);
}

GPU::MemoryRequirements VulkanAdapter::texture_get_memory_requirements(GPU::TextureID texture)
{
    Texture& tex = _get_texture(texture);
    LogicalDevice& ld = _get_logical_device(tex.device);

    VkImageMemoryRequirementsInfo2 vk_buffer_req =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2,
        .pNext = nullptr,
        .image = tex.vk_image,
    };

    VkMemoryRequirements2 vk_memory_requirements =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
        .pNext = nullptr,
        .memoryRequirements = {},
    };
    
    ld.vk.vkGetImageMemoryRequirements2(ld.vk_device, &vk_buffer_req, &vk_memory_requirements);

    GPU::MemoryRequirements requirements =
    {
        .size = vk_memory_requirements.memoryRequirements.size,
        .alignment = vk_memory_requirements.memoryRequirements.alignment,
        .heap_usage = GPU::HeapUsage::Unknown,
    };

    for(uint32_t i = 0; i < ld.vk_physical_device_memory_properties.memoryTypeCount; i++)
    {
        if(!HasValue((1 << i) & vk_memory_requirements.memoryRequirements.memoryTypeBits))
        {
            continue;
        }

        requirements.heap_usage = _vk_memory_property_to_heap_usage(ld.vk_physical_device_memory_properties.memoryTypes[i].propertyFlags);
        break;
    }

    VKFailOn(requirements.heap_usage == GPU::HeapUsage::Unknown, "invalid heap usage");

    return requirements;
}

void VulkanAdapter::texture_bind_memory_heap(GPU::TextureID texture, const GPU::BindMemoryInfo &bind_info)
{
    Texture& tex = _get_texture(texture);
    LogicalDevice& ld = _get_logical_device(tex.device);
    MemoryHeap& heap = _get_memory_heap(bind_info.memory_heap);

    VkBindImageMemoryInfo vk_bind_info =
    {
        .sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO,
        .pNext = nullptr,
        .image = tex.vk_image,
        .memory = heap.vk_memory,
        .memoryOffset = bind_info.heap_offset,
    };

    ld.vk.vkBindImageMemory2(ld.vk_device, 1, &vk_bind_info);
}

GPU::TextureViewID VulkanAdapter::texture_view_create(GPU::DeviceID device, const GPU::TextureViewCreateInfo &ci)
{
    GPU::TextureViewID texture_view_id = texture_views.add(TextureView());
    TextureView& tex_view = _get_texture_view(texture_view_id);
    LogicalDevice& ld = _get_logical_device(device);
    Texture& tex = _get_texture(ci.texture);

    tex_view.vk_device = ld.vk_device;
    tex_view.format = ci.format;
    tex_view.device = device;
    tex_view.texture_view = texture_view_id;

    VkImageViewCreateInfo vk_image_view_info =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = tex.vk_image,
        .viewType = VkUtils::_vk_get_image_view_type(ci.type),
        .format = VkUtils::_vk_get_texture_format(ci.format),
        .components =
        {
            .r = VkUtils::_vk_get_component_swizzle(ci.components.r),
            .g = VkUtils::_vk_get_component_swizzle(ci.components.g),
            .b = VkUtils::_vk_get_component_swizzle(ci.components.b),
            .a = VkUtils::_vk_get_component_swizzle(ci.components.a),
        },
        .subresourceRange =
        {
            // TODO: check if format is depth or stencil
            .aspectMask = VkUtils::_vk_get_aspect_masks(ci.subresource_range.aspect),
            .baseMipLevel = ci.subresource_range.base_mip_level,
            .levelCount = ci.subresource_range.level_count,
            .baseArrayLayer = ci.subresource_range.base_array_layer,
            .layerCount = ci.subresource_range.layer_count,
        },
    };

    VkResult result = ld.vk.vkCreateImageView(ld.vk_device, &vk_image_view_info, Vulkan::allocation_callbacks(this), &tex_view.vk_image_view);
    VKFailOn(result != VK_SUCCESS, "vkCreateImage({})", Vulkan::result_as_string(result));

    return texture_view_id;
}

void VulkanAdapter::texture_view_destroy(GPU::TextureViewID texture_view)
{
    TextureView& tex_view = _get_texture_view(texture_view);
    LogicalDevice& ld = _get_logical_device(tex_view.device);

    ld.vk.vkDestroyImageView(tex_view.vk_device, tex_view.vk_image_view, Vulkan::allocation_callbacks(this));

    texture_views.remove(texture_view);
}

GPU::DescriptorSetLayoutID VulkanAdapter::descriptor_set_layout_create(GPU::DeviceID device, const GPU::DescriptorSetLayoutCreateInfo& ci)
{
    GPU::DescriptorSetLayoutID descriptor_set_layout_id = descriptor_set_layouts.add(DescriptorSetLayout());
    DescriptorSetLayout& layout = _get_descriptor_set_layout(descriptor_set_layout_id);
    LogicalDevice& ld = _get_logical_device(device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    layout.vk_device = ld.vk_device;
    layout.device = device;
    layout.descriptor_set_layout = descriptor_set_layout_id;

    Slice<VkDescriptorSetLayoutBinding> vk_bindings = allocator->array<VkDescriptorSetLayoutBinding>(ci.bindings.len);
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

    VkResult result = ld.vk.vkCreateDescriptorSetLayout(ld.vk_device, &vk_set_layout_info, Vulkan::allocation_callbacks(this), &layout.vk_set_layout);
    VKFailOn(result != VK_SUCCESS, "vkCreateDescriptorSetLayout({})", Vulkan::result_as_string(result));
 
    return descriptor_set_layout_id;
}

void VulkanAdapter::descriptor_set_layout_destroy(GPU::DescriptorSetLayoutID descriptor_set_layout)
{
    DescriptorSetLayout& layout = _get_descriptor_set_layout(descriptor_set_layout);
    LogicalDevice& ld = _get_logical_device(layout.device);

    ld.vk.vkDestroyDescriptorSetLayout(layout.vk_device, layout.vk_set_layout, Vulkan::allocation_callbacks(this));

    descriptor_set_layouts.remove(descriptor_set_layout);
}

GPU::DescriptorPoolID VulkanAdapter::descriptor_pool_create(GPU::DeviceID device, const GPU::DescriptorPoolCreateInfo& ci)
{
    GPU::DescriptorPoolID descriptor_pool_id = descriptor_pools.add(DescriptorPool());
    DescriptorPool& descriptor_pool = descriptor_pools.get(descriptor_pool_id);
    LogicalDevice& ld = _get_logical_device(device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    descriptor_pool.vk_device = ld.vk_device;
    descriptor_pool.device = device;
    descriptor_pool.descriptor_pool = descriptor_pool_id;
    descriptor_pool.allocated_sets = Array<GPU::DescriptorSetID>::with_allocator(get_allocator());

    Slice<VkDescriptorPoolSize> vk_pool_sizes = allocator->array<VkDescriptorPoolSize>(ci.sizes.len);
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

    VkResult result = ld.vk.vkCreateDescriptorPool(ld.vk_device, &vk_global_pool_info, Vulkan::allocation_callbacks(this), &descriptor_pool.vk_descriptor_pool);
    VKFailOn(result != VK_SUCCESS, "vkCreateDescriptorPool({})", Vulkan::result_as_string(result));

    return descriptor_pool_id;
}

void VulkanAdapter::descriptor_pool_destroy(GPU::DescriptorPoolID descriptor_pool)
{
    DescriptorPool& pool = descriptor_pools.get(descriptor_pool);
    LogicalDevice& ld = _get_logical_device(pool.device);

    ld.vk.vkDestroyDescriptorPool(pool.vk_device, pool.vk_descriptor_pool, Vulkan::allocation_callbacks(this));
    pool.allocated_sets.destroy();

    descriptor_pools.remove(descriptor_pool);
}

void VulkanAdapter::descriptor_pool_reset(GPU::DescriptorPoolID descriptor_pool)
{
    DescriptorPool& pool = descriptor_pools.get(descriptor_pool);
    LogicalDevice& ld = _get_logical_device(pool.device);

    ld.vk.vkResetDescriptorPool(pool.vk_device, pool.vk_descriptor_pool, VkDescriptorPoolResetFlags());
    for(GPU::DescriptorSetID set : pool.allocated_sets.iter())
    {
        descriptor_sets.remove(set);
    }
}

void VulkanAdapter::descriptor_set_allocate(GPU::DeviceID device, const GPU::DescriptorSetAllocateInfo& ci, Slice<GPU::DescriptorSetID> out_descriptor_sets)
{
    DescriptorPool& pool = _get_descriptor_pool(ci.pool);
    LogicalDevice& ld = _get_logical_device(device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    Slice<VkDescriptorSetLayout> vk_set_layouts = allocator->array<VkDescriptorSetLayout>(ci.set_layouts.len);
    Slice<VkDescriptorSet> vk_sets = allocator->array<VkDescriptorSet>(ci.set_layouts.len);
    for(usize i = 0; i < vk_set_layouts.len; i++)
    {
        vk_set_layouts[i] = _get_descriptor_set_layout(ci.set_layouts[i]).vk_set_layout;
    }

    VkDescriptorSetAllocateInfo vk_allocate_set_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = pool.vk_descriptor_pool,
        .descriptorSetCount = static_cast<uint32_t>(vk_set_layouts.len),
        .pSetLayouts = vk_set_layouts.ptr(),
    };

    VkResult result = ld.vk.vkAllocateDescriptorSets(ld.vk_device, &vk_allocate_set_info, vk_sets.ptr());
    VKFailOn(result != VK_SUCCESS, "vkAllocateDescriptorSets({})", Vulkan::result_as_string(result));

    for(usize i = 0; i < vk_sets.len; i++)
    {
        out_descriptor_sets[i] = descriptor_sets.add(DescriptorSet());

        DescriptorSet& set = _get_descriptor_set(out_descriptor_sets[i]);
        set.vk_device = ld.vk_device;
        set.vk_descriptor_pool = pool.vk_descriptor_pool;
        set.vk_descriptor_set = vk_sets[i];
        set.device = device;
        set.descriptor_set = out_descriptor_sets[i];
        set.descriptor_pool = ci.pool;

        (void)pool.allocated_sets.add(out_descriptor_sets[i]);
    }
}

void VulkanAdapter::descriptor_set_free(GPU::DescriptorPoolID descriptor_pool, const Slice<const GPU::DescriptorSetID>& _descriptor_sets)
{
    DescriptorPool& pool = _get_descriptor_pool(descriptor_pool);
    LogicalDevice& ld = _get_logical_device(pool.device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    Slice<VkDescriptorSet> vk_sets = allocator->array<VkDescriptorSet>(_descriptor_sets.len);
    for(usize i = 0; i < vk_sets.len; i++)
    {
        vk_sets[i] = _get_descriptor_set(_descriptor_sets[i]).vk_descriptor_set;
    }

    VkResult result = ld.vk.vkFreeDescriptorSets(
        ld.vk_device, pool.vk_descriptor_pool,
        static_cast<uint32_t>(vk_sets.len), vk_sets.ptr()
    );
    VKFailOn(result != VK_SUCCESS, "vkFreeDescriptorSets({})", Vulkan::result_as_string(result));

    for(usize i = 0; i < vk_sets.len; i++)
    {
        descriptor_sets.remove(_descriptor_sets[i]);
        pool.allocated_sets.remove(_descriptor_sets[i]);
    }
}

void VulkanAdapter::descriptor_set_update_descriptors(GPU::DeviceID device, const GPU::UpdateDescriptorInfo& update_info)
{
    LogicalDevice& ld = _get_logical_device(device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    Slice<VkWriteDescriptorSet> vk_write_descriptor = allocator->array<VkWriteDescriptorSet>(update_info.write_infos.len);

    usize total_buffer_infos = 0;
    usize total_image_infos = 0;
    for(usize i = 0; i < update_info.write_infos.len; i++)
    {
        if(update_info.write_infos[i].type == GPU::DescriptorType::UniformBuffer
            || update_info.write_infos[i].type == GPU::DescriptorType::StorageBuffer)
        {
            total_buffer_infos += update_info.write_infos[i].buffers.len;
        }
        else if(update_info.write_infos[i].type == GPU::DescriptorType::CombinedTextureSampler)
        {
            total_image_infos += update_info.write_infos[i].textures.len;
        }
    }

    Slice<VkDescriptorBufferInfo> vk_buffer_infos = allocator->array<VkDescriptorBufferInfo>(total_buffer_infos);
    Slice<VkDescriptorImageInfo> vk_image_infos = allocator->array<VkDescriptorImageInfo>(total_image_infos);
    usize vk_buffer_infos_index = 0;
    usize vk_image_infos_index = 0;

    for(usize i = 0; i < update_info.write_infos.len; i++)
    {
        const GPU::WriteDescriptorInfo& write_info = update_info.write_infos[i];
        vk_write_descriptor[i] =
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _get_descriptor_set(write_info.descriptor_set).vk_descriptor_set,
            .dstBinding = write_info.binding,
            .dstArrayElement = write_info.array_element,
            .descriptorCount = 0, // Based on type
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
            vk_write_descriptor[i].descriptorCount = static_cast<uint32_t>(write_info.buffers.len);
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
            vk_write_descriptor[i].descriptorCount = static_cast<uint32_t>(write_info.textures.len);
            for(usize texture_i = 0; texture_i < write_info.textures.len; texture_i++)
            {
                const GPU::DescriptorTextureInfo& texture_info = write_info.textures[texture_i];

                vk_image_infos[vk_image_infos_index] =
                {
                    .sampler = _get_sampler(texture_info.sampler).vk_sampler,
                    .imageView = _get_texture_view(texture_info.texture_view).vk_image_view,
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
        ld.vk_device, static_cast<uint32_t>(vk_write_descriptor.len), vk_write_descriptor.ptr(),
        0, nullptr
    );
}

GPU::PipelineLayoutID VulkanAdapter::pipeline_layout_create(GPU::DeviceID device, const GPU::PipelineLayoutCreateInfo& ci)
{
    LogicalDevice& ld = _get_logical_device(device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    GPU::PipelineLayoutID pipeline_layout_id = pipeline_layouts.add(PipelineLayout());
    PipelineLayout& pipe_layout = _get_pipeline_layout(pipeline_layout_id);
    pipe_layout.vk_device = ld.vk_device;
    pipe_layout.device = device;
    pipe_layout.pipeline_layout = pipeline_layout_id;

    Slice<VkPushConstantRange> vk_push_ranges = allocator->array<VkPushConstantRange>(ci.constant_blocks.len);
    for(usize i = 0; i < ci.constant_blocks.len; i++)
    {
        vk_push_ranges[i] =
        {
            .stageFlags = VkUtils::_vk_get_shader_stage(ci.constant_blocks[i].stages),
            .offset = ci.constant_blocks[i].offset,
            .size = ci.constant_blocks[i].size,
        };
    }

    Slice<VkDescriptorSetLayout> vk_set_layouts = allocator->array<VkDescriptorSetLayout>(ci.set_layouts.len);
    for(usize i = 0; i < ci.set_layouts.len; i++)
    {
        vk_set_layouts[i] = _get_descriptor_set_layout(ci.set_layouts[i]).vk_set_layout;
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

    VkResult result = ld.vk.vkCreatePipelineLayout(ld.vk_device, &vk_pipeline_layout_info, Vulkan::allocation_callbacks(this), &pipe_layout.vk_pipeline_layout);
    VKFailOn(result != VK_SUCCESS, "vkCreatePipelineLayout({})", Vulkan::result_as_string(result));

    return pipeline_layout_id;
}

void VulkanAdapter::pipeline_layout_destroy(GPU::PipelineLayoutID pipeline_layout)
{
    PipelineLayout& pipe_layout = _get_pipeline_layout(pipeline_layout);
    LogicalDevice& ld = _get_logical_device(pipe_layout.device);

    ld.vk.vkDestroyPipelineLayout(pipe_layout.vk_device, pipe_layout.vk_pipeline_layout, Vulkan::allocation_callbacks(this)); 

    pipeline_layouts.remove(pipeline_layout);
}

GPU::PipelineID VulkanAdapter::pipeline_create(GPU::DeviceID device, const GPU::PipelineCreateInfo& ci)
{
    LogicalDevice& ld = _get_logical_device(device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    GPU::PipelineID pipeline_id = pipelines.add(Pipeline());
    Pipeline& pipe = _get_pipeline(pipeline_id);
    pipe.vk_device = ld.vk_device;
    pipe.device = device;
    pipe.pipeline = pipeline_id;

    Slice<VkPipelineShaderStageCreateInfo> vk_shader_stages = allocator->array<VkPipelineShaderStageCreateInfo>(ci.shader_stages.len);

    for(usize i = 0; i < ci.shader_stages.len; i++)
    {
        Slice<char> null_terminated = allocator->array<char>(ci.shader_stages[i].name.len + 1);
        Mem::copy(null_terminated, ci.shader_stages[i].name);
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

    Slice<VkVertexInputBindingDescription> vk_vertex_bindings = allocator->array<VkVertexInputBindingDescription>(ci.vertex_input.bindings.len);
    Slice<VkVertexInputAttributeDescription> vk_vertex_attributes = allocator->array<VkVertexInputAttributeDescription>(ci.vertex_input.attributes.len);
    
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
    VkPipelineLayout vk_pipeline_layout = _get_pipeline_layout(ci.pipeline_layout).vk_pipeline_layout;

    Slice<VkFormat> vk_color_attachment_formats = allocator->array<VkFormat>(ci.rendering_info.render_attachment_formats.len);
    for(usize i = 0; i < vk_color_attachment_formats.len; i++)
    {
        vk_color_attachment_formats[i] = VkUtils::_vk_get_texture_format(ci.rendering_info.render_attachment_formats[i]);
    }

    VkPipelineRenderingCreateInfoKHR vk_pipeline_rendering_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .pNext = nullptr,
        .viewMask = 0,
        .colorAttachmentCount = static_cast<uint32_t>(vk_color_attachment_formats.len),
        .pColorAttachmentFormats = vk_color_attachment_formats.ptr(),
        .depthAttachmentFormat = ci.rendering_info.depth_attachment_format == GPU::TextureFormat::Unknown ?
            VK_FORMAT_UNDEFINED : VkUtils::_vk_get_texture_format(ci.rendering_info.depth_attachment_format),
        .stencilAttachmentFormat = ci.rendering_info.stencil_attachment_format == GPU::TextureFormat::Unknown ?
            VK_FORMAT_UNDEFINED : VkUtils::_vk_get_texture_format(ci.rendering_info.stencil_attachment_format),
    };

    RenderPassCache* render_pass = nullptr;
    if(ld.feature_level == FeatureLevel::Level0)
    {
        render_pass = &_get_render_pass_for_pipeline(ld, ci.rendering_info);
    }
    
    VkGraphicsPipelineCreateInfo vk_graphics_pipeline_info =
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = ld.feature_level == FeatureLevel::Level1 ? &vk_pipeline_rendering_info : nullptr,
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
        .layout = vk_pipeline_layout,
        .renderPass = ld.feature_level == FeatureLevel::Level1 ? nullptr : render_pass->vk_render_pass,
        .subpass = 0, // TODO: can change
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
    };

    VkResult result = ld.vk.vkCreateGraphicsPipelines(
        ld.vk_device, VK_NULL_HANDLE, 1, &vk_graphics_pipeline_info,
        Vulkan::allocation_callbacks(this), &pipe.vk_pipeline
    );
    VKFailOn(result != VK_SUCCESS, "vkCreateGraphicsPipelines({})", Vulkan::result_as_string(result));

    // destroy shader modules
    for(usize i = 0; i < ci.shader_stages.len; i++)
    {
        ld.vk.vkDestroyShaderModule(ld.vk_device, vk_shader_stages[i].module, Vulkan::allocation_callbacks(this));
    }

    return pipeline_id;
}

void VulkanAdapter::pipeline_destroy(GPU::PipelineID pipeline)
{
    Pipeline& pipe = _get_pipeline(pipeline);
    LogicalDevice& ld = _get_logical_device(pipe.device);

    ld.vk.vkDestroyPipeline(pipe.vk_device, pipe.vk_pipeline, Vulkan::allocation_callbacks(this));

    pipelines.remove(pipeline);
}

GPU::CommandPoolID VulkanAdapter::command_pool_create(GPU::DeviceID device, const GPU::CommandPoolCreateInfo& ci)
{
    LogicalDevice& ld = _get_logical_device(device);
    
    GPU::CommandPoolID cmd_pool_id = command_pools.add(CommandPool());
    CommandPool& cmd_pool = _get_command_pool(cmd_pool_id);
    cmd_pool.vk_device = ld.vk_device;
    cmd_pool.device = device;

    VkCommandPoolCreateInfo vk_cmd_pool_info =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = ld.families[u32(ci.usage) - 1].vk_family_index,
    };

    VkResult result = ld.vk.vkCreateCommandPool(ld.vk_device, &vk_cmd_pool_info, Vulkan::allocation_callbacks(this), &cmd_pool.vk_command_pool);
    VKFailOn(result != VK_SUCCESS, "vkCreateCommandPool({})", Vulkan::result_as_string(result));

    return cmd_pool_id;
}

void VulkanAdapter::command_pool_destroy(GPU::CommandPoolID command_pool)
{
    CommandPool& cmd_pool = _get_command_pool(command_pool);
    LogicalDevice& ld = _get_logical_device(cmd_pool.device);

    ld.vk.vkDestroyCommandPool(cmd_pool.vk_device, cmd_pool.vk_command_pool, Vulkan::allocation_callbacks(this));

    command_pools.remove(command_pool);
}

GPU::CommandBufferID VulkanAdapter::command_buffer_allocate(const GPU::CommandBufferAllocateInfo& ci)
{
    CommandPool& cmd_pool = _get_command_pool(ci.pool);
    LogicalDevice& ld = _get_logical_device(cmd_pool.device);

    GPU::CommandBufferID cmd_buffer_id = command_buffers.add(CommandBuffer());
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

void VulkanAdapter::command_buffer_free(GPU::CommandBufferID command_buffer)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    ld.vk.vkFreeCommandBuffers(ld.vk_device, cmd_buffer.vk_command_pool, 1, &cmd_buffer.vk_command_buffer);
    
    command_buffers.remove(command_buffer);
}

void VulkanAdapter::command_buffer_begin(GPU::CommandBufferID command_buffer)
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

void VulkanAdapter::command_buffer_end(GPU::CommandBufferID command_buffer)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    VkResult result = ld.vk.vkEndCommandBuffer(cmd_buffer.vk_command_buffer);
    VKFailOn(result != VK_SUCCESS, "vkEndCommandBuffer({})", Vulkan::result_as_string(result));
}

void VulkanAdapter::command_buffer_begin_renderpass(GPU::CommandBufferID command_buffer, const GPU::RenderPassBeginInfo& begin_info)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    // render attachment and depth + stencil
    VkClearValue vk_clear_values[GPU::MaxRenderAttachmentCount + 2] = {};

    bool has_depth = begin_info.depth_attachment.texture_view.is_valid();

    u32 vk_attachment_count = static_cast<u32>(begin_info.render_attachments.len) + u32(has_depth);

    for(usize i = 0; i < begin_info.render_attachments.len; i++)
    {
        vk_clear_values[i].color =
        {
            .float32 =
            {
                begin_info.render_attachments[i].clear_value.clear_color.r,
                begin_info.render_attachments[i].clear_value.clear_color.g,
                begin_info.render_attachments[i].clear_value.clear_color.b,
                begin_info.render_attachments[i].clear_value.clear_color.a,
            }
        };
    }

    if(has_depth)
    {
        vk_clear_values[GPU::MaxRenderAttachmentCount] =
        {
            .depthStencil =
            {
                .depth = begin_info.depth_attachment.clear_value.depth_stencil.depth,
                .stencil = begin_info.depth_attachment.clear_value.depth_stencil.stencil,
            },
        };
    }

    VkRect2D vk_render_area =
    {
        .offset =
        {
            .x = begin_info.offset.x,
            .y = begin_info.offset.y,
        },
        .extent = 
        {
            .width = begin_info.extent.x,
            .height = begin_info.extent.y
        },
    };

    switch(ld.feature_level)
    {
    case FeatureLevel::Level0:
    {
        VkRenderPass vk_render_pass;
        VkFramebuffer vk_framebuffer;
        _get_render_pass_and_framebuffer_for(ld, begin_info, &vk_render_pass, &vk_framebuffer);

        // render attachment and depth + stencil
        VkImageView vk_image_views[GPU::MaxRenderAttachmentCount + 2] = {};
        for(usize i = 0; i < begin_info.render_attachments.len; i++)
        {
            vk_image_views[i] = _get_texture_view(begin_info.render_attachments[i].texture_view).vk_image_view;
        }

        VkRenderPassAttachmentBeginInfoKHR vk_attachment_begin_info =
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO_KHR,
            .pNext = nullptr,
            .attachmentCount = vk_attachment_count,
            .pAttachments = vk_image_views,
        };

        VkRenderPassBeginInfo vk_begin_info =
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = &vk_attachment_begin_info,
            .renderPass = vk_render_pass,
            .framebuffer = vk_framebuffer,
            .renderArea = vk_render_area,
            .clearValueCount = vk_attachment_count,
            .pClearValues = vk_clear_values,
        };

        VkSubpassBeginInfoKHR vk_subpass_begin_info =
        {
            .sType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO_KHR,
            .pNext = nullptr,
            .contents = VK_SUBPASS_CONTENTS_INLINE,
        };

        ld.vk.vkCmdBeginRenderPass2KHR(cmd_buffer.vk_command_buffer, &vk_begin_info, &vk_subpass_begin_info);
    }
        break;
    case FeatureLevel::Level1:
    {
        VkRenderingAttachmentInfoKHR vk_color_attachments[GPU::MaxRenderAttachmentCount] = {};
        for(usize i = 0; i < begin_info.render_attachments.len; i++)
        {
            vk_color_attachments[i] =
            {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .pNext = nullptr,
                .imageView = _get_texture_view(begin_info.render_attachments[i].texture_view).vk_image_view,
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .resolveImageView = VK_NULL_HANDLE,
                .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .loadOp = VkUtils::_vk_get_load_op(begin_info.render_attachments[i].load_op),
                .storeOp = VkUtils::_vk_get_store_op(begin_info.render_attachments[i].store_op),
                .clearValue = vk_clear_values[i],
            };
        }

        VkRenderingAttachmentInfo vk_depth_attachment = {};
        bool has_depth = begin_info.depth_attachment.texture_view.is_valid();

        if(has_depth)
        {
            vk_depth_attachment =
            {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .pNext = nullptr,
                .imageView = _get_texture_view(begin_info.depth_attachment.texture_view).vk_image_view,
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .resolveImageView = VK_NULL_HANDLE,
                .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .loadOp = VkUtils::_vk_get_load_op(begin_info.depth_attachment.load_op),
                .storeOp = VkUtils::_vk_get_store_op(begin_info.depth_attachment.store_op),
                .clearValue =
                {
                    vk_clear_values[GPU::MaxRenderAttachmentCount]
                },
            };
        }

        VkRenderingInfoKHR vk_rendering_info =
        {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .renderArea = vk_render_area,
            .layerCount = begin_info.extent.z,
            .viewMask = 0,
            .colorAttachmentCount = static_cast<u32>(begin_info.render_attachments.len),
            .pColorAttachments = vk_color_attachments,
            .pDepthAttachment = has_depth ? &vk_depth_attachment : nullptr,
            .pStencilAttachment = nullptr,
        };

        ld.vk.vkCmdBeginRenderingKHR(cmd_buffer.vk_command_buffer, &vk_rendering_info);
    }
        break;
    default:
        break;
    }
}

void VulkanAdapter::command_buffer_end_renderpass(GPU::CommandBufferID command_buffer, const GPU::RenderPassEndInfo& end_info)
{
    Unused(end_info);

    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    switch(ld.feature_level)
    {
    case FeatureLevel::Level0:
    {
        VkSubpassEndInfoKHR vk_subpass_end_info =
        {
            .sType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO_KHR,
            .pNext = nullptr,
        };

        ld.vk.vkCmdEndRenderPass2KHR(cmd_buffer.vk_command_buffer, &vk_subpass_end_info);
    }
        break;
    case FeatureLevel::Level1:
        ld.vk.vkCmdEndRenderingKHR(cmd_buffer.vk_command_buffer);
        break;
    default:
        VKFailOn(true, "invalid feature level");
        break;
    }
}

void VulkanAdapter::command_buffer_pipeline_barrier(GPU::CommandBufferID command_buffer, const GPU::PipelineBarrier& pipeline_barrier)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    VkPipelineStageFlags vk_src_stages = VkUtils::_vk_get_pipeline_stages(pipeline_barrier.src_stages);
    VkPipelineStageFlags vk_dest_stages = VkUtils::_vk_get_pipeline_stages(pipeline_barrier.dest_stages);

    Slice<VkMemoryBarrier> vk_memory_barries = allocator->array<VkMemoryBarrier>(pipeline_barrier.memory_barriers.len);
    for(usize i = 0; i < vk_memory_barries.len; i++)
    {
        vk_memory_barries[i] =
        {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VkUtils::_vk_get_access_masks(pipeline_barrier.memory_barriers[i].src_masks),
            .dstAccessMask = VkUtils::_vk_get_access_masks(pipeline_barrier.memory_barriers[i].dest_masks),
        };
    }

    Slice<VkBufferMemoryBarrier> vk_buffer_barries = allocator->array<VkBufferMemoryBarrier>(pipeline_barrier.buffer_barriers.len);
    for(usize i = 0; i < vk_buffer_barries.len; i++)
    {
        Buffer& buffer = _get_buffer(pipeline_barrier.buffer_barriers[i].buffer);
        vk_buffer_barries[i] =
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VkUtils::_vk_get_access_masks(pipeline_barrier.buffer_barriers[i].src_masks),
            .dstAccessMask = VkUtils::_vk_get_access_masks(pipeline_barrier.buffer_barriers[i].dest_masks),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = buffer.vk_buffer,
            .offset = pipeline_barrier.buffer_barriers[i].offset,
            .size = pipeline_barrier.buffer_barriers[i].size,
        };
    }

    Slice<VkImageMemoryBarrier> vk_image_barries = allocator->array<VkImageMemoryBarrier>(pipeline_barrier.texture_barriers.len);
    for(usize i = 0; i < vk_image_barries.len; i++)
    {
        Texture& tex = _get_texture(pipeline_barrier.texture_barriers[i].texture);
        vk_image_barries[i] =
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VkUtils::_vk_get_access_masks(pipeline_barrier.texture_barriers[i].src_masks),
            .dstAccessMask = VkUtils::_vk_get_access_masks(pipeline_barrier.texture_barriers[i].dest_masks),
            .oldLayout = VkUtils::_vk_get_image_layout(pipeline_barrier.texture_barriers[i].src_layout),
            .newLayout = VkUtils::_vk_get_image_layout(pipeline_barrier.texture_barriers[i].dest_layout),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = tex.vk_image,
            .subresourceRange = 
            {
                .aspectMask = VkUtils::_vk_get_aspect_masks(pipeline_barrier.texture_barriers[i].subresource_range.aspect),
                .baseMipLevel = pipeline_barrier.texture_barriers[i].subresource_range.base_mip_level,
                .levelCount = pipeline_barrier.texture_barriers[i].subresource_range.level_count,
                .baseArrayLayer = pipeline_barrier.texture_barriers[i].subresource_range.base_array_layer,
                .layerCount = pipeline_barrier.texture_barriers[i].subresource_range.layer_count,
            },
        };
    }
        
    ld.vk.vkCmdPipelineBarrier(
        cmd_buffer.vk_command_buffer, vk_src_stages, vk_dest_stages, 0,
        static_cast<uint32_t>(vk_memory_barries.len), vk_memory_barries.ptr(),
        static_cast<uint32_t>(vk_buffer_barries.len), vk_buffer_barries.ptr(),
        static_cast<uint32_t>(vk_image_barries.len), vk_image_barries.ptr()
    );
}

void VulkanAdapter::command_buffer_copy_buffer_to_texture(GPU::CommandBufferID command_buffer, const GPU::CopyBufferToTextureInfo& copy_info)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    Buffer& src_buffer = _get_buffer(copy_info.src_buffer);
    Texture& dest_texture = _get_texture(copy_info.dest_texture);

    Slice<VkBufferImageCopy> vk_regions = allocator->array<VkBufferImageCopy>(copy_info.regions.len);
    for(usize i = 0; i < vk_regions.len; i++)
    {
        vk_regions[i] =
        {
            .bufferOffset = copy_info.regions[i].buffer_offset,
            .bufferRowLength = copy_info.regions[i].buffer_row_length,
            .bufferImageHeight = copy_info.regions[i].buffer_texture_height,
            .imageSubresource =
            {
                .aspectMask = VkUtils::_vk_get_aspect_masks(copy_info.regions[i].texture_subresource_layer.aspect),
                .mipLevel = copy_info.regions[i].texture_subresource_layer.mip_level,
                .baseArrayLayer = copy_info.regions[i].texture_subresource_layer.base_array_layer,
                .layerCount = copy_info.regions[i].texture_subresource_layer.layer_count,
            },
            .imageOffset =
            {
                .x = copy_info.regions[i].texture_offset.x,
                .y = copy_info.regions[i].texture_offset.y,
                .z = copy_info.regions[i].texture_offset.z,
            },
            .imageExtent =
            {
                .width = copy_info.regions[i].texture_extent.x,
                .height = copy_info.regions[i].texture_extent.y,
                .depth = copy_info.regions[i].texture_extent.z,
            },
        };
    }

    ld.vk.vkCmdCopyBufferToImage(
        cmd_buffer.vk_command_buffer,
        src_buffer.vk_buffer, dest_texture.vk_image,
        VkUtils::_vk_get_image_layout(copy_info.dest_layout),
        static_cast<uint32_t>(vk_regions.len), vk_regions.ptr()
    );
}
    
void VulkanAdapter::command_buffer_copy_buffer(GPU::CommandBufferID command_buffer, const GPU::CopyBufferInfo& copy_info)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    Buffer& src_buffer = _get_buffer(copy_info.src_buffer);
    Buffer& dest_buffer = _get_buffer(copy_info.dest_buffer);

    Slice<VkBufferCopy> vk_regions = allocator->array<VkBufferCopy>(copy_info.copy_regions.len);
    for(usize i = 0; i < copy_info.copy_regions.len; i++)
    {
        vk_regions[i] =
        {
            .srcOffset = copy_info.copy_regions[i].src_offset,
            .dstOffset = copy_info.copy_regions[i].dest_offset,
            .size = copy_info.copy_regions[i].size,
        };
    }

    ld.vk.vkCmdCopyBuffer(
        cmd_buffer.vk_command_buffer, src_buffer.vk_buffer, dest_buffer.vk_buffer,
        static_cast<uint32_t>(vk_regions.len), vk_regions.ptr()

    );
}

void VulkanAdapter::command_buffer_bind_pipeline(GPU::CommandBufferID command_buffer, GPU::PipelineBindPoint bind_point, GPU::PipelineID pipeline)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    Pipeline& pipe = _get_pipeline(pipeline);

    ld.vk.vkCmdBindPipeline(cmd_buffer.vk_command_buffer, VkUtils::_vk_get_bind_point(bind_point), pipe.vk_pipeline);
}

void VulkanAdapter::command_buffer_bind_descriptor_sets(GPU::CommandBufferID command_buffer, GPU::PipelineBindPoint bind_point,
    GPU::PipelineLayoutID pipeline_layout, u32 base_set, const Slice<const GPU::DescriptorSetID>& descriptor_sets)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    PipelineLayout& pipe_layout = _get_pipeline_layout(pipeline_layout);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    Slice<VkDescriptorSet> vk_descriptor_sets = allocator->array<VkDescriptorSet>(descriptor_sets.len);
    for(usize i = 0; i < descriptor_sets.len; i++)
    {
        vk_descriptor_sets[i] = _get_descriptor_set(descriptor_sets[i]).vk_descriptor_set;
    }

    ld.vk.vkCmdBindDescriptorSets(
        cmd_buffer.vk_command_buffer, VkUtils::_vk_get_bind_point(bind_point), pipe_layout.vk_pipeline_layout,
        base_set, static_cast<uint32_t>(vk_descriptor_sets.len), vk_descriptor_sets.ptr(), 0, nullptr 
    );
}

void VulkanAdapter::command_buffer_bind_vertex_buffers(GPU::CommandBufferID command_buffer, u32 base_binding,
    const Slice<const GPU::BufferID>& buffers, const Slice<const usize>& offsets)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    Slice<VkBuffer> vk_buffers = allocator->array<VkBuffer>(buffers.len);
    for(usize i = 0; i < buffers.len; i++)
    {
        vk_buffers[i] = _get_buffer(buffers[i]).vk_buffer;
    }

    ld.vk.vkCmdBindVertexBuffers(
        cmd_buffer.vk_command_buffer, base_binding,
        static_cast<uint32_t>(vk_buffers.len), vk_buffers.ptr(), reinterpret_cast<const VkDeviceSize*>(offsets.ptr())
    );

    allocator->free(Mem::to_bytes(vk_buffers));
}

void VulkanAdapter::command_buffer_constant_block(GPU::CommandBufferID command_buffer, GPU::PipelineLayoutID pipeline_layout,
    GPU::ShaderStage stages, u32 offset, u32 size, MemoryAddress block_address)
{   
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    PipelineLayout& pipe_layout = _get_pipeline_layout(pipeline_layout);

    ld.vk.vkCmdPushConstants(
        cmd_buffer.vk_command_buffer, pipe_layout.vk_pipeline_layout, VkUtils::_vk_get_shader_stage(stages),
        offset, size, reinterpret_cast<void*>(block_address)
    );
}

void VulkanAdapter::command_buffer_set_viewports(GPU::CommandBufferID command_buffer, u32 base_viewport,
    const Slice<const GPU::Viewport>& viewports)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    Slice<VkViewport> vk_viewports = allocator->array<VkViewport>(viewports.len);
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

void VulkanAdapter::command_buffer_set_scissors(GPU::CommandBufferID command_buffer, u32 base_scissor, const Slice<const GPU::Scissor>& scissors)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);
    Mem::Allocator* allocator = acquire_tmp_allocator();

    Slice<VkRect2D> vk_scissors = allocator->array<VkRect2D>(scissors.len);
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

void VulkanAdapter::command_buffer_draw(GPU::CommandBufferID command_buffer, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance)
{
    CommandBuffer& cmd_buffer = _get_command_buffer(command_buffer);
    LogicalDevice& ld = _get_logical_device(cmd_buffer.device);

    ld.vk.vkCmdDraw(cmd_buffer.vk_command_buffer, vertex_count, instance_count, base_vertex, base_instance);
}

void VulkanAdapter::_get_physical_devices()
{
    uint32_t physical_device_count = 0;
    vk.vkEnumeratePhysicalDevices(
        instance, &physical_device_count, nullptr
    );

    Mem::Allocator* allocator = acquire_tmp_allocator();

    physical_devices = get_allocator()->array<PhysicalDevice>(physical_device_count);
    Slice<VkPhysicalDevice> vk_physical_devices = allocator->array<VkPhysicalDevice>(physical_device_count);
    vk.vkEnumeratePhysicalDevices(instance, &physical_device_count, vk_physical_devices.ptr());

    physical_device_ids = get_allocator()->array<GPU::PhysicalDeviceID>(physical_devices.len);
    for(usize i = 0; i < physical_device_ids.len; i++)
    {
        physical_device_ids[i] = GPU::PhysicalDeviceID(static_cast<u32>(i));
    }

    for(usize device_index = 0; device_index < physical_devices.len; device_index++)
    {
        PhysicalDevice& physical_device = physical_devices[device_index];
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

VkShaderModule VulkanAdapter::_vk_create_shader_module(LogicalDevice& ld, const GPU::ShaderStageInfo& shader_stage_info)
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

    VkResult result = ld.vk.vkCreateShaderModule(ld.vk_device, &vk_shader_module_info, Vulkan::allocation_callbacks(this), &vk_module);
    VKFailOn(result != VK_SUCCESS, "vkCreateShaderModule({})", Vulkan::result_as_string(result));

    return vk_module;
}

VulkanAdapter::RenderPassCache& VulkanAdapter::_get_render_pass_for(LogicalDevice& ld, const GPU::RenderPassBeginInfo& begin_info)
{
    VkDriverRenderPassKey render_pass_key = VkDriverRenderPassKey::from_render_pass_begin_info(this, begin_info);

    if(ld.render_pass_cache.has(render_pass_key))
    {
        return ld.render_pass_cache.get(render_pass_key);
    }

    bool has_depth = begin_info.depth_attachment.texture_view.is_valid();

    // render attachment and depth + stencil
    VkAttachmentDescription2KHR vk_attachment_descriptions[GPU::MaxRenderAttachmentCount + 2] = {};
    u32 vk_attachment_count = static_cast<u32>(begin_info.render_attachments.len) + u32(has_depth);

    for(u32 i = 0; i < begin_info.render_attachments.len; i++)
    {
        vk_attachment_descriptions[i] =
        {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR,
            .pNext = nullptr,
            .flags = 0,
            .format = VkUtils::_vk_get_texture_format(_get_texture_view(begin_info.render_attachments[i].texture_view).format),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VkUtils::_vk_get_load_op(begin_info.render_attachments[i].load_op),
            .storeOp = VkUtils::_vk_get_store_op(begin_info.render_attachments[i].store_op),
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VkUtils::_vk_get_image_layout(begin_info.render_attachments[i].layout),
            .finalLayout = VkUtils::_vk_get_image_layout(begin_info.render_attachments[i].layout),
        };
    }

    if(has_depth)
    {
        vk_attachment_descriptions[GPU::MaxRenderAttachmentCount] =
        {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR,
            .pNext = nullptr,
            .flags = 0,
            .format = VkUtils::_vk_get_texture_format(_get_texture_view(begin_info.depth_attachment.texture_view).format),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VkUtils::_vk_get_load_op(begin_info.depth_attachment.load_op),
            .storeOp = VkUtils::_vk_get_store_op(begin_info.depth_attachment.store_op),
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VkUtils::_vk_get_image_layout(begin_info.depth_attachment.layout),
            .finalLayout = VkUtils::_vk_get_image_layout(begin_info.depth_attachment.layout),
        };
    }

    VkAttachmentReference2KHR vk_color_attachments[GPU::MaxRenderAttachmentCount] = {};
    VkAttachmentReference2KHR vk_depth_attachment = {};
    for(u32 i = 0; i < begin_info.render_attachments.len; i++)
    {
        vk_color_attachments[i] =
        {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR,
            .pNext = nullptr,
            .attachment = i,
            .layout = VkUtils::_vk_get_image_layout(begin_info.render_attachments[i].layout),
            .aspectMask = 0,
        };
    }

    if(has_depth)
    {
        vk_depth_attachment =
        {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR,
            .pNext = nullptr,
            .attachment = static_cast<u32>(begin_info.render_attachments.len),
            .layout = VkUtils::_vk_get_image_layout(begin_info.depth_attachment.layout),
            .aspectMask = 0,
        };
    }

    VkSubpassDescription2KHR vk_subpass =
    {
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2_KHR,
        .pNext = nullptr,
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .viewMask = 0,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = static_cast<u32>(begin_info.render_attachments.len),
        .pColorAttachments = vk_color_attachments,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = has_depth ? &vk_depth_attachment : nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr,
    };

    VkRenderPassCreateInfo2KHR vk_render_pass_info =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = vk_attachment_count,
        .pAttachments = vk_attachment_descriptions,
        .subpassCount = 1,
        .pSubpasses = &vk_subpass,
        .dependencyCount = 0,
        .pDependencies = nullptr,
        .correlatedViewMaskCount = 0,
        .pCorrelatedViewMasks = nullptr,
    };

    RenderPassCache& render_pass_cache = ld.render_pass_cache.insert(render_pass_key, RenderPassCache());
    render_pass_cache.vk_framebuffer = VK_NULL_HANDLE;
    render_pass_cache.vk_attachment_count = vk_attachment_count;
    render_pass_cache.device = ld.device;
    ld.vk.vkCreateRenderPass2KHR(ld.vk_device, &vk_render_pass_info, Vulkan::allocation_callbacks(this), &render_pass_cache.vk_render_pass);
    
    return render_pass_cache;
}

VulkanAdapter::RenderPassCache& VulkanAdapter::_get_render_pass_for_pipeline(LogicalDevice& ld,
    const GPU::RenderingInfo& pipeline_rendering_info)
{
    VkDriverRenderPassKey render_pass_key = VkDriverRenderPassKey::from_rendering_info(pipeline_rendering_info);
    if(ld.render_pass_cache.has(render_pass_key))
    {
        return ld.render_pass_cache.get(render_pass_key);
    }

    bool has_depth = pipeline_rendering_info.depth_attachment_format != GPU::TextureFormat::Unknown;
    //bool has_stencil = pipeline_rendering_info.stencil_format != GPU::TextureFormat::Unknown;

    // render attachment and depth + stencil
    VkAttachmentDescription2KHR vk_attachment_descriptions[GPU::MaxRenderAttachmentCount + 2] = {};
    u32 vk_attachment_count = static_cast<u32>(pipeline_rendering_info.render_attachment_formats.len) + u32(has_depth);

    for(u32 i = 0; i < pipeline_rendering_info.render_attachment_formats.len; i++)
    {
        vk_attachment_descriptions[i] =
        {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR,
            .pNext = nullptr,
            .flags = 0,
            .format = VkUtils::_vk_get_texture_format(pipeline_rendering_info.render_attachment_formats[i]),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_GENERAL,
            .finalLayout = VK_IMAGE_LAYOUT_GENERAL,
        };
    }

    VkAttachmentReference2KHR vk_color_attachments[GPU::MaxRenderAttachmentCount] = {};
    VkAttachmentReference2KHR vk_depth_attachment = {};
    for(u32 i = 0; i < pipeline_rendering_info.render_attachment_formats.len; i++)
    {
        vk_color_attachments[i] =
        {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR,
            .pNext = nullptr,
            .attachment = i,
            .layout = VK_IMAGE_LAYOUT_GENERAL,
            .aspectMask = 0,
        };
    }

    if(has_depth)
    {
        vk_depth_attachment =
        {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR,
            .pNext = nullptr,
            .attachment = static_cast<u32>(pipeline_rendering_info.render_attachment_formats.len),
            .layout = VK_IMAGE_LAYOUT_GENERAL,
            .aspectMask = 0,
        };
    }

    VkSubpassDescription2KHR vk_subpass =
    {
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2_KHR,
        .pNext = nullptr,
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .viewMask = 0,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = static_cast<u32>(pipeline_rendering_info.render_attachment_formats.len),
        .pColorAttachments = vk_color_attachments,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = has_depth ? &vk_depth_attachment : nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr,
    };

    VkRenderPassCreateInfo2KHR vk_render_pass_info =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = vk_attachment_count,
        .pAttachments = vk_attachment_descriptions,
        .subpassCount = 1,
        .pSubpasses = &vk_subpass,
        .dependencyCount = 0,
        .pDependencies = nullptr,
        .correlatedViewMaskCount = 0,
        .pCorrelatedViewMasks = nullptr,
    };

    RenderPassCache& render_pass_cache = ld.render_pass_cache.insert(render_pass_key, RenderPassCache());
    render_pass_cache.vk_framebuffer = VK_NULL_HANDLE;
    render_pass_cache.vk_attachment_count = vk_attachment_count;
    render_pass_cache.device = ld.device;
    ld.vk.vkCreateRenderPass2KHR(ld.vk_device, &vk_render_pass_info, Vulkan::allocation_callbacks(this), &render_pass_cache.vk_render_pass);
    
    return render_pass_cache;
}

void VulkanAdapter::_get_render_pass_and_framebuffer_for(LogicalDevice& ld, const GPU::RenderPassBeginInfo& begin_info,
    VkRenderPass* vk_render_pass, VkFramebuffer* vk_framebuffer)
{
    RenderPassCache& render_pass_cache = _get_render_pass_for(ld, begin_info);

    if(render_pass_cache.vk_framebuffer != VK_NULL_HANDLE)
    {
        *vk_render_pass = render_pass_cache.vk_render_pass;
        *vk_framebuffer = render_pass_cache.vk_framebuffer;
        return;
    }

    bool has_depth = begin_info.depth_attachment.texture_view.is_valid();

    // render attachment and depth + stencil
    VkFramebufferAttachmentImageInfoKHR vk_framebuffer_attachment_image_infos[GPU::MaxRenderAttachmentCount + 2] = {};
    for(usize i = 0; i < begin_info.render_attachments.len; i++)
    {
        TextureView& tex = _get_texture_view(begin_info.render_attachments[i].texture_view);
        VkFormat vk_format = VkUtils::_vk_get_texture_format(tex.format);

        vk_framebuffer_attachment_image_infos[i] =
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .width = begin_info.extent.x,
            .height = begin_info.extent.y,
            .layerCount = begin_info.extent.z,
            .viewFormatCount = 1,
            .pViewFormats = &vk_format,
        };
    }

    if(has_depth)
    {
        TextureView& tex = _get_texture_view(begin_info.depth_attachment.texture_view);
        VkFormat vk_format = VkUtils::_vk_get_texture_format(tex.format);
        
        vk_framebuffer_attachment_image_infos[begin_info.render_attachments.len] =
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .width = begin_info.extent.x,
            .height = begin_info.extent.y,
            .layerCount = begin_info.extent.z,
            .viewFormatCount = 1,
            .pViewFormats = &vk_format,
        };
    }    

    VkFramebufferAttachmentsCreateInfoKHR vk_framebuffer_attachments_info =
    {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO_KHR,
        .pNext = nullptr,
        .attachmentImageInfoCount = render_pass_cache.vk_attachment_count,
        .pAttachmentImageInfos = vk_framebuffer_attachment_image_infos,
    };

    VkFramebufferCreateInfo vk_framebuffer_info =
    {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = &vk_framebuffer_attachments_info,
        .flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR,
        .renderPass = render_pass_cache.vk_render_pass,
        .attachmentCount = render_pass_cache.vk_attachment_count,
        .pAttachments = nullptr,
        .width = begin_info.extent.x,
        .height = begin_info.extent.y,
        .layers = begin_info.extent.z,
    };

    ld.vk.vkCreateFramebuffer(ld.vk_device, &vk_framebuffer_info, Vulkan::allocation_callbacks(this), vk_framebuffer);
    render_pass_cache.vk_framebuffer = *vk_framebuffer;

    *vk_render_pass = render_pass_cache.vk_render_pass;
}

GPU::DeviceType VulkanAdapter::_vk_device_type_to_device_type(VkPhysicalDeviceType vk_device_type)
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

GPU::PresentMode VulkanAdapter::_vk_present_mode_to_present_mode(VkPresentModeKHR vk_present_mode)
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

GPU::HeapUsage VulkanAdapter::_vk_memory_property_to_heap_usage(VkMemoryPropertyFlags vk_memory_properties)
{
    VkMemoryPropertyFlags cpu_exclusive = VkUtils::_vk_get_memory_properties(GPU::HeapUsage::CPUExclusive);
    VkMemoryPropertyFlags gpu_exclusive = VkUtils::_vk_get_memory_properties(GPU::HeapUsage::GPUExclusive);
    VkMemoryPropertyFlags cpu_gpu_coherent = VkUtils::_vk_get_memory_properties(GPU::HeapUsage::CPUGPUCoherent);

    if(HasValue(vk_memory_properties & cpu_exclusive))
    {
        return GPU::HeapUsage::CPUExclusive;
    }
    if(HasValue(vk_memory_properties & gpu_exclusive))
    {
        return GPU::HeapUsage::GPUExclusive;
    }
    if(HasValue(vk_memory_properties & cpu_gpu_coherent))
    {
        return GPU::HeapUsage::CPUGPUCoherent;
    }

    VKFailOn(true, "invalid vulkan memory properties");

}

uint32_t VulkanAdapter::_get_queue_family_for(const Slice<VkQueueFamilyProperties2>& vk_families,
    const Slice<u32>& acquired, VkQueueFlags vk_queue_flags)
{
    uint32_t best_match_index = MaxValue<uint32_t>;

    for(usize i = 0; i < vk_families.len; i++)
    {
        if(acquired[i] != 0)
        {
            continue;
        }

        bool exact_match = (vk_families[i].queueFamilyProperties.queueFlags & vk_queue_flags) == vk_queue_flags;
        
        if(exact_match && acquired[i] == 0)
        {
            best_match_index = i;
        }
    }

    return best_match_index;
}

uint32_t VulkanAdapter::_get_queue_family_for_present(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface,
    const Slice<VkQueueFamilyProperties2>& vk_families)
{
    uint32_t best_match_index = MaxValue<uint32_t>;
    for(usize i = 0; i < vk_families.len; i++)
    {
        VkBool32 supported = VK_FALSE;
        vk.vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device, i, vk_surface, &supported);

        if(supported == VK_TRUE)
        {
            best_match_index = i;
        }
    }

    return best_match_index;
}

void VulkanAdapter::_vk_get_surface_format(GPU::TextureFormat surface_format, VkFormat* vk_image_format, VkColorSpaceKHR* vk_color_space)
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

VkSurfaceCapabilitiesKHR VulkanAdapter::_vk_get_surface_capabilities(VkPhysicalDevice vk_physical_device, VkSurfaceKHR vk_surface)
{
    VkSurfaceCapabilitiesKHR capabilities;
    vk.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, vk_surface, &capabilities);
    return capabilities;   
}

VkExtent2D VulkanAdapter::_vk_get_swap_chain_extent(const Vector2U& size, const VkSurfaceCapabilitiesKHR& vk_capabilities)
{
    if(vk_capabilities.currentExtent.width != MaxValue<uint32_t>)
    {
        return vk_capabilities.currentExtent;
    }
    
    VkExtent2D vk_extent = {};
    vk_extent.width = Math::clamp(size.width, vk_capabilities.minImageExtent.width, vk_capabilities.maxImageExtent.width);
    vk_extent.height = Math::clamp(size.height, vk_capabilities.minImageExtent.height, vk_capabilities.maxImageExtent.height);
    return vk_extent;
}
