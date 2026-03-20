#include "render/2d/scene_renderer_2d.h"

#include "render/render_device.h"

#include "engine/engine.h"
#include "io/file.h"


void SceneRenderer2D::initialize(const mem::Allocator &allocator)
{
    data.allocator = allocator;

    data.device = Engine::get_system_manager().get_system<RenderDevice>()->get_graphics_device();

    data.graphics_queue = Graphics::queue_create(
        {
            .device = data.device,
            .usage = Graphics::QueueUsage::Graphics,
        }
    );

    data.present_queue = Graphics::queue_create(
        {
            .device = data.device,
            .usage = Graphics::QueueUsage::Present,
        }
    );

    data.command_pool = Graphics::command_pool_create(
        {
            .device = data.device,
            .queue = data.graphics_queue,
        }
    );
    
    data.swap_chain = Graphics::SwapChainID();
    _recreate_swap_chain();
    
    data.render_image_finish_semaphore = get_allocator().array<Graphics::SemaphoreID>(Graphics::swap_chain_get_image_count(data.swap_chain));
    for(usize i = 0; i < data.render_image_finish_semaphore.len; i++)
    {
        data.render_image_finish_semaphore[i] = Graphics::semaphore_create({.device = data.device});
    }

    data.memory.vertex_buffer_heap = Graphics::memory_heap_create(
        {
            .device = data.device,
            .heap_usage = Graphics::HeapUsage::CPUGPUCoherent,
            .heap_size = mem::align_up(VertexBufferHeapSize, Graphics::HeapAlignment),
        }
    );

    data.memory.vertex_buffer = Graphics::buffer_create(
        {
            .device = data.device,
            .usage = Graphics::BufferUsage::VertexBuffer,
            .size = mem::align_up(VertexBufferHeapSize, Graphics::MinHeapResourceAlignment),
            .memory_heap = data.memory.vertex_buffer_heap,
            .heap_offset = 0,
        }
    );

    data.memory.mapped_vertex_buffer = Graphics::buffer_map_memory(data.memory.vertex_buffer, 0, VertexBufferHeapSize);

    data.memory.frame_uniform_heap = Graphics::memory_heap_create(
        {
            .device = data.device,
            .heap_usage = Graphics::HeapUsage::CPUGPUCoherent,
            .heap_size = mem::align_up(FrameUniformBufferHeapSize, Graphics::HeapAlignment),
        }
    );

    data.memory.frame_uniform_buffer = Graphics::buffer_create(
        {
            .device = data.device,
            .usage = Graphics::BufferUsage::UniformBuffer,
            .size = mem::align_up(usize(sizeof(FrameUniformInfo)), Graphics::MinHeapResourceAlignment),
            .memory_heap = data.memory.frame_uniform_heap,
            .heap_offset = 0,
        }
    );

    data.memory.mapped_frame_uniform_buffer = Graphics::buffer_map_memory(data.memory.frame_uniform_buffer, 0, FrameUniformBufferHeapSize);
    
    _create_pipelines();

    data.frames_in_flight = get_allocator().array<FrameInFlightInfo>(MaxFrameCount);
    for(usize i = 0; i < data.frames_in_flight.len; i++)
    {
        data.frames_in_flight[i] = _create_frame_info(data.command_pool, i);
    }

    data.frame_index = 0;

    data.can_render = true;
}

void SceneRenderer2D::shutdown()
{
    Graphics::queue_wait_idle(data.graphics_queue);
    Graphics::queue_wait_idle(data.present_queue);

    _destroy_pipelines();

    for(usize i = 0; i < data.frames_in_flight.len; i++)
    {
        _destroy_frame_info(data.frames_in_flight[i]);
    }
    get_allocator().free(mem::to_bytes(data.frames_in_flight));

    Graphics::buffer_unmap_memory(data.memory.frame_uniform_buffer, data.memory.mapped_frame_uniform_buffer);
    Graphics::buffer_destroy(data.memory.frame_uniform_buffer);
    Graphics::memory_heap_destroy(data.memory.frame_uniform_heap);

    Graphics::buffer_unmap_memory(data.memory.vertex_buffer, data.memory.mapped_vertex_buffer);
    Graphics::buffer_destroy(data.memory.vertex_buffer);
    Graphics::memory_heap_destroy(data.memory.vertex_buffer_heap);

    for(usize i = 0; i < data.render_image_finish_semaphore.len; i++)
    {
        Graphics::semaphore_destroy(data.render_image_finish_semaphore[i]);
    }
    get_allocator().free(mem::to_bytes(data.render_image_finish_semaphore));

    Graphics::swap_chain_destroy(data.swap_chain);

    Graphics::queue_destroy(data.graphics_queue);
    Graphics::queue_destroy(data.present_queue);

    Graphics::command_pool_destroy(data.command_pool);
}

void SceneRenderer2D::draw_rect(const Transform2D& transform, const Color& color, const Rect2D& rect)
{
    Unused(transform);
    FrameInFlightInfo& frame_info = data.frames_in_flight[data.frame_index];

    QuadInstance& quad = *reinterpret_cast<QuadInstance*>(
        data.memory.mapped_vertex_buffer.add(frame_info.vertex_heap_offset + frame_info.quad_count * sizeof(QuadInstance)).ptr()
    );
    quad.xx = transform[0];
    quad.yy = transform[1];
    quad.zz = transform.get_position();
    quad.color = color;
    quad.rect = rect;

    frame_info.quad_count++;
}

void SceneRenderer2D::dispatch()
{
    if(data.can_render == false)
        return;

    FrameInFlightInfo& frame_info = data.frames_in_flight[data.frame_index];

    Graphics::fence_wait_for(Slice(&frame_info.draw_fence, 1), true, MaxValue<u64>);
    Graphics::fence_reset(Slice(&frame_info.draw_fence, 1));

    u32 image_index;
    Graphics::swap_chain_acquire_next_image(
        data.swap_chain,
        {
            .timeout = MaxValue<u64>,
            .semaphore = frame_info.present_semaphore,
            .fence = {},
        },
        &image_index
    );

    FrameUniformInfo& frame_uniform_info = *reinterpret_cast<FrameUniformInfo*>(data.memory.mapped_frame_uniform_buffer.add(frame_info.frame_uniform_heap_offset).ptr());
    Vector2I window_size = Engine::get_main_window().get_size();

    frame_uniform_info.view = Mat4::identity();
    frame_uniform_info.projection = Projection::orthographic(0, window_size.width, 0, window_size.height, 0, 1);
    frame_uniform_info.view_projection = frame_uniform_info.view * frame_uniform_info.projection;
    
    frame_uniform_info.view.transpose();
    frame_uniform_info.projection.transpose();
    frame_uniform_info.view_projection.transpose();

    Graphics::command_buffer_begin(frame_info.command_buffer);
    Graphics::command_buffer_begin_renderpass(
        frame_info.command_buffer,
        {
            .size = Engine::get_main_window().get_size(),
            .swap_chain = data.swap_chain,
            .image_index = image_index,
            .clear_color = Color(0, 0, 0, 255),
        }
    );

    if(frame_info.quad_count != 0)
    {
        Graphics::command_buffer_bind_pipeline(
            frame_info.command_buffer, Graphics::PipelineBindPoint::Graphics, data.pipelines.quad_pipeline
        );
        Graphics::command_buffer_bind_descriptor_sets(
            frame_info.command_buffer, Graphics::PipelineBindPoint::Graphics, 0, Slice(&frame_info.quad_frame_set, 1)
        );

        Graphics::command_buffer_bind_vertex_buffers(
            frame_info.command_buffer, 0, Slice(&data.memory.vertex_buffer, 1),
            Slice(&frame_info.vertex_heap_offset, 1)
        );

        Graphics::Viewport viewport =
        {
            .x = 0,
            .y = 0,
            .width = f32(window_size.width),
            .height = f32(window_size.height),
            .min_depth = 0.f,
            .max_depth = 1.f,
        };

        Graphics::Scissor scissor =
        {
            .x = 0,
            .y = 0,
            .width = static_cast<u32>(window_size.width),
            .height = static_cast<u32>(window_size.height),
        };

        Graphics::command_buffer_set_viewports(frame_info.command_buffer, 0, Slice(&viewport, 1));
        Graphics::command_buffer_set_scissors(frame_info.command_buffer, 0, Slice(&scissor, 1));

        Graphics::command_buffer_draw(frame_info.command_buffer, 6, frame_info.quad_count, 0, 0);
        frame_info.quad_count = 0;
    }

    Graphics::command_buffer_end_renderpass(frame_info.command_buffer, {});
    Graphics::command_buffer_end(frame_info.command_buffer);

    Graphics::PipelineStages wait_stage = Graphics::PipelineStages::RenderOutput;
    Graphics::queue_execute_command_buffer(
        data.graphics_queue,
        {
            .wait_semaphores = Slice(&frame_info.present_semaphore, 1),
            .wait_stages = Slice(&wait_stage, 1),
            .command_buffers = Slice(&frame_info.command_buffer, 1),
            .signal_semaphores = Slice(&data.render_image_finish_semaphore[image_index], 1),
            .fence = frame_info.draw_fence,
        }
    );

    Graphics::queue_present(
        data.present_queue,
        {
            .wait_semaphores = Slice(&data.render_image_finish_semaphore[image_index], 1),
            .swapchains = Slice(&data.swap_chain, 1),
            .image_indices = Slice(&image_index, 1),
        }
    );

    data.frame_index = (data.frame_index + 1) % MaxFrameCount;
}

void SceneRenderer2D::_recreate_swap_chain()
{
    Graphics::queue_wait_idle(data.graphics_queue);
    Graphics::queue_wait_idle(data.present_queue);

    data.can_render = false;
    Vector2I window_size = Engine::get_main_window().get_size();
    if(window_size.width == 0 || window_size.height == 0)
    {
        return;
    }

    if(data.swap_chain.is_valid())
    {
        Graphics::swap_chain_destroy(data.swap_chain);
        data.swap_chain = Graphics::SwapChainID();
    }
    
    data.swap_chain = Graphics::swap_chain_create(
        {
            .device = data.device,
            .surface = Engine::get_main_window().get_surface(),
            .present_mode = Graphics::PresentMode::Immediate,
            .format = SwapChainFormat,
            .min_image_count = SwapChainMinImageCount,
            .size = Vector2U(window_size),
        }
    );

    data.can_render = true;
}

SceneRenderer2D::FrameInFlightInfo SceneRenderer2D::_create_frame_info(Graphics::CommandPoolID command_pool, u32 frame_index)
{
    FrameInFlightInfo frame_info = {};

    frame_info.command_buffer = Graphics::command_buffer_allocate(
        {
            .pool = command_pool,
        }
    );

    frame_info.draw_fence = Graphics::fence_create(
        {
            .device = data.device,
            .signaled = true,
        }
    );

    frame_info.present_semaphore = Graphics::semaphore_create(
        {
            .device = data.device,
        }
    );

    frame_info.quad_frame_set = Graphics::descriptor_set_create(
        {
            .device = data.device,
            .set_layout = data.pipelines.quad_layout,
        }
    );

    Graphics::DescriptorBufferInfo buffers[] =
    {
        { .buffer = data.memory.frame_uniform_buffer, .offset = frame_index * sizeof(FrameUniformInfo), .range = sizeof(FrameUniformInfo), },
    };

    Graphics::WriteDescriptorInfo write_infos[] =
    {
        { .binding = 0, .array_element = 0, .count = 1, .type = Graphics::DescriptorType::UniformBuffer, .textures = {}, .buffers = buffers, },
    };

    Graphics::descriptor_set_update_descriptors(
        frame_info.quad_frame_set,
        {
            .write_infos = write_infos,
        }
    );

    frame_info.vertex_heap_offset = frame_index * QuadInstancePerFrameSize;
    frame_info.frame_uniform_heap_offset = frame_index * sizeof(FrameUniformInfo);
    
    return frame_info;
}

void SceneRenderer2D::_destroy_frame_info(FrameInFlightInfo& frame_info)
{
    Graphics::command_buffer_free(frame_info.command_buffer);
    Graphics::fence_destroy(frame_info.draw_fence);
    Graphics::semaphore_destroy(frame_info.present_semaphore);
    Graphics::descriptor_set_destroy(frame_info.quad_frame_set);
}

void SceneRenderer2D::_create_pipelines()
{
    {
        Graphics::DescriptorBinding quad_bindings[] =
        {
            { .type = Graphics::DescriptorType::UniformBuffer, .binding = 0, .count = 1, .stages = Graphics::ShaderStage::Vertex },
        };

        data.pipelines.quad_layout = Graphics::descriptor_set_layout_create(
            {
                .device = data.device,
                .bindings = quad_bindings,
            }
        );
        
        Slice<u8> shader_code = File::read_all(get_allocator(), "shaders/bread/SceneRenderer2D/Quad.slang.spirv");

        Graphics::ShaderStageInfo quad_stages[] =
        {
            { .stage = Graphics::ShaderStage::Vertex, .code = shader_code, .name = "VertexMain", },
            { .stage = Graphics::ShaderStage::Fragment, .code = shader_code, .name = "FragmentMain", },
        };

        Graphics::VertexBinding quad_binding[] =
        {
            { .binding = 0, .stride = sizeof(QuadInstance), .input_rate = Graphics::InputRate::Instance },
        };

        Graphics::VertexAttribute quad_attributes[3] = {};
        for(usize i = 0; i < ArraySize(quad_attributes); i++)
        {
            quad_attributes[i] = 
            {
                .location = static_cast<u32>(i),
                .binding = 0,
                .format = Graphics::VertexFormat::RGBA32Float,
                .offset = static_cast<u32>(sizeof(Vector4) * i),
            };
        }

        data.pipelines.quad_pipeline = Graphics::pipeline_create(
            {
                .device = data.device,
                .bind_point = Graphics::PipelineBindPoint::Graphics,
                .shader_stages = quad_stages,
                .vertex_input =
                {
                    .bindings = Slice(quad_binding),
                    .attributes = Slice(quad_attributes),
                },
                .input_assembly =
                {
                    .topology = Graphics::PrimitiveTopology::TriangleList,
                },
                .rasterizer_state =
                {
                    .depth_clamp_enable = false,
                    .rasterizer_discard_enable = false,
                    .polygon_mode = Graphics::PolygonMode::Fill,
                    .cull_mode = Graphics::CullMode::Front,
                    .front_face = Graphics::FrontFace::ClockWise,
                    .line_width = 1.f,
                },
                .multisample_state =
                {
                    .sample_count = Graphics::SampleCount::Sample1,
                    .min_sample_shading = 0,
                    .sample_shading_enable = false,
                    .alpha_to_coverage_enable = false,
                    .alpha_one_enable = false,
                },
                .depth_stencil_state =
                {
                    .depth_test_enable = false,
                    .depth_write_enable = false,
                    .depth_bounds_test_enable = false,
                    .stencil_test_enable = false,
                    .min_depth_bounds = 0.f,
                    .max_depth_bounds = 1.f,
                },
                .pipeline_layout =
                {
                    .constant_blocks = {},
                    .set_layouts = Slice(&data.pipelines.quad_layout, 1),
                },
                .surface_format = SwapChainFormat,
            }
        );

        get_allocator().free(shader_code);
    }
}

void SceneRenderer2D::_destroy_pipelines()
{
    Graphics::descriptor_set_layout_destroy(data.pipelines.quad_layout);
    Graphics::pipeline_destroy(data.pipelines.quad_pipeline);
}
