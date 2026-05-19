#include "renderer/renderer_2d.h"

#include "engine/engine.h"
#include "math/projection.h"


void Renderer2D::init(const Renderer2DCreateInfo& info)
{
    Renderer::init(info);

    batcher.init(
        {
            .allocator = info.allocator,
            .graphics_device = info.graphics_device,
            .gpu_memory_allocator = info.gpu_memory_allocator,
            .max_frames_in_flight = info.max_frames_in_flight,
            .max_instances_per_type = info.max_instances_per_primitive,
            .surface_format = info.surface_format,
        }
    );

    sampler = graphics_device->create_sampler(
        {
            .min_filter = GPU::Filter::Nearest,
            .mag_filter = GPU::Filter::Nearest,
            .mipmap_mode = GPU::SamplerMipMapMode::Nearest,
            .address_mode_u = GPU::SamplerAddressMode::Repeat,
            .address_mode_v = GPU::SamplerAddressMode::Repeat,
            .address_mode_w = GPU::SamplerAddressMode::Repeat,
            .mip_lod_bias = 0.F,
            .anisotropy_enable = true,
            .max_anisotropy = 1,
            .compare_enable = false,
            .compare_op = GPU::CompareOp::Always,
            .min_lod = 0.F,
            .max_lod = 0.F,
        }
    );
}

void Renderer2D::destroy()
{
    graphics_device->get_graphics_queue()->wait_idle();
    graphics_device->get_present_queue()->wait_idle();
    
    batcher.destroy();
    sampler->destroy();
    Renderer::destroy();
}

void Renderer2D::commit_sprite(const RendererBatch2D::SpriteInstance& sprite, GPU::TextureID texture)
{
    batcher.commit_sprite(sprite, texture, sampler);
}

void Renderer2D::commit_quad(const RendererBatch2D::QuadInstance& quad)
{
    batcher.commit_quad(quad);
}

void Renderer2D::commit_line(const RendererBatch2D::LineInstance& line)
{
    batcher.commit_line(line);
}

void Renderer2D::commit_circle(const RendererBatch2D::CircleInstance& circle)
{
    batcher.commit_circle(circle);
}

void Renderer2D::render(const FrameInfo& frame_info)
{
    if(!HasValue(frame_info.flags & FrameFlags::Acquired))
    {
        return;
    }

    Vector2I size = Engine::get_main_window()->get_size();
    Vector2I viewport_size = Engine::get_configuration().viewport_size;
    RenderFrame& frame = frames.get(frame_info.frame_index);

    RendererBatch2D::FrameInfo batch_frame = {};
    batch_frame.frame_index = frame_info.frame_index;
    batch_frame.image_index = frame_info.image_index;
    batch_frame.image = frame_info.image;
    batch_frame.viewport_size = Vector2(viewport_size);

    batcher.prepare_scene(batch_frame);
    batcher.build_batch(batch_frame);
    batcher.finish_scene(batch_frame);
    
    GPU::PipelineStages wait_stages[] =
    {
        GPU::PipelineStages::RenderOutput,
    };

    Graphics::CommandEncoder encoder = command_queue->acquire_encoder();
    encoder.begin();

    batcher.begin_batch_record(batch_frame, encoder);

    encoder.texture_barrier(
        {
            .src_stages = GPU::PipelineStages::RenderOutput,
            .dest_stages = GPU::PipelineStages::RenderOutput,
            .src_masks = GPU::AccessMasks(0),
            .dest_masks = GPU::AccessMasks::RenderOutputWrite,
            .src_layout = GPU::TextureLayout::Unknown,
            .dest_layout = GPU::TextureLayout::RenderOutput,
            .texture = frame_info.image,
            .subresource_range =
            {
                .aspect = GPU::TextureAspect::Color,
                .base_mip_level = 0,
                .level_count = 1,
                .base_array_layer = 0,
                .layer_count = 1,
            },
        }
    );

    GPU::AttachmentInfo render_attachment =
    {
        .texture = frame_info.image,
        .layout = GPU::TextureLayout::RenderOutput,
        .resolve_texture = GPU::TextureID::invalid(),
        .resolve_layout = GPU::TextureLayout::Unknown,
        .load_op = GPU::LoadOp::Clear,
        .store_op = GPU::StoreOp::Store,
        .clear_value =
        {
            .clear_color =
            {
                .r = 0.F,
                .g = 0.F,
                .b = 0.F,
                .a = 1.F,
            }
        },
    };

    encoder.begin_renderpass(
        {
            .offset = Vector2I(0, 0),
            .extent = Vector3U(size.x, size.y, 1),
            .render_attachments = Slice(&render_attachment, 1),
            .depth_attachment = {},
            .stencil_attachment = {},
        }
    );

    GPU::Viewport viewport = 
    {
        .x = 0,
        .y = 0,
        .width = f32(size.width),
        .height = f32(size.height),
        .min_depth = 0.F,
        .max_depth = 1.F,
    };

    GPU::Scissor scissor =
    {
        .x = 0,
        .y = 0,
        .width = u32(size.width),
        .height = u32(size.height),
    };

    encoder.set_viewports(0, Slice(&viewport, 1));
    encoder.set_scissors(0, Slice(&scissor, 1));
    
    batcher.end_batch_record(batch_frame, encoder);
    
    encoder.end_renderpass({});
    encoder.texture_barrier(
        {
            .src_stages = GPU::PipelineStages::RenderOutput,
            .dest_stages = GPU::PipelineStages::End,
            .src_masks = GPU::AccessMasks::RenderOutputWrite,
            .dest_masks = GPU::AccessMasks(0),
            .src_layout = GPU::TextureLayout::RenderOutput,
            .dest_layout = GPU::TextureLayout::Present,
            .texture = frame_info.image,
            .subresource_range =
            {
                .aspect = GPU::TextureAspect::Color,
                .base_mip_level = 0,
                .level_count = 1,
                .base_array_layer = 0,
                .layer_count = 1,
            },
        }
    );
    encoder.end();

    frame.in_flight_fence = command_queue->execute(
        {
            .wait_semaphores = Slice(&frame.present_complete_semaphore, 1),
            .wait_stages = wait_stages,
            .signal_semaphores = Slice(&render_finished_semaphores.get(frame_info.image_index), 1),
            .encoder = encoder,
        }
    );
    
    swap_chain->present(
        graphics_device->get_present_queue(),
        frame_info.image_index,
        Slice(&render_finished_semaphores.get(frame_info.image_index), 1)
    );
}

