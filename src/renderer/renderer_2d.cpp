#include "renderer/renderer_2d.h"


void Renderer2D::init(const RendererCreateInfo& info)
{
    Renderer::init(info);

    sampler = GPU::sampler_create(
        render_device->get_device(),
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

    scene_uniform_buffer.init(
        {
            .allocator = allocator,
            .device = render_device->get_device(),
            .gpu_memory_allocator = info.gpu_memory_allocator,
            .buffer_size = MaxSceneUniformSize,
            .frame_count = max_frames_in_flight,
            .usage = GPU::BufferUsage::UniformBuffer,
        }
    );

    GPU::DescriptorSetLayoutCreateInfo set_layouts =
    {
        { SceneRenderer::GlobalSceneSet },
    };

    global_set_layout = GPU::descriptor_set_layout_create(render_device->get_device(), set_layouts);

    global_scene_layout = GPU::pipeline_layout_create(render_device->get_device(),
        GPU::PipelineLayoutCreateInfo::create({}, Slice(&global_set_layout, 1))
    );

    GPU::DescriptorPoolSize pool_sizes[] =
    {
        { .type = GPU::DescriptorType::UniformBuffer, .count = u32(max_frames_in_flight), },
    };

    global_scene_pool = GPU::descriptor_pool_create(render_device->get_device(),
        GPU::DescriptorPoolCreateInfo::create(max_frames_in_flight, pool_sizes)
    );

    global_scene_set = allocator->array<GPU::DescriptorSetID>(max_frames_in_flight);
    Slice<GPU::DescriptorSetLayoutID> global_scene_set_layouts = allocator->array<GPU::DescriptorSetLayoutID>(info.max_frames_in_flight);
    for(usize i = 0; i < global_scene_set_layouts.len; i++)
    {
        global_scene_set_layouts[i] = global_set_layout;
    }

    GPU::descriptor_set_allocate(render_device->get_device(),
        {.pool = global_scene_pool, .set_layouts = global_scene_set_layouts},
        global_scene_set
    );

    allocator->free(Mem::to_bytes(global_scene_set_layouts));
}

void Renderer2D::destroy()
{
    GPU::queue_wait_idle(render_device->get_graphics_queue());
    GPU::queue_wait_idle(render_device->get_present_queue());
    
    allocator->free(Mem::to_bytes(global_scene_set));

    GPU::descriptor_set_layout_destroy(global_set_layout);
    GPU::descriptor_set_free(global_scene_pool, global_scene_set);
    GPU::descriptor_pool_destroy(global_scene_pool);
    GPU::pipeline_layout_destroy(global_scene_layout);
    scene_uniform_buffer.destroy();
    GPU::sampler_destroy(sampler);
    Renderer::destroy();
}

SceneRenderer::SceneUniform* Renderer2D::get_scene_uniform(const FrameInfo& frame_info)
{
    return reinterpret_cast<SceneRenderer::SceneUniform*>(scene_uniform_buffer.get_mapped(frame_info.image_index).ptr());
}

GPU::DescriptorSetID Renderer2D::get_global_set(const FrameInfo& frame_info)
{
    return global_scene_set[frame_info.frame_index];
}
