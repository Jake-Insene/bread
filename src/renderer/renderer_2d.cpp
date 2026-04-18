#include "renderer/renderer_2d.h"

#include "engine/engine.h"
#include "math/projection.h"


void Renderer2D::init(const Renderer2DCreateInfo& info)
{
    Renderer::init(info);

    Graphics::Shader shader_code = {};
    shader_code.init(
        allocator,
        {
            .file_path = "shaders/bread/Renderer2D/Instance.slang.spirv",
            .vertex_name = "VertexMain",
            .fragment_name = "FragmentMain",
        }
    );

    GPU::VertexBinding bindings[] =
    {
        { .binding = 0, .stride = InstanceSize, .input_rate = GPU::InputRate::Instance },
    };

    GPU::VertexAttribute attributes[AttributesPerInstance] = {};
    for(usize i = 0; i < ArraySize(attributes); i++)
    {
        attributes[i] =
        {
            .location = u32(i),
            .binding = 0,
            .format = GPU::VertexFormat::RGBA32Float,
            .offset = u32(sizeof(Vector4) * i),
        };
    }

    GPU::DescriptorBinding frame_bindings[] =
    {
        { .type = GPU::DescriptorType::UniformBuffer, .binding = 0, .count = 1, .stages = GPU::ShaderStage::Vertex, },
    };
    
    Graphics::DescriptorSetLayoutCreateInfo set_layouts[] =
    {
        // Frame set
        {
            .bindings = frame_bindings,
        }
    };

    GPU::TextureFormat image_format = swap_chain->surface_format;
    Graphics::PipelineInfo pipeline_info = Graphics::Pipeline2D::make_default(
        {
            .shader = shader_code,
            .vertex_input = {
                .bindings = bindings,
                .attributes = attributes,
            },
            .constant_blocks = {},
            .set_layout_infos = set_layouts,
            .rendering_info =
            {
                .render_attachments = Slice(&image_format, 1),
            },
        }
    );
    pipeline = graphics_device->create_pipeline(pipeline_info);
    shader_code.destroy();

    buffers.init(
        {
            .allocator = allocator,
            .graphics_device = info.graphics_device,
            .gpu_memory_allocator = info.gpu_memory_allocator,
            .buffer_size = sizeof(InstanceSize) * MaxInstancePerFramedBuffer,
            .frame_count = info.max_frames_in_flight,
            .usage = GPU::BufferUsage::VertexBuffer,
        }
    );

    uniform_buffers.init(
        {
            .allocator = allocator,
            .graphics_device = info.graphics_device,
            .gpu_memory_allocator = info.gpu_memory_allocator,
            .buffer_size = sizeof(SceneUniform),
            .frame_count = info.max_frames_in_flight,
            .usage = GPU::BufferUsage::UniformBuffer,
        }
    );

    GPU::DescriptorPoolSize pool_sizes[] =
    {
        { .type = GPU::DescriptorType::UniformBuffer, .count = 1 * max_frames_in_flight, }
    };

    uniform_pool.init(
        {
            .allocator = allocator,
            .graphics_device = graphics_device,
            .sizes = pool_sizes,
            .max_sets = max_frames_in_flight,
            .frame_count = max_frames_in_flight,
            .gpu_set_layout = pipeline->get_set_layout(0),
        }
    );

    for(usize i = 0; i < max_frames_in_flight; i++)
    {
        FramedBuffer::BufferInfo buffer_info = uniform_buffers.get_buffer_info(i);
        uniform_pool.get_set(i)->set_uniform_buffer(0, uniform_buffers.get_buffer(), buffer_info.offset, sizeof(SceneUniform));
    }
}

void Renderer2D::destroy()
{
    graphics_device->get_graphics_queue().wait_idle();
    graphics_device->get_present_queue().wait_idle();
    pipeline->destroy();

    buffers.destroy();
    uniform_buffers.destroy();
    uniform_pool.destroy();

    Renderer::destroy();
}

void Renderer2D::build_frame(const FrameInfo& frame_info)
{
    if(!frame_was_acquired)
    {
        return;
    }

    Vector2I size = Engine::get_main_window().get_size();
    Vector2I viewport_size = Engine::get_configuration().viewport_size;
    RenderFrame& frame = frames.get(frame_info.frame_index);

    BaseInstance* instance = reinterpret_cast<BaseInstance*>(buffers.get_mapped_staging(frame_info.frame_index).ptr());
    instance[0].xx = Vector2(1, 0);
    instance[0].yy = Vector2(0, 1);
    instance[0].zz = Vector2(0, 0);
    instance[0].color = Color(255, 255, 255, 255);
    instance[0].rect = Rect2D(0, 0, 100, 100);

    SceneUniform* scene_uniform = reinterpret_cast<SceneUniform*>(uniform_buffers.get_mapped(frame_info.frame_index).ptr());
    scene_uniform->view = Mat4::identity();
    scene_uniform->projection = Projection::orthographic(
        0, viewport_size.width,
        0, viewport_size.height,
        0, 1
    );
    scene_uniform->view_projection = scene_uniform->view * scene_uniform->projection;

    scene_uniform->view.transpose();
    scene_uniform->projection.transpose();
    scene_uniform->view_projection.transpose();
    
    GPU::PipelineStages wait_stages[] =
    {
        GPU::PipelineStages::RenderOutput,
    };

    Graphics::CommandEncoder encoder = command_queue->acquire_encoder();
    encoder.begin();

    // doing staging
    FramedBuffer::BufferInfo vertex_buffer_info = buffers.get_buffer_info(frame_info.frame_index);
    GPU::BufferCopyRegion copy_regions[] =
    {
        {
            .source_offset = vertex_buffer_info.offset,
            .destination_offset = vertex_buffer_info.offset,
            .size = MaxInstancePerFramedBuffer,
        }
    };

    Graphics::Buffer* vb = buffers.get_buffer();
    Graphics::Buffer* svb = buffers.get_staging_buffer();
    GPU::command_buffer_copy_buffer(encoder.command_buffer,
        {
            .source_buffer = svb->gpu_buffer,
            .destination_buffer = vb->gpu_buffer,
            .copy_regions = copy_regions,
        }
    );

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
    encoder.begin_renderpass(
        {
            .offset = Vector2I(0, 0),
            .extent = Vector2U(size),
            .render_attachment =
            {
                .image = frame_info.image,
                .layout = GPU::TextureLayout::RenderOutput,
                .resolve_image = GPU::TextureID::invalid(),
                .resolve_layout = GPU::TextureLayout::Unknown,
                .load_op = GPU::LoadOp::Clear,
                .store_op = GPU::StoreOp::Store,
                .clear_color =
                {
                    .r = 0.f,
                    .g = 0.f,
                    .b = 0.f,
                    .a = 1.f,
                },
            },
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
        .min_depth = 0.f,
        .max_depth = 1.f,
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
    encoder.bind_pipeline(GPU::PipelineBindPoint::Graphics, pipeline);
    
    Graphics::DescriptorSet* uniform_set = uniform_pool.get_set(frame_info.frame_index);
    encoder.bind_set(GPU::PipelineBindPoint::Graphics, pipeline, 0, Slice(&uniform_set, 1));

    encoder.bind_vertex_buffers(0, Slice(&vb, 1), Slice(&vertex_buffer_info.offset, 1));

    encoder.draw(6, 1, 0, 0);
    
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

