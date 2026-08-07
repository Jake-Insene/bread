#include "engine/application.h"


Application::Application(const ApplicationAllocateInfo& alloc_info)
: swap_chain(
    {
        .allocator = alloc_info.allocator,
        .device = alloc_info.render_device->get_device(),
        .present_queue = alloc_info.render_device->get_present_queue(),
        .window = alloc_info.window,
        .surface_format = Graphics::SwapChain::DefaultSurfaceFormat,
    }
)
{}

Application::~Application()
{}

void Application::initialize(const ApplicationInitializeInfo&)
{
}

void Application::shutdown()
{
}

void Application::load_resources()
{
}

void Application::unload_resources()
{
}

void Application::update(f32)
{
}

void Application::render()
{
}

void Application::event(const Event&)
{
}
