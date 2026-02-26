#include "graphics/vk/vk_header.h"

#include "graphics/vk/vk_driver.h"
#include "mem/utils.h"
#include "vulkan/vulkan_core.h"



static constexpr const char* _vk_extensions[] =
{
    VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(BREAD_SHOW_DEBUG_INFO)
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
#if defined(BREAD_WIN32)
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
};


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
    const VkBool32 verbose_value = true;
#else
    const VkBool32 verbose_value = false;
#endif
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

    VkInstanceCreateInfo instance_info =
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = &layer_settings_create_info,
        .flags = 0,
        .pApplicationInfo = &application_info,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
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

	bool realloc_result = allocator.realloc(
		old_mem, size, alignment
	);

	if (realloc_result)
	{
		return old_mem.ptr();
	}

	allocator.free(old_mem);

	Slice<u8> bytes = allocator.alloc(size, alignment);
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
