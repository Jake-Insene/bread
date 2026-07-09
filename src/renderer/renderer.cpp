#include "renderer/renderer.h"

#include "graphics/command_pool.h"


void Renderer::init(const RendererCreateInfo& info)
{
    data.allocator = info.allocator;
    data.render_device = info.render_device;

    data.command_pool.init(
        {
            .allocator = data.allocator,
            .device = info.render_device->get_device(),
            .queue_usage = GPU::QueueUsage::Graphics
        }
    );

    data.swap_chain = info.swap_chain;

    data.max_frames_in_flight = info.max_frames_in_flight;
    data.frame_index = 0;

    data.frames = Array<RenderFrame>::with_size(data.allocator, data.max_frames_in_flight);
    data.frames.resize(data.max_frames_in_flight);
    (void)data.frames.iter().transform([&](RenderFrame&) -> RenderFrame
    {
        return RenderFrame
        {
            .present_complete_semaphore = GPU::semaphore_create(data.render_device->get_device(), {}),
            .in_flight_fence = GPU::FenceID::invalid(),
        };
    });

    data.render_finished_semaphores = Array<GPU::SemaphoreID>::with_size(data.allocator, data.swap_chain->get_image_count());
    data.render_finished_semaphores.resize(data.max_frames_in_flight);
    (void)data.render_finished_semaphores.iter().transform([&](GPU::SemaphoreID)
    {
        return GPU::semaphore_create(data.render_device->get_device(), {});
    });
   
    const GPU::DescriptorPoolSize pool_sizes[] =
    {
        GPU::DescriptorPoolSize::uniform_buffer(16),
        GPU::DescriptorPoolSize::storage_buffer(16),
        GPU::DescriptorPoolSize::combined_texture_sampler(16),
    };
    data.frame_pool.init(
        {
            .allocator = data.allocator,
            .device = data.render_device->get_device(),
            .frame_count = data.max_frames_in_flight,
            .max_sets = 8,
            .sizes = pool_sizes,
        }
    );
}

void Renderer::destroy()
{
    data.frame_pool.destroy();
    (void)data.render_finished_semaphores.iter().for_each([](GPU::SemaphoreID sem)
    {
        GPU::semaphore_destroy(sem);
    });
    (void)data.frames.iter().for_each([](RenderFrame& frame)
    {
        GPU::semaphore_destroy(frame.present_complete_semaphore);
    });
    data.render_finished_semaphores.destroy();
    data.frames.destroy();

    data.command_pool.destroy();
}

Renderer::FrameInfo Renderer::begin_frame()
{
    RenderFrame& frame = data.frames.get(data.frame_index);

    if(frame.in_flight_fence != GPU::FenceID::invalid())
    {
        GPU::fence_wait_for(Slice(&frame.in_flight_fence, 1), true, MaxValue<u64>);
    }

    data.frame_pool.reset_pool(data.frame_index);

    // Acquiring image
    GPU::PipelineStages wait_stages[] =
    {
        GPU::PipelineStages::RenderOutput,
    };

    u32 image_index = MaxValue<u32>;
    bool image_acquired = data.swap_chain->acquire_image(
        &image_index,
        frame.present_complete_semaphore
    );
    if(image_acquired && frame.in_flight_fence != GPU::FenceID::invalid())
    {
        data.command_pool.release_fence(frame.in_flight_fence);
        frame.in_flight_fence = GPU::FenceID::invalid();
    }

    FrameFlags frame_flags = FrameFlags(0);
    GPU::TextureID image = GPU::TextureID::invalid();
    GPU::TextureViewID image_view = GPU::TextureViewID::invalid();
    if(image_index == MaxValue<u32> && image_acquired)
    {
        frame.in_flight_fence = data.command_pool.execute_empty(
            data.render_device->get_graphics_queue(),
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
        image = data.swap_chain->get_image(image_index).image;    
        image_view = data.swap_chain->get_image(image_index).image_view;    
    }

    return FrameInfo
    {
        .flags = frame_flags,
        .frame_index = data.frame_index,
        .image_index = image_index,
        .image = image,
        .image_view = image_view,
        .pool = data.frame_pool.get_pool(data.frame_index),
    };
}

void Renderer::end_frame()
{
    data.frame_index = (data.frame_index + 1) % data.max_frames_in_flight;
}

GPU::CommandBufferID Renderer::acquire_command_buffer(const FrameInfo&)
{
    return data.command_pool.acquire_command_buffer();
}

void Renderer::submit_command_buffer(const FrameInfo& frame_info, const Slice<const GPU::PipelineStages>& wait_stages,
    GPU::CommandBufferID command_buffer)
{
    RenderFrame& frame = data.frames.get(frame_info.frame_index);
    frame.in_flight_fence = data.command_pool.execute(
        data.render_device->get_graphics_queue(),
        {
            .wait_semaphores = Slice(&frame.present_complete_semaphore, 1),
            .wait_stages = wait_stages,
            .command_buffer = command_buffer,
            .signal_semaphores = Slice(&data.render_finished_semaphores.get(frame_info.image_index), 1),
        }
    );
}

void Renderer::present(const FrameInfo& frame_info)
{
    data.swap_chain->present(
        frame_info.image_index,
        Slice(&data.render_finished_semaphores.get(frame_info.image_index), 1)
    );
}

