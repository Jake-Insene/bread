#include "renderer/renderer_batch_2d.h"

#include "math/projection.h"
#include "renderer/renderer_2d.h"


void RendererBatch2D::init(const RendererBatch2DCreateInfo& batch_info)
{
    allocator = batch_info.allocator;
    graphics_device = batch_info.graphics_device;

    Graphics::Shader shader_codes[4] = {};
    shader_codes[0].init(
        allocator,
        {
            .file_path = "shaders/bread/Renderer2D/Instance.slang.Sprite.spirv",
            .vertex_name = "VertexMain",
            .fragment_name = "FragmentMain",
        }
    );
    shader_codes[1].init(
        allocator,
        {
            .file_path = "shaders/bread/Renderer2D/Instance.slang.Quad.spirv",
            .vertex_name = "VertexMain",
            .fragment_name = "FragmentMain",
        }
    );
    shader_codes[2].init(
        allocator,
        {
            .file_path = "shaders/bread/Renderer2D/Instance.slang.Line.spirv",
            .vertex_name = "VertexMain",
            .fragment_name = "FragmentMain",
        }
    );
    shader_codes[3].init(
        allocator,
        {
            .file_path = "shaders/bread/Renderer2D/Instance.slang.Circle.spirv",
            .vertex_name = "VertexMain",
            .fragment_name = "FragmentMain",
        }
    );

    constexpr usize PipelineVersionSizes[] = { sizeof(SpriteInstance), sizeof(QuadInstance), sizeof(LineInstance), sizeof(CircleInstance) };
    Graphics::Pipeline* pipelines[] = {nullptr, nullptr, nullptr, nullptr};
    for(usize i = 0; i < 4; i++)
    {
        GPU::VertexBinding bindings[] =
        {
            { .binding = 0, .stride = u32(PipelineVersionSizes[i]), .input_rate = GPU::InputRate::Instance },
        };

        usize attribute_count = PipelineVersionSizes[i]/sizeof(Vector4);
        GPU::VertexAttribute attributes[GPU::MaxVertexInputAttributes] = {};
        for(usize atti = 0; atti < attribute_count; atti++)
        {
            attributes[atti] =
            {
                .location = u32(atti),
                .binding = 0,
                .format = GPU::VertexFormat::RGBA32Float,
                .offset = u32(sizeof(Vector4) * atti),
            };
        }

        GPU::DescriptorBinding frame_bindings[] =
        {
            { .type = GPU::DescriptorType::UniformBuffer, .binding = 0, .count = 1, .stages = GPU::ShaderStage::Vertex, },
            { .type = GPU::DescriptorType::CombinedTextureSampler, .binding = 1, .count = 1, .stages = GPU::ShaderStage::Fragment, },
        };
        
        Graphics::DescriptorSetLayoutCreateInfo set_layouts[] =
        {
            // Frame set
            {
                .bindings = frame_bindings,
            }
        };

        GPU::TextureFormat image_format = batch_info.surface_format;
        Graphics::PipelineInfo pipeline_info = Graphics::Pipeline2D::make_default(
            {
                .shader = shader_codes[i],
                .vertex_input = {
                    .bindings = bindings,
                    .attributes = Slice(&attributes[0], attribute_count),
                },
                .constant_blocks = {},
                .set_layout_infos = set_layouts,
                .rendering_info =
                {
                    .render_attachments = Slice(&image_format, 1),
                },
            }
        );

        pipelines[i] = graphics_device->create_pipeline(pipeline_info);
        shader_codes[i].destroy();
    }

    sprite_pipeline = pipelines[0];
    quad_pipeline = pipelines[1];
    line_pipeline = pipelines[2];
    circle_pipeline = pipelines[3];

    instance_buffer_size = 0;
    for(usize instance_size : PipelineVersionSizes)
    {
        instance_buffer_size += instance_size * batch_info.max_instances_per_type;
    }

    sprite_offset_begin = 0;
    sprite_offset_end = sizeof(SpriteInstance) * batch_info.max_instances_per_type;
    quad_offset_begin = sprite_offset_end;
    quad_offset_end = quad_offset_begin + (sizeof(QuadInstance) * batch_info.max_instances_per_type);
    line_offset_begin = quad_offset_end;
    line_offset_end = line_offset_begin + (sizeof(LineInstance) * batch_info.max_instances_per_type);
    circle_offset_begin = line_offset_end;
    circle_offset_end = circle_offset_begin + (sizeof(CircleInstance) * batch_info.max_instances_per_type);

    instance_buffer.init(
        {
            .allocator = allocator,
            .graphics_device = graphics_device,
            .gpu_memory_allocator = batch_info.gpu_memory_allocator,
            .buffer_size = instance_buffer_size,
            .frame_count = batch_info.max_frames_in_flight,
            .usage = GPU::BufferUsage::VertexBuffer,
        }
    );

    uniform_buffer.init(
        {
            .allocator = allocator,
            .graphics_device = graphics_device,
            .gpu_memory_allocator = batch_info.gpu_memory_allocator,
            .buffer_size = sizeof(Renderer2D::SceneUniform),
            .frame_count = batch_info.max_frames_in_flight,
            .usage = GPU::BufferUsage::UniformBuffer,
        }
    );

    GPU::DescriptorPoolSize pool_sizes[] =
    {
        { .type = GPU::DescriptorType::UniformBuffer, .count = 1 * batch_info.max_frames_in_flight, },
        { .type = GPU::DescriptorType::CombinedTextureSampler, .count = 128 * batch_info.max_frames_in_flight, },
    };

    uniform_pool.init(
        {
            .allocator = allocator,
            .graphics_device = graphics_device,
            .sizes = pool_sizes,
            .max_sets = batch_info.max_frames_in_flight,
            .frame_count = batch_info.max_frames_in_flight,
            .gpu_set_layout = sprite_pipeline->get_set_layout(0),
        }
    );

    for(usize i = 0; i < batch_info.max_frames_in_flight; i++)
    {
        FramedBuffer::BufferInfo buffer_info = uniform_buffer.get_buffer_info(i);
        uniform_pool.get_set(i)->set_uniform_buffer(0, uniform_buffer.get_buffer(), buffer_info.offset, sizeof(Renderer2D::SceneUniform));
    }

    batches = Array<Batch>::with_size(allocator, 32);

    quad_count = 0;
    line_count = 0;
    circle_count = 0;

    quads = Array<QuadInstance>::with_size(allocator, batch_info.max_instances_per_type);
    lines = Array<LineInstance>::with_size(allocator, batch_info.max_instances_per_type);
    circles = Array<CircleInstance>::with_size(allocator, batch_info.max_instances_per_type);

    last_pipeline = nullptr;
}

void RendererBatch2D::destroy()
{
    sprite_pipeline->destroy();
    quad_pipeline->destroy();
    line_pipeline->destroy();
    circle_pipeline->destroy();

    instance_buffer.destroy();
    uniform_buffer.destroy();
    uniform_pool.destroy();

    quads.destroy();
    lines.destroy();
    circles.destroy();

    batches.destroy();
}

void RendererBatch2D::prepare_scene(const FrameInfo& frame_info)
{
    last_pipeline = nullptr;

    Renderer2D::SceneUniform* scene_uniform = reinterpret_cast<Renderer2D::SceneUniform*>(uniform_buffer.get_mapped(frame_info.frame_index).ptr());
    scene_uniform->view = Mat4::identity();
    scene_uniform->projection = Projection::orthographic(
        0, frame_info.viewport_size.width,
        0, frame_info.viewport_size.height,
        0, 1
    );
    scene_uniform->view_projection = scene_uniform->view * scene_uniform->projection;

    scene_uniform->view.transpose();
    scene_uniform->projection.transpose();
    scene_uniform->view_projection.transpose();
}

void RendererBatch2D::build_batch(const FrameInfo& frame_info)
{
    // build batches
    u8* staging_ptr = instance_buffer.get_mapped_staging(frame_info.frame_index).ptr();

    QuadInstance* quad_buffer = reinterpret_cast<QuadInstance*>(staging_ptr + quad_offset_begin);
    mem::copy(Slice(quad_buffer, quads.count), Slice(quads.items.items, quads.count));
    quad_count = quads.count;

    LineInstance* line_buffer = reinterpret_cast<LineInstance*>(staging_ptr + line_offset_begin);
    mem::copy(Slice(line_buffer, lines.count), Slice(lines.items.items, lines.count));
    line_count = lines.count;

    CircleInstance* circle_buffer = reinterpret_cast<CircleInstance*>(staging_ptr + circle_offset_begin);
    mem::copy(Slice(circle_buffer, circles.count), Slice(circles.items.items, circles.count));
    circle_count = circles.count;

    Graphics::DescriptorSet* uniform_set = uniform_pool.get_set(frame_info.frame_index);
    for (Batch& batch : batches.iter())
    {
        batch.set = uniform_set;
    }
}

void RendererBatch2D::finish_scene(const FrameInfo&)
{
    quads.clear();
    lines.clear();
    circles.clear();
}

void RendererBatch2D::begin_batch_record(const FrameInfo& frame_info, Graphics::CommandEncoder& encoder)
{
    FramedBuffer::BufferInfo vertex_buffer_info = instance_buffer.get_buffer_info(frame_info.frame_index);
    Graphics::Buffer* vb = instance_buffer.get_buffer();
    Graphics::Buffer* svb = instance_buffer.get_staging_buffer();

    // Copy per type
    // quad
    if(quad_count > 0)
    {
        GPU::BufferCopyRegion region =
        {
            .source_offset = vertex_buffer_info.offset + quad_offset_begin,
            .destination_offset = vertex_buffer_info.offset + quad_offset_begin,
            .size = quad_count * sizeof(QuadInstance),
        };
        GPU::command_buffer_copy_buffer(encoder.command_buffer,
            {
                .source_buffer = svb->gpu_buffer,
                .destination_buffer = vb->gpu_buffer,
                .copy_regions = Slice(&region, 1),
            }
        );
    }
    // line
    if(line_count > 0)
    {
        GPU::BufferCopyRegion region =
        {
            .source_offset = vertex_buffer_info.offset + line_offset_begin,
            .destination_offset = vertex_buffer_info.offset + line_offset_end,
            .size = line_count * sizeof(LineInstance),
        };
        GPU::command_buffer_copy_buffer(encoder.command_buffer,
            {
                .source_buffer = svb->gpu_buffer,
                .destination_buffer = vb->gpu_buffer,
                .copy_regions = Slice(&region, 1),
            }
        );
    }
    // circle
    if(circle_count > 0)
    {
        GPU::BufferCopyRegion region =
        {
            .source_offset = vertex_buffer_info.offset + circle_offset_begin,
            .destination_offset = vertex_buffer_info.offset + circle_offset_begin,
            .size = circle_count * sizeof(CircleInstance),
        };
        GPU::command_buffer_copy_buffer(encoder.command_buffer,
            {
                .source_buffer = svb->gpu_buffer,
                .destination_buffer = vb->gpu_buffer,
                .copy_regions = Slice(&region, 1),
            }
        );
    }

    quad_count = 0;
    line_count = 0;
    circle_count = 0;
}

void RendererBatch2D::end_batch_record(const FrameInfo& frame_info, Graphics::CommandEncoder& encoder)
{
    FramedBuffer::BufferInfo vertex_buffer_info = instance_buffer.get_buffer_info(frame_info.frame_index);

    Graphics::Buffer* vb = instance_buffer.get_buffer();

    for (const Batch& batch : batches.iter())
    {
        encoder.bind_pipeline(GPU::PipelineBindPoint::Graphics, batch.pipeline);
        Graphics::DescriptorSet* sets[] = { batch.set };
        encoder.bind_set(GPU::PipelineBindPoint::Graphics, batch.pipeline, 0, sets);

        usize buffer_offset = vertex_buffer_info.offset + batch.offset;
        encoder.bind_vertex_buffers(0, Slice(&vb, 1), Slice(&buffer_offset, 1));

        encoder.draw(batch.vertices_per_instance, batch.instance_count, 0, 0);
    }

    batches.clear();
}

void RendererBatch2D::commit_quad(const QuadInstance& quad)
{
    if (last_pipeline != quad_pipeline) {
        (void)batches.add({
            .pipeline = quad_pipeline,
            .set = nullptr,
            .offset = quad_offset_begin,
            .vertices_per_instance = 6,
            .instance_count = 0,
        });
        last_pipeline = quad_pipeline;
    }
    (void)quads.add(quad);
    batches.get(batches.count - 1).instance_count++;
}

void RendererBatch2D::commit_line(const LineInstance& line)
{
    if (last_pipeline != line_pipeline) {
        (void)batches.add({
            .pipeline = line_pipeline,
            .set = nullptr,
            .offset = line_offset_begin,
            .vertices_per_instance = 2,
            .instance_count = 0,
        });
        last_pipeline = line_pipeline;
    }
    (void)lines.add(line);
    batches.get(batches.count - 1).instance_count++;
}

void RendererBatch2D::commit_circle(const CircleInstance& circle)
{
    if (last_pipeline != circle_pipeline)
    {
        (void)batches.add({
            .pipeline = circle_pipeline,
            .set = nullptr,
            .offset = circle_offset_begin,
            .vertices_per_instance = 6,
            .instance_count = 0,
        });
        last_pipeline = circle_pipeline;
    }
    (void)circles.add(circle);
    batches.get(batches.count - 1).instance_count++;
}

