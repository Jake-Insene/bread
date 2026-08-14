#pragma once
#include "Collections/StringView.hpp"
#include "Debug/Fail.hpp"
#include "gpu/vk/vk_vtable.h"
#include "Debug/Log.hpp"
#include "os/os.h"


#define VKDebugInfo(...) Log::debug("[VKDriver]: " __VA_ARGS__)
#define VKFatal(...) Fatal("[VKDriver]: " __VA_ARGS__)
#define VKFailOn(cond, ...) FailOn(cond, "[VKDriver]: " __VA_ARGS__)

struct VulkanAdapter;

struct Vulkan
{
    static constexpr Collections::StringView VkApplicationName = "Game";
    static constexpr Collections::StringView VkEngineName = "Bread";

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

    static Collections::StringView result_as_string(VkResult result)
    {
        return Collections::StringView(string_VkResult(result), Core::NullTerminatedLen(string_VkResult(result)));
    }
    
    static Collections::StringView vulkan_string_to_sv(const char* vk_str)
    {
        return Collections::StringView(vk_str, Core::NullTerminatedLen(vk_str));
    }
    
    static VkAllocationCallbacks* allocation_callbacks(VulkanAdapter* adapter);
    static const char* const* core_device_extensions();

    static void load_core_procs(OS::Handle vk_lib);
    static void load_instance_procs(VkInstance instance);
    static void load_device_procs(DeviceVulkanTable& table, VkDevice device);

    static uint32_t get_api_version();
    static VkInstance create_instance(VulkanAdapter* adapter);

    static VkSurfaceKHR create_surface(VulkanAdapter* adapter, VkInstance instance, MemoryAddress native_handle);
    
    static AdditionalExtensionSupport check_device_extensions(Slice<VkExtensionProperties> vk_device_extensions);
    static void check_device_features(VkPhysicalDevice physical_device);

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

    static void _check_instance_extensions(Mem::Allocator& allocator);
};

