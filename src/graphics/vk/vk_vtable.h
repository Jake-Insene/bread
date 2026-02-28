#pragma once
#include "platform/platform_header.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/vk_enum_string_helper.h>

#if defined(BREAD_WIN32)
#include <vulkan/vulkan_win32.h>
#endif


using VkFuncGeneric = void(*)();
#if defined(BREAD_ANDROID)
static_assert(false, "implement this!");
#elif defined(BREAD_WIN32)
inline VkFuncGeneric(*platform_get_proc)(const char* name) = nullptr;
#endif 

#define VK_REQUIRED_LOAD(name, lib) \
    vk.name = reinterpret_cast<decltype(vk.name)>(OS::get_proc_address(lib, #name)); \
    VKFailOn(vk.name == nullptr, "couldn't load the function {}", #name);

#define VK_DEVICE_REQUIRED_LOAD(table, device, name) \
    table.name = reinterpret_cast<decltype(table.name)>(vk.vkGetDeviceProcAddr(device, #name)); \
    VKFailOn(table.name == nullptr, "couldn't load the function {}", #name);

#define VK_INSTANCE_REQUIRED_LOAD(instance, name) \
    vk.name = reinterpret_cast<decltype(vk.name)>(vk.vkGetInstanceProcAddr(instance, #name)); \
    VKFailOn(vk.name == nullptr, "couldn't load the function {}", #name);

#define VK_INSTANCE_LOAD(instance, name) \
    vk.name = reinterpret_cast<decltype(vk.name)>(vk.vkGetInstanceProcAddr(instance, #name)); \

#define VK_NOT_REQUIRED_LOAD(name) \
    vk.name = reinterpret_cast<decltype(vk.name)>(platform_get_proc(#name));

struct SharedVulkanTable
{
    PFN_vkCreateInstance vkCreateInstance;
    PFN_vkDestroyInstance vkDestroyInstance;
    PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
    PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
    PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

    // VK_ext_debug_utils
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;

    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
    PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
    PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2;
    PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
    
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;

#if defined(BREAD_WIN32)
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
    PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR;
#endif
    PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;

    PFN_vkCreateDevice vkCreateDevice;
    PFN_vkDestroyDevice vkDestroyDevice;
    PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
};

struct DeviceVulkanTable
{
    PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
    PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;

    PFN_vkGetDeviceQueue vkGetDeviceQueue;

    PFN_vkQueuePresentKHR vkQueuePresentKHR;
};

inline SharedVulkanTable vk = {};



