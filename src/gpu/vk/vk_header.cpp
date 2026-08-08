#include "gpu/vk/vk_header.h"

#include "gpu/vk/vk_adapter.h"
#include "gpu/vk/vk_vtable.h"
#include "mem/utils.h"
#include "os/os.h"
#include "platform/platform_header.h"


static constexpr const char* VkInstanceExtensions[] =
{
#if defined(BREAD_SHOW_DEBUG_INFO) && defined(BREAD_WIN32)
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
    VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(BREAD_WIN32)
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(BREAD_ANDROID)
    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#endif
};

static inline VkAllocationCallbacks vk_allocation_callbacks =
{
    .pUserData = nullptr,
    .pfnAllocation = &Vulkan::_vk_driver_allocate,
    .pfnReallocation = &Vulkan::_vk_driver_reallocate,
    .pfnFree = &Vulkan::_vk_driver_free,
    .pfnInternalAllocation = &Vulkan::_vk_driver_internal_allocate,
    .pfnInternalFree = &Vulkan::_vk_driver_internal_free,
};

VkAllocationCallbacks* Vulkan::allocation_callbacks(VulkanAdapter* adapter)
{
    vk_allocation_callbacks.pUserData = adapter;
    return nullptr;
}

void Vulkan::load_core_procs(OS::Handle vk_lib)
{
    VK_REQUIRED_LOAD(vkCreateInstance, vk_lib);
    VK_REQUIRED_LOAD(vkDestroyInstance, vk_lib);
    VK_REQUIRED_LOAD(vkEnumerateInstanceVersion, vk_lib);
    VK_REQUIRED_LOAD(vkEnumerateInstanceLayerProperties, vk_lib);
    VK_REQUIRED_LOAD(vkEnumerateInstanceExtensionProperties, vk_lib);
    VK_REQUIRED_LOAD(vkGetInstanceProcAddr, vk_lib);
}

void Vulkan::load_instance_procs(VkInstance instance)
{
    // Instance procs
    VK_INSTANCE_LOAD(instance, vkCreateDebugUtilsMessengerEXT);
    VK_INSTANCE_LOAD(instance, vkDestroyDebugUtilsMessengerEXT);

    // physical device
    VK_INSTANCE_REQUIRED_LOAD(instance, vkEnumeratePhysicalDevices);
    VK_INSTANCE_REQUIRED_LOAD(instance, vkEnumerateDeviceExtensionProperties);
    VK_INSTANCE_REQUIRED_LOAD(instance, vkGetPhysicalDeviceFeatures2);
    VK_INSTANCE_REQUIRED_LOAD(instance, vkGetPhysicalDeviceProperties2);
    VK_INSTANCE_REQUIRED_LOAD(instance, vkGetPhysicalDeviceQueueFamilyProperties2);
    VK_INSTANCE_REQUIRED_LOAD(instance, vkGetPhysicalDeviceMemoryProperties2);

    VK_INSTANCE_REQUIRED_LOAD(instance, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    VK_INSTANCE_REQUIRED_LOAD(instance, vkGetPhysicalDeviceSurfaceFormatsKHR);
    VK_INSTANCE_REQUIRED_LOAD(instance, vkGetPhysicalDeviceSurfacePresentModesKHR);
    VK_INSTANCE_REQUIRED_LOAD(instance, vkGetPhysicalDeviceSurfaceSupportKHR);

    // surface
#if defined(BREAD_WIN32)
    VK_INSTANCE_REQUIRED_LOAD(instance, vkCreateWin32SurfaceKHR);
#elif defined(BREAD_ANDROID)
    VK_INSTANCE_REQUIRED_LOAD(instance, vkCreateAndroidSurfaceKHR);
#endif
    VK_INSTANCE_REQUIRED_LOAD(instance, vkDestroySurfaceKHR);
    
    // device
    VK_INSTANCE_REQUIRED_LOAD(instance, vkCreateDevice);
    VK_INSTANCE_REQUIRED_LOAD(instance, vkDestroyDevice);
    VK_INSTANCE_REQUIRED_LOAD(instance, vkGetDeviceProcAddr);
}

void Vulkan::load_device_procs(DeviceVulkanTable& table, VkDevice device)
{
    // swapchain
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateSwapchainKHR);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroySwapchainKHR);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkGetSwapchainImagesKHR);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkAcquireNextImageKHR);

    // queue
    VK_DEVICE_REQUIRED_LOAD(table, device, vkGetDeviceQueue);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkQueuePresentKHR);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkQueueSubmit);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkQueueWaitIdle);

    // fence
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateFence);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyFence);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkGetFenceStatus);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkResetFences);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkWaitForFences);

    // semaphore
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateSemaphore);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroySemaphore);

    // memory
    VK_DEVICE_REQUIRED_LOAD(table, device, vkAllocateMemory);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkFreeMemory);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkMapMemory);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkUnmapMemory);

    // buffer
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateBuffer);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyBuffer);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkBindBufferMemory2);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkGetBufferMemoryRequirements2);

    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateBufferView);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyBufferView);

    // sampler
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateSampler);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroySampler);

    // image
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateImage);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyImage);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkBindImageMemory2);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkGetImageMemoryRequirements2);

    // image view
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateImageView);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyImageView);

    // framebuffer
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateFramebuffer);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyFramebuffer);

    // descriptors
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateDescriptorPool);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyDescriptorPool);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkResetDescriptorPool);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkAllocateDescriptorSets);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkFreeDescriptorSets);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkUpdateDescriptorSets);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateDescriptorSetLayout);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyDescriptorSetLayout);

    // render pass
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateRenderPass2KHR);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyRenderPass);

    // pipeline
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateGraphicsPipelines);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyPipeline);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateShaderModule);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyShaderModule);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreatePipelineLayout);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyPipelineLayout);

    // command pool
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateCommandPool);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyCommandPool);

    // command buffer
    VK_DEVICE_REQUIRED_LOAD(table, device, vkAllocateCommandBuffers);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkFreeCommandBuffers);

    // commands
    VK_DEVICE_REQUIRED_LOAD(table, device, vkBeginCommandBuffer);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkEndCommandBuffer);

    // vk_khr_create_renderpass2
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdBeginRenderPass2KHR);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdEndRenderPass2KHR);

    // vk_khr_dynamic_rendering
    VK_DEVICE_TRY_LOAD(table, device, vkCmdBeginRenderingKHR, vkCmdBeginRenderingKHR);
    VK_DEVICE_TRY_LOAD(table, device, vkCmdBeginRenderingKHR, vkCmdBeginRendering);
    VK_DEVICE_TRY_LOAD(table, device, vkCmdEndRenderingKHR, vkCmdEndRenderingKHR);
    VK_DEVICE_TRY_LOAD(table, device, vkCmdEndRenderingKHR, vkCmdEndRendering);

    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdPipelineBarrier);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdCopyBufferToImage);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdCopyBuffer);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdBindPipeline);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdBindDescriptorSets);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdBindVertexBuffers);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdPushConstants);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdSetViewport);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdSetScissor);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdDraw);
}

uint32_t Vulkan::get_api_version()
{
    uint32_t api_version = 0;
    vk.vkEnumerateInstanceVersion(&api_version);
    return api_version;
}

VkInstance Vulkan::create_instance(VulkanAdapter* adapter)
{
    _check_instance_extensions(adapter->get_allocator());

    VkApplicationInfo application_info =
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = VkApplicationName.ptr(),
        .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .pEngineName = VkEngineName.ptr(),
        .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .apiVersion = VK_MAKE_API_VERSION(0, 1, 1, 0),
    };

    // Validation layer
#if defined(BREAD_SHOW_DEBUG_INFO)
    const VkBool32 verbose_value = VK_TRUE;
    const VkBool32 gpu_validation = VK_TRUE;

    const VkLayerSettingEXT layer_settings[] =
    {
        {
            .pLayerName = "VK_LAYER_KHRONOS_validation",
            .pSettingName = "printf_verbose",
            .type = VK_LAYER_SETTING_TYPE_BOOL32_EXT,
            .valueCount = 1,
            .pValues = &verbose_value,
        },
        {
            .pLayerName = "VK_LAYER_KHRONOS_validation",
            .pSettingName = "validate_gpu_based",
            .type = VK_LAYER_SETTING_TYPE_BOOL32_EXT,
            .valueCount = 1,
            .pValues = &gpu_validation
        }
    };

#if defined(BREAD_SHOW_DEBUG_INFO) && defined(BREAD_WIN32)
    const char* vk_layers[] =
    {
        "VK_LAYER_KHRONOS_validation"
    };
#endif

    VkLayerSettingsCreateInfoEXT layer_settings_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
        .pNext = nullptr,
        .settingCount = static_cast<uint32_t>(Core::ArraySize(layer_settings)),
        .pSettings = layer_settings,
    };
    (void)layer_settings_create_info;
#endif

    VkInstanceCreateInfo instance_info =
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#if defined(BREAD_SHOW_DEBUG_INFO)
        .pNext = &layer_settings_create_info,
#else
        .pNext = nullptr,
#endif
        .flags = 0,
        .pApplicationInfo = &application_info,
#if defined(BREAD_SHOW_DEBUG_INFO) && defined(BREAD_WIN32)
        .enabledLayerCount = static_cast<uint32_t>(Core::ArraySize(vk_layers)),
        .ppEnabledLayerNames = vk_layers,
#else
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
#endif
        .enabledExtensionCount = static_cast<uint32_t>(Core::ArraySize(VkInstanceExtensions)),
        .ppEnabledExtensionNames = VkInstanceExtensions,
    };

    VkInstance instance = VK_NULL_HANDLE;
    {
        VkResult result = vk.vkCreateInstance(&instance_info, Vulkan::allocation_callbacks(adapter), &instance);
        VKFailOn(result != VK_SUCCESS, "vkCreateInstance({})", Vulkan::result_as_string(result));
    }

    return instance;
}

VkSurfaceKHR Vulkan::create_surface(VulkanAdapter* adapter, VkInstance instance, MemoryAddress native_handle)
{
    VkSurfaceKHR vk_surface = VK_NULL_HANDLE;

#if defined(BREAD_WIN32)
    VkWin32SurfaceCreateInfoKHR vk_surface_info =
    {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .hinstance = GetModuleHandleA(nullptr),
        .hwnd = HWND(native_handle),
    };

    if(native_handle == 0)
    {
        static bool initialized_dummy_class = false;
        if(!initialized_dummy_class)
        {
            WNDCLASS wc = {};
            wc.lpfnWndProc = &DefWindowProcA;
            wc.lpszClassName = "Bread:vulkan_dummy";

            RegisterClassA(&wc);
        }
        
        vk_surface_info.hwnd = CreateWindowExA(
            0, "Bread:vulkan_dummy", "-", WS_OVERLAPPEDWINDOW,
            0, 0, 200, 200, 0, 0, 0, 0
        );
    }
    
    VkResult result = vk.vkCreateWin32SurfaceKHR(instance, &vk_surface_info, allocation_callbacks(adapter), &vk_surface);
#elif defined(BREAD_ANDROID)
    VkAndroidSurfaceCreateInfoKHR vk_surface_info =
    {
        .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .window = reinterpret_cast<ANativeWindow*>(native_handle),
    };

    VkResult result = vk.vkCreateAndroidSurfaceKHR(instance, &vk_surface_info, allocation_callbacks(adapter), &vk_surface);
#endif

    VKFailOn(result != VK_SUCCESS, "vkCreateSurfaceKHR({})", Vulkan::result_as_string(result));

    return vk_surface;
}

Vulkan::AdditionalExtensionSupport Vulkan::check_device_extensions(Mem::Allocator& allocator, VkPhysicalDevice physical_device)
{
    u32 extension_count;
    vk.vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
    
    Slice vk_device_extensions = allocator.array<VkExtensionProperties>(extension_count);
    vk.vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, vk_device_extensions.ptr());

    usize finded_count = 0;
    for (const char* ext : VkCoreDeviceExtensions)
    {
        bool finded = _has_extension(vk_device_extensions, ext);

        if(finded)
        {
            finded_count++;
        }
    }

    VKFailOn(
        finded_count != Core::ArraySize(VkCoreDeviceExtensions),
        "the required extensions were not found"
    );

    AdditionalExtensionSupport additional_extension_support = {};
    additional_extension_support.has_dynamic_rendering = _has_extension(vk_device_extensions, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    
    allocator.free(Mem::to_bytes(vk_device_extensions));
    return additional_extension_support;
}

void Vulkan::check_device_features(VkPhysicalDevice physical_device)
{
    VkPhysicalDeviceShaderFloat16Int8FeaturesKHR vk_float16_features = {};
    vk_float16_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR;
    vk_float16_features.pNext = nullptr;

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR vk_imageless_framebuffer = {};
    vk_imageless_framebuffer.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES_KHR;
    vk_imageless_framebuffer.pNext = &vk_float16_features;

    VkPhysicalDeviceVulkan11Features vk_1_1_features = {};
    vk_1_1_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    vk_1_1_features.pNext = &vk_imageless_framebuffer;

    VkPhysicalDeviceFeatures2 vk_features =
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &vk_1_1_features,
        .features = {},
    };

    vk.vkGetPhysicalDeviceFeatures2(physical_device, &vk_features);

    VKFailOn(vk_float16_features.shaderFloat16 == VK_FALSE,
        "VkPhysicalDeviceShaderFloat16Int8FeaturesKHR::shaderFloat16 was required"
    );
    
    VKFailOn(vk_1_1_features.shaderDrawParameters == VK_FALSE,
        "VkPhysicalDeviceVulkan11Features::shaderDrawParameters was required"
    );
    VKFailOn(vk_1_1_features.storageInputOutput16 == VK_FALSE,
        "VkPhysicalDeviceVulkan11Features::storageInputOutput16 was required"
    );

    VKFailOn(vk_features.features.samplerAnisotropy == VK_FALSE,
        "VkPhysicalDeviceFeatures2::samplerAnisotropy was required"
    );
}

const char** Vulkan::get_device_extensions(Mem::Allocator& allocator, [[maybe_unused]] VkPhysicalDevice physical_device,
    const AdditionalExtensionSupport& add_ext, uint32_t* extension_count)
{
    usize additional_extension_count = 0;
    if(add_ext.has_dynamic_rendering)
    {
        additional_extension_count += 3;
    }

    Slice extensions = allocator.array<const char*>(Core::ArraySize(VkCoreDeviceExtensions) + additional_extension_count);
    for(usize i = 0; i < Core::ArraySize(VkCoreDeviceExtensions); i++)
    {
        extensions[i] = VkCoreDeviceExtensions[i];
    }
    usize index = Core::ArraySize(VkCoreDeviceExtensions);
    if(add_ext.has_dynamic_rendering)
    {
        extensions[index++] = VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME;
        extensions[index++] = VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME;
    }

    *extension_count = index;
    return extensions.ptr();
}

VkPhysicalDeviceFeatures2* Vulkan::get_device_features(Mem::Allocator& allocator, const AdditionalExtensionSupport& add_ext)
{
    VkPhysicalDeviceDynamicRenderingFeaturesKHR* vk_dynamic_rendering_features = nullptr;
    if(add_ext.has_dynamic_rendering)
    {
        vk_dynamic_rendering_features =
            allocator.object<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
        vk_dynamic_rendering_features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
        vk_dynamic_rendering_features->dynamicRendering = VK_TRUE;
    }

    VkPhysicalDeviceShaderFloat16Int8FeaturesKHR* vk_float16_features =
        allocator.object<VkPhysicalDeviceShaderFloat16Int8FeaturesKHR>();
    vk_float16_features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR;
    vk_float16_features->pNext = vk_dynamic_rendering_features;
    vk_float16_features->shaderFloat16 = VK_TRUE;

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR* vk_imageless_framebuffer =
        allocator.object<VkPhysicalDeviceImagelessFramebufferFeaturesKHR>();
    vk_imageless_framebuffer->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES_KHR;
    vk_imageless_framebuffer->pNext = vk_float16_features;
    vk_imageless_framebuffer->imagelessFramebuffer = VK_TRUE;

    VkPhysicalDeviceVulkan11Features* vk_1_1_features =
        allocator.object<VkPhysicalDeviceVulkan11Features>();
    vk_1_1_features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    vk_1_1_features->pNext = vk_imageless_framebuffer;
    vk_1_1_features->shaderDrawParameters = VK_TRUE;
    vk_1_1_features->storageInputOutput16 = VK_TRUE;

    VkPhysicalDeviceFeatures2* vk_features =
        allocator.object<VkPhysicalDeviceFeatures2>();

    vk_features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    vk_features->pNext = vk_1_1_features;
    vk_features->features.samplerAnisotropy = VK_TRUE;

    return vk_features;
}

bool Vulkan::_has_extension(const Slice<VkExtensionProperties>& vk_device_extensions, const char* ext_name)
{
    for(VkExtensionProperties& ext : vk_device_extensions)
    {
        if(Vulkan::vulkan_string_to_sv(ext.extensionName).equals(Vulkan::vulkan_string_to_sv(ext_name)))
        {
            return true;
        }
    }
    return false;
}

VkBool32 VKAPI_PTR Vulkan::_vk_debug_utils_callback(
	[[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	[[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	[[maybe_unused]] void* pUserData)
{
    StringView msg_view = Vulkan::vulkan_string_to_sv(pCallbackData->pMessage);
    VKDebugInfo("{}", msg_view);
 	return VK_FALSE;
}

void* VKAPI_PTR Vulkan::_vk_driver_allocate(void* pUserData, size_t size, size_t alignment,
    [[maybe_unused]] VkSystemAllocationScope allocationScope)
{
    if(size == 0)
    {
        return nullptr;
    }

    VulkanAdapter* vulkan_adapter = reinterpret_cast<VulkanAdapter*>(pUserData);
	Mem::Allocator& allocator = vulkan_adapter->get_allocator();
	Mutex& mutex = vulkan_adapter->get_allocator_mutex();
    OSMutexAuto(&mutex);

    Slice bytes = allocator.alloc(size, alignment);
    return bytes.ptr();
}

void* VKAPI_PTR Vulkan::_vk_driver_reallocate(void* pUserData, void* pOriginal, size_t size, size_t alignment,
    [[maybe_unused]] VkSystemAllocationScope allocationScope)
{
    if(pOriginal == nullptr)
    {
        return _vk_driver_allocate(pUserData, size, alignment, allocationScope);
    }

    if(size == 0)
    {
        _vk_driver_free(pUserData, pOriginal);
        return nullptr;
    }

    VulkanAdapter* vulkan_adapter = reinterpret_cast<VulkanAdapter*>(pUserData);
	Mem::Allocator& allocator = vulkan_adapter->get_allocator();
	Mutex& mutex = vulkan_adapter->get_allocator_mutex();
    OSMutexAuto(&mutex);

    Slice new_mem = allocator.remap(Slice<u8>(reinterpret_cast<u8*>(pOriginal), 1), size, alignment);
    return new_mem.ptr();
}

void VKAPI_PTR Vulkan::_vk_driver_free([[maybe_unused]] void* pUserData, void* pMemory)
{
	if(pMemory == nullptr)
	{
        return;
    }

    VulkanAdapter* vulkan_adapter = reinterpret_cast<VulkanAdapter*>(pUserData);
	Mem::Allocator& allocator = vulkan_adapter->get_allocator();
	Mutex& mutex = vulkan_adapter->get_allocator_mutex();
    OSMutexAuto(&mutex);

    allocator.free(Slice<u8>(reinterpret_cast<u8*>(pMemory), 1));
}

void VKAPI_PTR Vulkan::_vk_driver_internal_allocate([[maybe_unused]] void* pUserData, [[maybe_unused]] size_t size,
    [[maybe_unused]] VkInternalAllocationType allocationType, [[maybe_unused]] VkSystemAllocationScope allocationScope)
{
    VKDebugInfo("driver allocated {} bytes", size);
}

void VKAPI_PTR Vulkan::_vk_driver_internal_free([[maybe_unused]] void* pUserData, [[maybe_unused]] size_t size,
    [[maybe_unused]] VkInternalAllocationType allocationType, [[maybe_unused]] VkSystemAllocationScope allocationScope)
{
    VKDebugInfo("driver deletes {} bytes", size);
}

void Vulkan::_check_instance_extensions(Mem::Allocator& allocator)
{
    uint32_t extension_count = 0;
    vk.vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    Slice instance_extensions = allocator.array<VkExtensionProperties>(extension_count);
    vk.vkEnumerateInstanceExtensionProperties(
        nullptr, &extension_count, instance_extensions.ptr()
    );

    VKDebugInfo("Vulkan Extensions");
    for (VkExtensionProperties& extension : instance_extensions)
    {
        VKDebugInfo("{}",
            Vulkan::vulkan_string_to_sv(extension.extensionName)
        );
    }

    // Checking required extensions

    for (const char* ext : VkInstanceExtensions)
    {
        bool finded = false;
        StringView ext_view = Vulkan::vulkan_string_to_sv(ext);
        for (VkExtensionProperties& act_ext : instance_extensions)
        {
            if (ext_view.equals(Vulkan::vulkan_string_to_sv(act_ext.extensionName)))
            {
                finded = true;
                break;
            }
        }

        VKFailOn(finded == false,
            "{} was required", ext_view
        );
    }

    allocator.free(Mem::to_bytes(instance_extensions));
}

