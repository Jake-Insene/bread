#include "graphics/vk/vk_header.h"

#include "graphics/vk/vk_driver.h"
#include "graphics/vk/vk_vtable.h"
#include "mem/utils.h"
#include "os/os.h"



static constexpr const char* _vk_extensions[] =
{
#if defined(BREAD_SHOW_DEBUG_INFO)
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
    VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(BREAD_WIN32)
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
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
    VK_INSTANCE_REQUIRED_LOAD(instance, vkCreateWin32SurfaceKHR);
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
    VK_DEVICE_REQUIRED_LOAD(table, device, vkAcquireNextImage2KHR);

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
    VK_DEVICE_REQUIRED_LOAD(table, device, vkMapMemory2);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkUnmapMemory2);

    // buffer
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateBuffer);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyBuffer);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkBindBufferMemory2);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateBufferView);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyBufferView);

    // descriptors
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateDescriptorPool);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyDescriptorPool);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkAllocateDescriptorSets);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkFreeDescriptorSets);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkUpdateDescriptorSets);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateDescriptorSetLayout);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyDescriptorSetLayout);

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

    // VK_KHR_dynamic_rendering
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdBeginRenderingKHR);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdEndRendering);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdPipelineBarrier);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdCopyBuffer);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdBindPipeline);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdBindDescriptorSets);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdBindVertexBuffers);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdPushConstants);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdSetViewport);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdSetScissor);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCmdDraw);

    // image
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateImage);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyImage);

    // image view
    VK_DEVICE_REQUIRED_LOAD(table, device, vkCreateImageView);
    VK_DEVICE_REQUIRED_LOAD(table, device, vkDestroyImageView);
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
#else
    const VkBool32 verbose_value = false;
    const VkLayerSettingEXT layer_setting =
    {
        .pLayerName = "VK_LAYER_KHRONOS_validation",
        .pSettingName = "printf_verbose",
        .type = VK_LAYER_SETTING_TYPE_BOOL32_EXT,
        .valueCount = 1,
        .pValues = &verbose_value,
    };

    VkLayerSettingsCreateInfoEXT layer_settings_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
        .pNext = nullptr,
        .settingCount = 1,
        .pSettings = &layer_setting,
    };
    (void)layer_settings_create_info;
#endif

    const char* layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    VkInstanceCreateInfo instance_info =
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &application_info,
        .enabledLayerCount = 1,
        .ppEnabledLayerNames = layers,
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
    VkSurfaceKHR surface = VK_NULL_HANDLE;

#if defined(BREAD_WIN32)
    VkWin32SurfaceCreateInfoKHR surface_info =
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
        
        surface_info.hwnd = CreateWindowExA(
            0, "Bread:vulkan_dummy", "-", WS_OVERLAPPEDWINDOW,
            0, 0, 200, 200, 0, 0, 0, 0
        );
    }
    
    VkResult result = vk.vkCreateWin32SurfaceKHR(instance, &surface_info, allocation_callbacks(), &surface);
#endif
    VKFailOn(result != VK_SUCCESS, "vkCreateSurfaceKHR({})", Vulkan::result_as_string(result));

    return surface;
}

void Vulkan::destroy_surface(VkInstance instance, VkSurfaceKHR surface)
{
    vk.vkDestroySurfaceKHR(instance, surface, allocation_callbacks());
}

void Vulkan::check_device_extensions(VkPhysicalDevice physical_device)
{
    mem::Allocator allocator = VulkanDriver::get_allocator();

    u32 extension_count;
    vk.vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
    
    Slice<VkExtensionProperties> device_extensions = allocator.array<VkExtensionProperties>(extension_count);
    vk.vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, device_extensions.ptr());

    usize finded_count = 0;
    for (const char* ext : VkCoreDeviceExtensions)
    {
        StringView ext_view = Vulkan::vulkan_string_to_sv(ext);
        for (VkExtensionProperties& act_ext : device_extensions)
        {
            if (ext_view.equals(Vulkan::vulkan_string_to_sv(act_ext.extensionName)))
            {
                finded_count++;
                break;
            }
        }
    }

    VKFailOn(
        finded_count != ArraySize(VkCoreDeviceExtensions),
        "the required extensions were not found"
    );

    allocator.free(mem::to_bytes(device_extensions));
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
 	return VK_TRUE;
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

	bool realloc_result = allocator.realloc(
		old_mem, size, alignment
	);

	if (realloc_result)
	{
		return old_mem.ptr();
	}

	Slice<u8> bytes = allocator.alloc(size, alignment);
    mem::copy(bytes, Slice<const u8>(reinterpret_cast<const u8*>(pOriginal), allocator.get_size_of(old_mem)));
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
	Unused(pUserData, size, allocationType, allocationScope);
}

void VKAPI_PTR Vulkan::_vk_driver_internal_free(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{
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

    usize finded_count = 0;
    for (const char* ext : _vk_extensions)
    {
        StringView ext_view = Vulkan::vulkan_string_to_sv(ext);
        for (VkExtensionProperties& act_ext : instance_extensions)
        {
            if (ext_view.equals(Vulkan::vulkan_string_to_sv(act_ext.extensionName)))
            {
                finded_count++;
                break;
            }
        }
    }

    VKFailOn(
        finded_count != ArraySize(_vk_extensions),
        "the required extensions were not found"
    );

    allocator.free(mem::to_bytes(instance_extensions));

}
