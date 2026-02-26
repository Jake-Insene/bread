#pragma once
#include "core/header.h"
#include "collections/string_view.h"
#include "graphics/vk/vk_vtable.h"



struct Vulkan
{
    static constexpr StringView VkApplicationName = "Game";
    static constexpr StringView VkEngineName = "Bread";

    static inline VkAllocationCallbacks* allocation_callbacks()
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

    static StringView result_as_string(VkResult result)
    {
        return StringView(string_VkResult(result), __string_len(string_VkResult(result)));
    }

    static StringView vulkan_string_to_sv(const char* vk_str)
    {
        return StringView(vk_str, __string_len(vk_str));
    }

    static uint32_t get_api_version();
    static VkInstance create_instance();

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

