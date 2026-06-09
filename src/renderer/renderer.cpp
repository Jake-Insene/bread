#include "renderer/renderer.h"

#include "graphics/command_pool.h"
#include "graphics/fence.h"


void Renderer::init(const RendererCreateInfo& info)
{
    allocator = info.allocator;

    graphics_device = info.graphics_device;

    command_pool = graphics_device->create_command_pool(GPU::QueueUsage::Graphics);

    swap_chain = graphics_device->create_swap_chain(info.target_window, info.surface_format);

    max_frames_in_flight = info.max_frames_in_flight;
    frame_index = 0;

    frames = Array<RenderFrame>::with_size(allocator, info.max_frames_in_flight);
    frames.resize(max_frames_in_flight);
    (void)frames.iter().transform([&](RenderFrame&) -> RenderFrame
    {
        return RenderFrame
        {
            .present_complete_semaphore = graphics_device->create_semaphore(),
            .in_flight_fence = nullptr,
        };
    });

    render_finished_semaphores = Array<Graphics::Semaphore*>::with_size(allocator, swap_chain->get_image_count());
    render_finished_semaphores.resize(max_frames_in_flight);
    (void)render_finished_semaphores.iter().transform([&](Graphics::Semaphore*){ return graphics_device->create_semaphore(); });
}

void Renderer::destroy()
{
    (void)render_finished_semaphores.iter().for_each([](Graphics::Semaphore* sem)
    {
        sem->destroy();
    });
    (void)frames.iter().for_each([](RenderFrame& frame)
    {
        frame.present_complete_semaphore->destroy();
    });
    render_finished_semaphores.destroy();
    frames.destroy();

    command_pool->destroy();
    swap_chain->destroy();
}

Renderer::FrameInfo Renderer::begin_frame()
{
    RenderFrame& frame = frames.get(frame_index);

    if(frame.in_flight_fence != nullptr)
    {
        frame.in_flight_fence->wait(MaxValue<u64>);
    }

    // Acquiring image
    GPU::PipelineStages wait_stages[] =
    {
        GPU::PipelineStages::RenderOutput,
    };

    u32 image_index = MaxValue<u32>;
    bool image_acquired = swap_chain->acquire_image(
        &image_index,
        frame.present_complete_semaphore
    );
    if(image_acquired && frame.in_flight_fence != nullptr)
    {
        command_pool->release_fence(frame.in_flight_fence);
        frame.in_flight_fence = nullptr;
    }

    FrameFlags frame_flags = FrameFlags(0);
    GPU::TextureID image = GPU::TextureID::invalid();
    GPU::TextureViewID image_view = GPU::TextureViewID::invalid();
    if(image_index == MaxValue<u32> && image_acquired)
    {
        frame.in_flight_fence = command_pool->execute_empty(
            graphics_device->get_graphics_queue(),
            {
                .wait_semaphores = Slice(&frame.present_complete_semaphore, 1),
                .wait_stages = wait_stages,
                .signal_semaphores = {},
            }
        );
    }
    else if(image_acquired && image_index != MaxValue<u32>)
    {
        frame_flags |= FrameFlags::Acquired;
        image = swap_chain->get_image(image_index).image;    
        image_view = swap_chain->get_image(image_index).image_view;    
    }

    return FrameInfo
    {
        .flags = frame_flags,
        .frame_index = frame_index,
        .image_index = image_index,
        .image = image,
        .image_view = image_view,
    };
}

void Renderer::end_frame()
{
    frame_index = (frame_index + 1) % max_frames_in_flight;
}

Graphics::CommandBuffer* Renderer::acquire_command_buffer(const FrameInfo&)
{
    return command_pool->acquire_command_buffer();
}

void Renderer::submit_command_buffer(const FrameInfo& frame_info, const Slice<const GPU::PipelineStages>& wait_stages,
    Graphics::CommandBuffer* command_buffer)
{
    RenderFrame& frame = frames.get(frame_info.frame_index);
    frame.in_flight_fence = command_pool->execute(
        graphics_device->get_graphics_queue(),
        {
            .wait_semaphores = Slice(&frame.present_complete_semaphore, 1),
            .wait_stages = wait_stages,
            .command_buffer = command_buffer,
            .signal_semaphores = Slice(&render_finished_semaphores.get(frame_info.image_index), 1),
        }
    );
}

void Renderer::present(const FrameInfo& frame_info)
{
    swap_chain->present(
        graphics_device->get_present_queue(),
        frame_info.image_index,
        Slice(&render_finished_semaphores.get(frame_info.image_index), 1)
    );
}

