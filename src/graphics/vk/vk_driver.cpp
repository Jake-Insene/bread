#include "graphics/vk/vk_driver.h"

#include "graphics/vk/vk_vtable.h"
#include "graphics/vk/vk_header.h"
#include "os/os.h"
#include "vulkan/vulkan_core.h"



Adapter VulkanDriver::get_adapter()
{
    return Adapter
    {
        .initialize = &VulkanDriver::initialize,
        .shutdown = &VulkanDriver::shutdown,
        .swap_chain_create = &VulkanDriver::swap_chain_create,
        .swap_chain_destroy = &VulkanDriver::swap_chain_destroy,
        .swap_chain_get_render_target = &VulkanDriver::swap_chain_get_render_target,
        .swap_chain_present = &VulkanDriver::swap_chain_present,
        .buffer_create = &VulkanDriver::buffer_create,
        .buffer_destroy = &VulkanDriver::buffer_destroy,
        .buffer_map_memory = &VulkanDriver::buffer_map_memory,
        .buffer_unmap_memory = &VulkanDriver::buffer_unmap_memory,
        .texture_create = &VulkanDriver::texture_create,
        .texture_destroy = &VulkanDriver::texture_destroy,
        .texture_get_size = &VulkanDriver::texture_get_size,
        .render_target_create = &VulkanDriver::render_target_create,
        .render_target_destroy = &VulkanDriver::render_target_destroy,
        .render_target_get_texture = &VulkanDriver::render_target_get_texture,
        .pipeline_create = &VulkanDriver::pipeline_create,
        .pipeline_destroy = &VulkanDriver::pipeline_destroy,
        .program_create = &VulkanDriver::program_create,
        .program_destroy = &VulkanDriver::program_destroy,
        .command_buffer_create = &VulkanDriver::command_buffer_create,
        .command_buffer_destroy = &VulkanDriver::command_buffer_destroy,
        .command_buffer_begin = &VulkanDriver::command_buffer_begin,
        .command_buffer_blit_framebuffer = &VulkanDriver::command_buffer_blit_framebuffer,
        .command_buffer_bind_vertex_buffers = &VulkanDriver::command_buffer_bind_vertex_buffers,
        .command_buffer_bind_index_buffer = &VulkanDriver::command_buffer_bind_index_buffer,
        .command_buffer_bind_pipeline = &VulkanDriver::command_buffer_bind_pipeline,
        .command_buffer_bind_render_target = &VulkanDriver::command_buffer_bind_render_target,
        .command_buffer_set_texture_unit = &VulkanDriver::command_buffer_set_texture_unit,
        .command_buffer_set_uniform = &VulkanDriver::command_buffer_set_uniform,
        .command_buffer_set_viewport = &VulkanDriver::command_buffer_set_viewport,
        .command_buffer_clear = &VulkanDriver::command_buffer_clear,
        .command_buffer_draw = &VulkanDriver::command_buffer_draw,
        .command_buffer_draw_indexed = &VulkanDriver::command_buffer_draw_indexed,
        .command_buffer_end = &VulkanDriver::command_buffer_end,
        .queue_create = &VulkanDriver::queue_create,
        .queue_destroy = &VulkanDriver::queue_destroy,
        .queue_execute_command_buffer = &VulkanDriver::queue_execute_command_buffer,
    };
}

void VulkanDriver::initialize(const mem::Allocator &allocator)
{
    data.allocator = allocator;

    data.vk_lib = OS::load_library("vulkan-1.dll");

    // Global procs, required for (struct Vulkan)
    VK_REQUIRED_LOAD(vkCreateInstance, data.vk_lib);
    VK_REQUIRED_LOAD(vkDestroyInstance, data.vk_lib);
    VK_REQUIRED_LOAD(vkEnumerateInstanceVersion, data.vk_lib);
    VK_REQUIRED_LOAD(vkEnumerateInstanceLayerProperties, data.vk_lib);
    VK_REQUIRED_LOAD(vkEnumerateInstanceExtensionProperties, data.vk_lib);
    VK_REQUIRED_LOAD(vkGetInstanceProcAddr, data.vk_lib);

    data.info.api_version = Vulkan::get_api_version();
    VKDebugInfo(
        "Vulkan API Version: {}.{}.{}",
        VK_API_VERSION_MAJOR(data.info.api_version),
        VK_API_VERSION_MINOR(data.info.api_version),
        VK_API_VERSION_PATCH(data.info.api_version)
    );

    data.instance = Vulkan::create_instance();

    // Instance procs
    VK_INSTANCE_REQUIRED_LOAD(data.instance, vkCreateDebugUtilsMessengerEXT);
    VK_INSTANCE_REQUIRED_LOAD(data.instance, vkDestroyDebugUtilsMessengerEXT);
    VK_INSTANCE_REQUIRED_LOAD(data.instance, vkEnumeratePhysicalDevices);
    VK_INSTANCE_REQUIRED_LOAD(data.instance, vkGetPhysicalDeviceFeatures2);
    VK_INSTANCE_REQUIRED_LOAD(data.instance, vkGetPhysicalDeviceProperties2);

    // VK_ext_debug_util
#if defined(BREAD_SHOW_DEBUG_INFO)

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
            data.instance, &debug_messenger_info, Vulkan::allocation_callbacks(), &data.messenger
        );
        VKFailOn(result != VK_SUCCESS, "vkCreateDebugUtilsMessengerEXT({})", Vulkan::result_as_string(result));
    }
#endif

    // Selecting a physical_device
    {
        uint32_t physical_device_count = 0;
        vk.vkEnumeratePhysicalDevices(
            data.instance, &physical_device_count, nullptr
        );

        Slice<VkPhysicalDevice> physical_devices = allocator.array<VkPhysicalDevice>(physical_device_count);
        vk.vkEnumeratePhysicalDevices(data.instance, &physical_device_count, physical_devices.ptr());

        for(usize i = 0; i < physical_device_count; i++)
        {
            VkPhysicalDevice device = physical_devices[i];
            
            VkPhysicalDeviceFeatures2 features2 =
            {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                .pNext = nullptr,
                .features = {},
            };
            VkPhysicalDeviceProperties2 properties2 =
            {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
                .pNext = nullptr,
                .properties = {}
            };
            vk.vkGetPhysicalDeviceFeatures2(device, &features2);
            vk.vkGetPhysicalDeviceProperties2(device, &properties2);

            VkPhysicalDeviceProperties properties = properties2.properties;

            VKDebugInfo("Device({}): Name: {}", i, Vulkan::vulkan_string_to_sv(properties.deviceName));
        }

        allocator.free(mem::to_bytes(physical_devices));
    }
}

void VulkanDriver::shutdown()
{
#if defined(BREAD_SHOW_DEBUG_INFO)
    vk.vkDestroyDebugUtilsMessengerEXT(
        data.instance, data.messenger, Vulkan::allocation_callbacks()
    );
#endif

    vk.vkDestroyInstance(data.instance, Vulkan::allocation_callbacks());

    OS::unload_library(data.vk_lib);
}

Graphics::SwapChainID VulkanDriver::swap_chain_create(const Graphics::SwapChainCreateInfo& ci)
{
    Unused(ci);
    return Graphics::SwapChainID();
}

void VulkanDriver::swap_chain_destroy(Graphics::SwapChainID swap_chain)
{
    Unused(swap_chain);
}

Graphics::RenderTargetID VulkanDriver::swap_chain_get_render_target(Graphics::SwapChainID swap_chain, usize render_target_index)
{
    Unused(swap_chain, render_target_index);
    return Graphics::RenderTargetID();
}

void VulkanDriver::swap_chain_present(Graphics::SwapChainID swap_chain, usize render_target_index)
{
    Unused(swap_chain, render_target_index);
}

Graphics::BufferID VulkanDriver::buffer_create(const Graphics::BufferCreateInfo& ci)
{
    Unused(ci);
    return Graphics::BufferID();
}

void VulkanDriver::buffer_destroy(Graphics::BufferID buffer)
{
    Unused(buffer);
}

Slice<u8> VulkanDriver::buffer_map_memory(Graphics::BufferID buffer, usize offset, usize len)
{
    Unused(buffer, offset, len);
    return Slice<u8>();
}

void VulkanDriver::buffer_unmap_memory(Graphics::BufferID buffer, const Slice<u8>& memory)
{
    Unused(buffer, memory);
}

Graphics::TextureID VulkanDriver::texture_create(const Graphics::TextureCreateInfo& ci)
{
    Unused(ci);
    return Graphics::TextureID();
}

void VulkanDriver::texture_destroy(Graphics::TextureID texture)
{
    Unused(texture);
}

Vector2I VulkanDriver::texture_get_size(Graphics::TextureID texture)
{
    Unused(texture);
    return Vector2I();
}

Graphics::RenderTargetID VulkanDriver::render_target_create(const Graphics::RenderTargetCreateInfo& ci)
{
    Unused(ci);
    return Graphics::RenderTargetID();
}

void VulkanDriver::render_target_destroy(Graphics::RenderTargetID render_target)
{
    Unused(render_target);
}

Graphics::TextureID VulkanDriver::render_target_get_texture(Graphics::RenderTargetID render_target)
{
    Unused(render_target);
    return Graphics::TextureID();
}

Graphics::PipelineID VulkanDriver::pipeline_create(const Graphics::PipelineCreateInfo& ci)
{
    Unused(ci);
    return Graphics::PipelineID();
}

void VulkanDriver::pipeline_destroy(Graphics::PipelineID pipeline)
{
    Unused(pipeline);
}

Graphics::ProgramID VulkanDriver::program_create(const Graphics::ProgramCreateInfo& ci)
{
    Unused(ci);
    return Graphics::ProgramID();
}

void VulkanDriver::program_destroy(Graphics::ProgramID program)
{
    Unused(program);
}

Graphics::CommandBufferID VulkanDriver::command_buffer_create(const Graphics::CommandBufferCreateInfo& ci)
{
    Unused(ci);
    return Graphics::CommandBufferID();
}

void VulkanDriver::command_buffer_destroy(Graphics::CommandBufferID cmd)
{
    Unused(cmd);
}

void VulkanDriver::command_buffer_begin(Graphics::CommandBufferID cmd)
{
    Unused(cmd);
}

void VulkanDriver::command_buffer_blit_framebuffer(Graphics::CommandBufferID cmd, Graphics::RenderTargetID src_render_target, Graphics::RenderTargetID dst_render_target, Rect2DI src_rect, Rect2DI dst_rect, Graphics::TextureFilter filter)
{
    Unused(cmd, src_render_target, dst_render_target, src_rect, dst_rect, filter);
}

void VulkanDriver::command_buffer_bind_vertex_buffers(Graphics::CommandBufferID cmd, u32 binding, const Slice<Graphics::BufferID>& buffers, const Slice<u32>& offsets, const Slice<u32>& strides)
{
    Unused(cmd, binding, buffers, offsets, strides);
}

void VulkanDriver::command_buffer_bind_index_buffer(Graphics::CommandBufferID cmd, Graphics::BufferID index_buffer, u32 offset, Graphics::IndexType index_type)
{
    Unused(cmd, index_buffer, offset, index_type);
}

void VulkanDriver::command_buffer_bind_pipeline(Graphics::CommandBufferID cmd, Graphics::PipelineID pipeline)
{
    Unused(cmd, pipeline);
}

void VulkanDriver::command_buffer_bind_render_target(Graphics::CommandBufferID cmd, Graphics::RenderTargetID render_target)
{
    Unused(cmd, render_target);
}

void VulkanDriver::command_buffer_set_texture_unit(Graphics::CommandBufferID cmd, u32 set, u32 base_slot, const Slice<Graphics::TextureID>& textures)
{
    Unused(cmd, set, base_slot, textures);
}

void VulkanDriver::command_buffer_set_uniform(Graphics::CommandBufferID cmd, u32 set, u32 base_slot, const Slice<Graphics::BufferID>& buffers)
{
    Unused(cmd, set, base_slot, buffers);
}

void VulkanDriver::command_buffer_set_viewport(Graphics::CommandBufferID cmd, Rect2DI viewport_rect)
{
    Unused(cmd, viewport_rect);
}

void VulkanDriver::command_buffer_clear(Graphics::CommandBufferID cmd, Graphics::RenderTargetID render_target, Color clear_color)
{
    Unused(cmd, render_target, clear_color);
}

void VulkanDriver::command_buffer_draw(Graphics::CommandBufferID cmd, u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance)
{
    Unused(cmd, vertex_count, instance_count, base_vertex, base_instance);
}

void VulkanDriver::command_buffer_draw_indexed(Graphics::CommandBufferID cmd, u32 index_count, u32 instance_count, u32 base_index, u32 base_vertex, u32 base_instance)
{
    Unused(cmd, index_count, instance_count, base_index, base_vertex, base_instance);
}

void VulkanDriver::command_buffer_end(Graphics::CommandBufferID cmd)
{
    Unused(cmd);
}

Graphics::QueueID VulkanDriver::queue_create(const Graphics::QueueCreateInfo& ci)
{
    Unused(ci);
    return Graphics::QueueID();
}

void VulkanDriver::queue_destroy(Graphics::QueueID queue)
{
    Unused(queue);
}

void VulkanDriver::queue_execute_command_buffer(Graphics::QueueID queue, const Slice<Graphics::CommandBufferID>& command_buffers)
{
    Unused(queue, command_buffers);
}

