#include "renderer/renderer.h"

#include "graphics/command_pool.h"


void Renderer::init(const RendererCreateInfo& info)
{
    allocator = info.allocator;
    render_device = info.render_device;

    command_pool.init(
        {
            .allocator = allocator,
            .device = info.render_device->get_device(),
            .queue_usage = GPU::QueueUsage::Graphics
        }
    );

    swap_chain.init(
        {
            .allocator = allocator,
            .device = render_device->get_device(),
            .present_queue = render_device->get_present_queue(),
            .window = info.target_window->window_id,
            .surface_format = info.surface_format,
        }
    );

    max_frames_in_flight = info.max_frames_in_flight;
    frame_index = 0;

    frames = Array<RenderFrame>::with_size(allocator, max_frames_in_flight);
    frames.resize(max_frames_in_flight);
    (void)frames.iter().transform([&](RenderFrame&) -> RenderFrame
    {
        return RenderFrame
        {
            .present_complete_semaphore = GPU::semaphore_create(render_device->get_device(), {}),
            .in_flight_fence = GPU::FenceID::invalid(),
        };
    });

    render_finished_semaphores = Array<GPU::SemaphoreID>::with_size(allocator, swap_chain.get_image_count());
    render_finished_semaphores.resize(max_frames_in_flight);
    (void)render_finished_semaphores.iter().transform([&](GPU::SemaphoreID){ return GPU::semaphore_create(render_device->get_device(), {}); });
}

void Renderer::destroy()
{
    (void)render_finished_semaphores.iter().for_each([](GPU::SemaphoreID sem)
    {
        GPU::semaphore_destroy(sem);
    });
    (void)frames.iter().for_each([](RenderFrame& frame)
    {
        GPU::semaphore_destroy(frame.present_complete_semaphore);
    });
    render_finished_semaphores.destroy();
    frames.destroy();

    command_pool.destroy();
    swap_chain.destroy();
}

Renderer::FrameInfo Renderer::begin_frame()
{
    RenderFrame& frame = frames.get(frame_index);

    if(frame.in_flight_fence != GPU::FenceID::invalid())
    {
        GPU::fence_wait_for(Slice(&frame.in_flight_fence, 1), true, MaxValue<u64>);
    }

    // Acquiring image
    GPU::PipelineStages wait_stages[] =
    {
        GPU::PipelineStages::RenderOutput,
    };

    u32 image_index = MaxValue<u32>;
    bool image_acquired = swap_chain.acquire_image(
        &image_index,
        frame.present_complete_semaphore
    );
    if(image_acquired && frame.in_flight_fence != GPU::FenceID::invalid())
    {
        command_pool.release_fence(frame.in_flight_fence);
        frame.in_flight_fence = GPU::FenceID::invalid();
    }

    FrameFlags frame_flags = FrameFlags(0);
    GPU::TextureID image = GPU::TextureID::invalid();
    GPU::TextureViewID image_view = GPU::TextureViewID::invalid();
    if(image_index == MaxValue<u32> && image_acquired)
    {
        frame.in_flight_fence = command_pool.execute_empty(
            render_device->get_graphics_queue(),
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
        image = swap_chain.get_image(image_index).image;    
        image_view = swap_chain.get_image(image_index).image_view;    
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

GPU::CommandBufferID Renderer::acquire_command_buffer(const FrameInfo&)
{
    return command_pool.acquire_command_buffer();
}

void Renderer::submit_command_buffer(const FrameInfo& frame_info, const Slice<const GPU::PipelineStages>& wait_stages,
    GPU::CommandBufferID command_buffer)
{
    RenderFrame& frame = frames.get(frame_info.frame_index);
    frame.in_flight_fence = command_pool.execute(
        render_device->get_graphics_queue(),
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
    swap_chain.present(
        frame_info.image_index,
        Slice(&render_finished_semaphores.get(frame_info.image_index), 1)
    );
}

