#pragma once
#include "core/header.h"
#include "collections/string_view.h"
#include "debug/fail.h"
#include "graphics/vk/vk_vtable.h"
#include "log/log.h"


#define VKDebugInfo(...) Log::debug("[VKDriver]: " __VA_ARGS__)
#define VKFatal(...) Fatal("[VKDriver]: " __VA_ARGS__)
#define VKFailOn(cond, ...) FailOn(cond, "[VKDriver]: " __VA_ARGS__)


struct Vulkan
{
    static constexpr StringView VkApplicationName = "Game";
    static constexpr StringView VkEngineName = "Bread";

    static constexpr const char* VkCoreDeviceExtensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
        VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,
    };

    static StringView result_as_string(VkResult result)
    {
        return StringView(string_VkResult(result), __string_len(string_VkResult(result)));
    }
    
    static StringView vulkan_string_to_sv(const char* vk_str)
    {
        return StringView(vk_str, __string_len(vk_str));
    }
    
    static VkAllocationCallbacks* allocation_callbacks();
    static const char* const* core_device_extensions();

    static void load_core_procs(MemoryAddress vk_lib);
    static void load_instance_procs(VkInstance instance);
    static void load_device_procs(DeviceVulkanTable& table, VkDevice device);

    static uint32_t get_api_version();
    static VkInstance create_instance();

    static VkSurfaceKHR create_surface(VkInstance instance, MemoryAddress native_handle);
    static void destroy_surface(VkInstance instance, VkSurfaceKHR surface);
    
    static void check_device_extensions(VkPhysicalDevice physical_device);

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

    static void _check_instance_extensions();
};

