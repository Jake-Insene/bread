#include "render/2d/scene_renderer_2d.h"

#include "render/render_device.h"

#include "engine/engine.h"
#include "io/file.h"


void SceneRenderer2D::initialize(const SystemInitializeInfo& info)
{
    allocator = info.allocator;

    device = Engine::get_system_manager().get_system<RenderDevice>()->get_graphics_device();

    graphics_queue = Engine::get_system_manager().get_system<RenderDevice>()->get_graphics_queue();

    present_queue = Engine::get_system_manager().get_system<RenderDevice>()->get_present_queue();

    GPU::DescriptorPoolSize pool_sizes[] =
    {
        { .type = GPU::DescriptorType::UniformBuffer, .count = 1000, }
    };

    descriptor_pool = GPU::descriptor_pool_create(
        {
            .device = device,
            .max_sets = 100,
            .sizes = pool_sizes,
        }
    );

    command_pool = GPU::command_pool_create(
        {
            .device = device,
            .queue = graphics_queue,
        }
    );
    
    swap_chain = GPU::SwapChainID();
    _recreate_swap_chain();
    
    render_image_finish_semaphore = get_allocator().array<GPU::SemaphoreID>(GPU::swap_chain_get_image_count(swap_chain));
    for(usize i = 0; i < render_image_finish_semaphore.len; i++)
    {
        render_image_finish_semaphore[i] = GPU::semaphore_create({.device = device});
    }

    memory.vertex_buffer_heap = GPU::memory_heap_create(
        {
            .device = device,
            .heap_usage = GPU::HeapUsage::CPUGPUCoherent,
            .heap_size = mem::align_up(VertexBufferHeapSize, GPU::HeapAlignment),
        }
    );

    memory.vertex_buffer = GPU::buffer_create(
        {
            .device = device,
            .usage = GPU::BufferUsage::VertexBuffer,
            .size = mem::align_up(VertexBufferHeapSize, GPU::MinHeapResourceAlignment),
            .memory_heap = memory.vertex_buffer_heap,
            .heap_offset = 0,
        }
    );

    memory.mapped_vertex_buffer = GPU::buffer_map_memory(memory.vertex_buffer, 0, VertexBufferHeapSize);

    memory.frame_uniform_heap = GPU::memory_heap_create(
        {
            .device = device,
            .heap_usage = GPU::HeapUsage::CPUGPUCoherent,
            .heap_size = mem::align_up(FrameUniformBufferHeapSize, GPU::HeapAlignment),
        }
    );

    memory.frame_uniform_buffer = GPU::buffer_create(
        {
            .device = device,
            .usage = GPU::BufferUsage::UniformBuffer,
            .size = mem::align_up(usize(sizeof(FrameUniformInfo)), GPU::MinHeapResourceAlignment),
            .memory_heap = memory.frame_uniform_heap,
            .heap_offset = 0,
        }
    );

    memory.mapped_frame_uniform_buffer = GPU::buffer_map_memory(memory.frame_uniform_buffer, 0, FrameUniformBufferHeapSize);
    
    _create_pipelines();

    frames_in_flight = get_allocator().array<FrameInFlightInfo>(MaxFrameCount);
    for(usize i = 0; i < frames_in_flight.len; i++)
    {
        frames_in_flight[i] = _create_frame_info(command_pool, i);
    }

    frame_index = 0;

    can_render = true;
}

void SceneRenderer2D::shutdown()
{
    GPU::queue_wait_idle(graphics_queue);
    GPU::queue_wait_idle(present_queue);

    _destroy_pipelines();

    for(usize i = 0; i < frames_in_flight.len; i++)
    {
        _destroy_frame_info(frames_in_flight[i]);
    }
    get_allocator().free(mem::to_bytes(frames_in_flight));

    GPU::buffer_unmap_memory(memory.frame_uniform_buffer, memory.mapped_frame_uniform_buffer);
    GPU::buffer_destroy(memory.frame_uniform_buffer);
    GPU::memory_heap_destroy(memory.frame_uniform_heap);

    GPU::buffer_unmap_memory(memory.vertex_buffer, memory.mapped_vertex_buffer);
    GPU::buffer_destroy(memory.vertex_buffer);
    GPU::memory_heap_destroy(memory.vertex_buffer_heap);

    for(usize i = 0; i < render_image_finish_semaphore.len; i++)
    {
        GPU::semaphore_destroy(render_image_finish_semaphore[i]);
    }
    get_allocator().free(mem::to_bytes(render_image_finish_semaphore));

    GPU::swap_chain_destroy(swap_chain);

    GPU::descriptor_pool_destroy(descriptor_pool);
    GPU::command_pool_destroy(command_pool);
}

void SceneRenderer2D::draw_rect(const Transform2D& transform, const Color& color, const Rect2D& rect)
{
    Unused(transform);
    FrameInFlightInfo& frame_info = frames_in_flight[frame_index];

    QuadInstance& quad = *reinterpret_cast<QuadInstance*>(
        memory.mapped_vertex_buffer.add(frame_info.vertex_heap_offset + frame_info.quad_count * sizeof(QuadInstance)).ptr()
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
    if(can_render == false)
        return;

    FrameInFlightInfo& frame_info = frames_in_flight[frame_index];

    GPU::fence_wait_for(Slice(&frame_info.draw_fence, 1), true, MaxValue<u64>);
    GPU::fence_reset(Slice(&frame_info.draw_fence, 1));

    u32 image_index;
    GPU::swap_chain_acquire_next_image(
        swap_chain,
        {
            .timeout = MaxValue<u64>,
            .semaphore = frame_info.present_semaphore,
            .fence = {},
        },
        &image_index
    );

    FrameUniformInfo& frame_uniform_info = *reinterpret_cast<FrameUniformInfo*>(memory.mapped_frame_uniform_buffer.add(frame_info.frame_uniform_heap_offset).ptr());
    Vector2I window_size = Engine::get_main_window().get_size();

    frame_uniform_info.view = Mat4::identity();
    frame_uniform_info.projection = Projection::orthographic(0, window_size.width, 0, window_size.height, 0, 1);
    frame_uniform_info.view_projection = frame_uniform_info.view * frame_uniform_info.projection;
    
    frame_uniform_info.view.transpose();
    frame_uniform_info.projection.transpose();
    frame_uniform_info.view_projection.transpose();

    GPU::command_buffer_begin(frame_info.command_buffer);
    GPU::command_buffer_begin_renderpass(
        frame_info.command_buffer,
        {
            .size = Engine::get_main_window().get_size(),
            .swap_chain = swap_chain,
            .image_index = image_index,
            .clear_color = Color(0, 0, 0, 255),
        }
    );

    if(frame_info.quad_count != 0)
    {
        GPU::command_buffer_bind_pipeline(
            frame_info.command_buffer, GPU::PipelineBindPoint::Graphics, pipelines.quad_pipeline
        );
        GPU::command_buffer_bind_descriptor_sets(
            frame_info.command_buffer, GPU::PipelineBindPoint::Graphics, 0, Slice(&frame_info.quad_frame_set, 1)
        );

        GPU::command_buffer_bind_vertex_buffers(
            frame_info.command_buffer, 0, Slice(&memory.vertex_buffer, 1),
            Slice(&frame_info.vertex_heap_offset, 1)
        );

        GPU::Viewport viewport =
        {
            .x = 0,
            .y = 0,
            .width = f32(window_size.width),
            .height = f32(window_size.height),
            .min_depth = 0.f,
            .max_depth = 1.f,
        };

        GPU::Scissor scissor =
        {
            .x = 0,
            .y = 0,
            .width = static_cast<u32>(window_size.width),
            .height = static_cast<u32>(window_size.height),
        };

        GPU::command_buffer_set_viewports(frame_info.command_buffer, 0, Slice(&viewport, 1));
        GPU::command_buffer_set_scissors(frame_info.command_buffer, 0, Slice(&scissor, 1));

        GPU::command_buffer_draw(frame_info.command_buffer, 6, frame_info.quad_count, 0, 0);
        frame_info.quad_count = 0;
    }

    GPU::command_buffer_end_renderpass(frame_info.command_buffer, {});
    GPU::command_buffer_end(frame_info.command_buffer);

    GPU::PipelineStages wait_stage = GPU::PipelineStages::RenderOutput;
    GPU::queue_execute_command_buffer(
        graphics_queue,
        {
            .wait_semaphores = Slice(&frame_info.present_semaphore, 1),
            .wait_stages = Slice(&wait_stage, 1),
            .command_buffers = Slice(&frame_info.command_buffer, 1),
            .signal_semaphores = Slice(&render_image_finish_semaphore[image_index], 1),
            .fence = frame_info.draw_fence,
        }
    );

    GPU::queue_present(
        present_queue,
        {
            .wait_semaphores = Slice(&render_image_finish_semaphore[image_index], 1),
            .swapchains = Slice(&swap_chain, 1),
            .image_indices = Slice(&image_index, 1),
        }
    );

    frame_index = (frame_index + 1) % MaxFrameCount;
}

void SceneRenderer2D::_recreate_swap_chain()
{
    GPU::queue_wait_idle(graphics_queue);
    GPU::queue_wait_idle(present_queue);

    can_render = false;
    Vector2I window_size = Engine::get_main_window().get_size();
    if(window_size.width == 0 || window_size.height == 0)
    {
        return;
    }

    if(swap_chain.is_valid())
    {
        GPU::swap_chain_destroy(swap_chain);
        swap_chain = GPU::SwapChainID();
    }
    
    swap_chain = GPU::swap_chain_create(
        {
            .device = device,
            .surface = Engine::get_main_window().get_surface(),
            .present_mode = GPU::PresentMode::Immediate,
            .format = SwapChainFormat,
            .min_image_count = SwapChainMinImageCount,
            .size = Vector2U(window_size),
        }
    );

    can_render = true;
}

SceneRenderer2D::FrameInFlightInfo SceneRenderer2D::_create_frame_info(GPU::CommandPoolID command_pool, u32 frame_index)
{
    FrameInFlightInfo frame_info = {};

    frame_info.command_buffer = GPU::command_buffer_allocate(
        {
            .pool = command_pool,
        }
    );

    frame_info.draw_fence = GPU::fence_create(
        {
            .device = device,
            .signaled = true,
        }
    );

    frame_info.present_semaphore = GPU::semaphore_create(
        {
            .device = device,
        }
    );

    frame_info.quad_frame_set = GPU::descriptor_set_allocate(
        {
            .device = device,
            .pool = descriptor_pool,
            .set_layout = pipelines.quad_layout,
        }
    );

    GPU::DescriptorBufferInfo buffers[] =
    {
        { .buffer = memory.frame_uniform_buffer, .offset = frame_index * sizeof(FrameUniformInfo), .range = sizeof(FrameUniformInfo), },
    };

    GPU::WriteDescriptorInfo write_infos[] =
    {
        { .binding = 0, .array_element = 0, .count = 1, .type = GPU::DescriptorType::UniformBuffer, .textures = {}, .buffers = buffers, },
    };

    GPU::descriptor_set_update_descriptors(
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
    GPU::command_buffer_free(frame_info.command_buffer);
    GPU::fence_destroy(frame_info.draw_fence);
    GPU::semaphore_destroy(frame_info.present_semaphore);
    GPU::descriptor_set_free(frame_info.quad_frame_set);
}

void SceneRenderer2D::_create_pipelines()
{
    {
        GPU::DescriptorBinding quad_bindings[] =
        {
            { .type = GPU::DescriptorType::UniformBuffer, .binding = 0, .count = 1, .stages = GPU::ShaderStage::Vertex },
        };

        pipelines.quad_layout = GPU::descriptor_set_layout_create(
            {
                .device = device,
                .bindings = quad_bindings,
            }
        );
        
        Slice<u8> shader_code = File::read_all(get_allocator(), "shaders/bread/SceneRenderer2D/Quad.slang.spirv");

        GPU::ShaderStageInfo quad_stages[] =
        {
            { .stage = GPU::ShaderStage::Vertex, .code = shader_code, .name = "VertexMain", },
            { .stage = GPU::ShaderStage::Fragment, .code = shader_code, .name = "FragmentMain", },
        };

        GPU::VertexBinding quad_binding[] =
        {
            { .binding = 0, .stride = sizeof(QuadInstance), .input_rate = GPU::InputRate::Instance },
        };

        GPU::VertexAttribute quad_attributes[3] = {};
        for(usize i = 0; i < ArraySize(quad_attributes); i++)
        {
            quad_attributes[i] = 
            {
                .location = static_cast<u32>(i),
                .binding = 0,
                .format = GPU::VertexFormat::RGBA32Float,
                .offset = static_cast<u32>(sizeof(Vector4) * i),
            };
        }

        pipelines.quad_pipeline = GPU::pipeline_create(
            {
                .device = device,
                .bind_point = GPU::PipelineBindPoint::Graphics,
                .shader_stages = quad_stages,
                .vertex_input =
                {
                    .bindings = Slice(quad_binding),
                    .attributes = Slice(quad_attributes),
                },
                .input_assembly =
                {
                    .topology = GPU::PrimitiveTopology::TriangleList,
                },
                .rasterizer_state =
                {
                    .depth_clamp_enable = false,
                    .rasterizer_discard_enable = false,
                    .polygon_mode = GPU::PolygonMode::Fill,
                    .cull_mode = GPU::CullMode::Front,
                    .front_face = GPU::FrontFace::ClockWise,
                    .line_width = 1.f,
                },
                .multisample_state =
                {
                    .sample_count = GPU::SampleCount::Sample1,
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
                    .set_layouts = Slice(&pipelines.quad_layout, 1),
                },
                .surface_format = SwapChainFormat,
            }
        );

        get_allocator().free(shader_code);
    }
}

void SceneRenderer2D::_destroy_pipelines()
{
    GPU::descriptor_set_layout_destroy(pipelines.quad_layout);
    GPU::pipeline_destroy(pipelines.quad_pipeline);
}
