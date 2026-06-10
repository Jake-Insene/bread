#pragma once
#include "collections/string_view.h"
#include "debug/fail.h"
#include "gpu/vk/vk_vtable.h"
#include "log/log.h"
#include "os/os.h"


#define VKDebugInfo(...) Log::debug("[VKDriver]: " __VA_ARGS__)
#define VKFatal(...) Fatal("[VKDriver]: " __VA_ARGS__)
#define VKFailOn(cond, ...) FailOn(cond, "[VKDriver]: " __VA_ARGS__)

struct VulkanAdapter;

struct Vulkan
{
    static constexpr StringView VkApplicationName = "Game";
    static constexpr StringView VkEngineName = "Bread";

    static constexpr const char* VkCoreDeviceExtensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,
        VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME,
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
        VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME,
        VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME,
    };

    struct AdditionalExtensionSupport
    {
        bool has_dynamic_rendering;
    };

    static StringView result_as_string(VkResult result)
    {
        return StringView(string_VkResult(result), __string_len(string_VkResult(result)));
    }
    
    static StringView vulkan_string_to_sv(const char* vk_str)
    {
        return StringView(vk_str, __string_len(vk_str));
    }
    
    static VkAllocationCallbacks* allocation_callbacks(VulkanAdapter* adapter);
    static const char* const* core_device_extensions();

    static void load_core_procs(OS::Handle vk_lib);
    static void load_instance_procs(VkInstance instance);
    static void load_device_procs(DeviceVulkanTable& table, VkDevice device);

    static uint32_t get_api_version();
    static VkInstance create_instance(VulkanAdapter* adapter);

    static VkSurfaceKHR create_surface(VulkanAdapter* adapter, VkInstance instance, MemoryAddress native_handle);
    
    static AdditionalExtensionSupport check_device_extensions(Mem::Allocator* allocator, VkPhysicalDevice physical_device);
    static void check_device_features(VkPhysicalDevice physical_device);

    static const char** get_device_extensions(Mem::Allocator* allocator, VkPhysicalDevice physical_device,
        const AdditionalExtensionSupport& add_ext, uint32_t* extension_count);
    static VkPhysicalDeviceFeatures2* get_device_features(Mem::Allocator* allocator, const AdditionalExtensionSupport& add_ext);

    static bool _has_extension(const Slice<VkExtensionProperties>& vk_device_extensions, const char* ext_name);

    /*
    * VK_ext_debug_utils
    */
    static VkBool32 VKAPI_PTR _vk_debug_utils_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );

    /*
    * Vk Driver Memory Allocator(Vendor driver side)
    */
    static void* VKAPI_PTR _vk_driver_allocate(
        void* pUserData,
        size_t size,
        size_t alignment,
        VkSystemAllocationScope allocationScope
    );

    static void* VKAPI_PTR _vk_driver_reallocate(
        void* pUserData,
        void* pOriginal,
        size_t size,
        size_t alignment,
        VkSystemAllocationScope allocationScope
    );

    static void VKAPI_PTR _vk_driver_free(
        void* pUserData,
        void* pMemory
    );

    static void VKAPI_PTR _vk_driver_internal_allocate(
        void* pUserData,
        size_t size,
        VkInternalAllocationType allocationType,
        VkSystemAllocationScope allocationScope
    );

    static void VKAPI_PTR _vk_driver_internal_free(
        void* pUserData,
        size_t size,
        VkInternalAllocationType allocationType,
        VkSystemAllocationScope allocationScope
    );

    static void _check_instance_extensions(Mem::Allocator* allocator);
};

