#include "render_device/render_device.h"

#include "engine/engine.h"
#include "log/log.h"


void RenderDevice::initialize(const RenderDeviceCreateInfo& info)
{
    allocator = info.allocator;

    device.init(allocator, Engine::get_selected_gpu_device());

    gpu_memory_allocator.init(
        {
            .allocator = allocator,
            .graphics_device = get_graphics_device(),
        }
    );
    resource_manager.init(
        {
            .allocator = allocator,
            .graphics_device = get_graphics_device(),
            .gpu_memory_allocator = get_gpu_memory_allocator(),
        }
    );
}

void RenderDevice::shutdown()
{
    device.get_graphics_queue()->wait_idle();
    device.get_compute_queue()->wait_idle();
    device.get_copy_queue()->wait_idle();
    device.get_present_queue()->wait_idle();

    resource_manager.destroy();
    gpu_memory_allocator.destroy();

    device.destroy();
}

