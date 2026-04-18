#include "gpu/vk/vk_header.h"

#include "gpu/vk/vk_driver.h"
#include "gpu/vk/vk_vtable.h"
#include "mem/utils.h"
#include "os/os.h"



static constexpr const char* _vk_extensions[] =
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


VkAllocationCallbacks* Vulkan::allocation_callbacks()
{
    static VkAllocationCallbacks _vk_allocation_callbacks =
    {
        .pUserData = nullptr,
        .pfnAllocation = &_vk_driver_allocate,
        .pfnReallocation = &_vk_driver_reallocate,
        .pfnFree = &_vk_driver_free,
        .pfnInternalAllocation = &_vk_driver_internal_allocate,
        .pfnInternalFree = &_vk_driver_internal_free,
    };
    return &_vk_allocation_callbacks;
}

void Vulkan::load_core_procs(MemoryAddress vk_lib)
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
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateBufferView);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyBufferView);

    // sampler
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateSampler);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroySampler);

    // image
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateImage);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyImage);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkBindImageMemory2)

    // image view
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateImageView);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyImageView);

    // framebuffer
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateFramebuffer);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyFramebuffer);

    // descriptors
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateDescriptorPool);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyDescriptorPool);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkAllocateDescriptorSets);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkFreeDescriptorSets);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkUpdateDescriptorSets);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateDescriptorSetLayout);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyDescriptorSetLayout);

    // render pass
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateRenderPass);
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
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdBeginRenderPass);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdEndRenderPass);

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

VkInstance Vulkan::create_instance()
{
    _check_instance_extensions();

    VkApplicationInfo application_info =
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = VkApplicationName.ptr(),
        .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .pEngineName = VkEngineName.ptr(),
        .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .apiVersion = get_api_version(),
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
    const char* vk_layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };
#endif

    VkLayerSettingsCreateInfoEXT layer_settings_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
        .pNext = nullptr,
        .settingCount = static_cast<uint32_t>(ArraySize(layer_settings)),
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
        .enabledLayerCount = static_cast<uint32_t>(ArraySize(vk_layers)),
        .ppEnabledLayerNames = vk_layers,
#else
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
#endif
        .enabledExtensionCount = static_cast<uint32_t>(ArraySize(_vk_extensions)),
        .ppEnabledExtensionNames = _vk_extensions,
    };

    VkInstance instance;
    {
        VkResult result = vk.vkCreateInstance(&instance_info, Vulkan::allocation_callbacks(), &instance);
        VKFailOn(result != VK_SUCCESS, "vkCreateInstance({})", Vulkan::result_as_string(result));
    }

    return instance;
}

VkSurfaceKHR Vulkan::create_surface(VkInstance instance, MemoryAddress native_handle)
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
        if(initialized_dummy_class == false)
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
    
    VkResult result = vk.vkCreateWin32SurfaceKHR(instance, &vk_surface_info, allocation_callbacks(), &vk_surface);
#elif defined(BREAD_ANDROID)
    VkAndroidSurfaceCreateInfoKHR vk_surface_info =
    {
        .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .window = reinterpret_cast<ANativeWindow*>(native_handle),
    };

    VkResult result = vk.vkCreateAndroidSurfaceKHR(instance, &vk_surface_info, allocation_callbacks(), &vk_surface);
#endif

    VKFailOn(result != VK_SUCCESS, "vkCreateSurfaceKHR({})", Vulkan::result_as_string(result));

    return vk_surface;
}

void Vulkan::destroy_surface(VkInstance instance, VkSurfaceKHR surface)
{
    vk.vkDestroySurfaceKHR(instance, surface, allocation_callbacks());
}

Vulkan::AdditionalExtensionSupport Vulkan::check_device_extensions(VkPhysicalDevice physical_device)
{
    mem::Allocator allocator = VulkanDriver::get_allocator();

    u32 extension_count;
    vk.vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
    
    Slice<VkExtensionProperties> vk_device_extensions = allocator.array<VkExtensionProperties>(extension_count);
    vk.vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, vk_device_extensions.ptr());

    usize finded_count = 0;
    for (const char* ext : VkCoreDeviceExtensions)
    {
        StringView ext_view = Vulkan::vulkan_string_to_sv(ext);
        bool finded = false;
        for (VkExtensionProperties& act_ext : vk_device_extensions)
        {
            StringView reported_ext = Vulkan::vulkan_string_to_sv(act_ext.extensionName);
            if (ext_view.equals(reported_ext))
            {
                finded = true;
                break;
            }
        }

        if(finded)
        {
            finded_count++;
        }
    }

    VKFailOn(
        finded_count != ArraySize(VkCoreDeviceExtensions),
        "the required extensions were not found"
    );

    allocator.free(mem::to_bytes(vk_device_extensions));

    AdditionalExtensionSupport additional_extension_support = {};
    additional_extension_support.has_dynamic_rendering = _has_extension(vk_device_extensions, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    additional_extension_support.has_imageless_framebuffer = _has_extension(vk_device_extensions, VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);

    return additional_extension_support;
}

void Vulkan::check_device_features(VkPhysicalDevice physical_device)
{
    VkPhysicalDeviceShaderFloat16Int8FeaturesKHR vk_float16_features = {};
    vk_float16_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR;
    vk_float16_features.pNext = nullptr;

    VkPhysicalDeviceVulkan11Features vk_1_1_features = {};
    vk_1_1_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    vk_1_1_features.pNext = &vk_float16_features;

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

const char** Vulkan::get_device_extensions(VkPhysicalDevice physical_device, const AdditionalExtensionSupport& add_ext, 
    const mem::Allocator& allocator)
{
    Unused(physical_device);
    usize additional_extension_count = 0;
    if(add_ext.has_dynamic_rendering)
    {
        additional_extension_count++;
    }
    if(add_ext.has_imageless_framebuffer)
    {
        additional_extension_count++;
    }

    Slice<const char*> extensions = allocator.array<const char*>(ArraySize(VkCoreDeviceExtensions) + additional_extension_count);
    for(usize i = 0; i < ArraySize(VkCoreDeviceExtensions); i++)
    {
        extensions[i] = VkCoreDeviceExtensions[i];
    }
    usize index = ArraySize(VkCoreDeviceExtensions);
    if(add_ext.has_dynamic_rendering)
    {
        extensions[index++] = VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME;
    }
    if(add_ext.has_imageless_framebuffer)
    {
        extensions[index++] = VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME;
    }

    return extensions.ptr();
}

VkPhysicalDeviceFeatures2* Vulkan::get_device_features(const AdditionalExtensionSupport& add_ext, const mem::Allocator& allocator)
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

    VkPhysicalDeviceVulkan11Features* vk_1_1_features =
        allocator.object<VkPhysicalDeviceVulkan11Features>();
    vk_1_1_features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    vk_1_1_features->pNext = vk_float16_features;
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
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	Unused(messageSeverity, messageTypes, pUserData);
    StringView msg_view = Vulkan::vulkan_string_to_sv(pCallbackData->pMessage);
    VKDebugInfo("{}", msg_view);
 	return VK_FALSE;
}

void* VKAPI_PTR Vulkan::_vk_driver_allocate(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	Unused(pUserData, size, alignment, allocationScope);

	mem::Allocator allocator = VulkanDriver::get_allocator();
	Slice<u8> bytes = allocator.alloc(size, alignment);
	return bytes.ptr();
}

void* VKAPI_PTR Vulkan::_vk_driver_reallocate(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	Unused(pUserData, pOriginal, size, alignment, allocationScope);

	mem::Allocator allocator = VulkanDriver::get_allocator();
    Slice<u8> old_mem = Slice(reinterpret_cast<u8*>(pOriginal), 1);

    if(pOriginal == nullptr)
    {
        return _vk_driver_allocate(pUserData, size, alignment, allocationScope);
    }

    if(size == 0)
    {
        _vk_driver_free(pUserData, pOriginal);
        return nullptr;
    }

    usize old_size = allocator.get_size_of(old_mem);
	bool realloc_result = allocator.realloc(
		old_mem, size, alignment
	);

	if (realloc_result)
	{
		return old_mem.ptr();
	}

	Slice<u8> bytes = allocator.alloc(size, alignment);
    mem::copy(bytes, Slice<const u8>(reinterpret_cast<const u8*>(pOriginal), old_size));
    allocator.free(old_mem);
	return bytes.ptr();
}

void VKAPI_PTR Vulkan::_vk_driver_free(void* pUserData, void* pMemory)
{
	Unused(pUserData, pMemory);
	
	if (pMemory == nullptr)
		return;

	mem::Allocator allocator = VulkanDriver::get_allocator();
	Slice<u8> old_mem = Slice(reinterpret_cast<u8*>(pMemory), 1);
	allocator.free(old_mem);
}

void VKAPI_PTR Vulkan::_vk_driver_internal_allocate(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{
    VKDebugInfo("driver allocated {} bytes", size);
	Unused(pUserData, size, allocationType, allocationScope);
}

void VKAPI_PTR Vulkan::_vk_driver_internal_free(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{
    VKDebugInfo("driver deletes {} bytes", size);
	Unused(pUserData, size, allocationType, allocationScope);
}

void Vulkan::_check_instance_extensions()
{
    uint32_t extension_count = 0;
    vk.vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    mem::Allocator allocator = VulkanDriver::get_allocator();

    Slice<VkExtensionProperties> instance_extensions = allocator.array<VkExtensionProperties>(extension_count);
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

    for (const char* ext : _vk_extensions)
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

    allocator.free(mem::to_bytes(instance_extensions));
}

