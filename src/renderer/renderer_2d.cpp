#include "renderer/renderer_2d.h"

#include "graphics/descriptor_pool.h"
#include "graphics/descriptor_set.h"
#include "graphics/pipeline_layout.h"


void Renderer2D::init(const RendererCreateInfo& info)
{
    Renderer::init(info);

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

    scene_uniform_buffer.init(
        {
            .allocator = allocator,
            .graphics_device = graphics_device,
            .gpu_memory_allocator = info.gpu_memory_allocator,
            .buffer_size = MaxSceneUniformSize,
            .frame_count = info.max_frames_in_flight,
            .usage = GPU::BufferUsage::UniformBuffer,
        }
    );

    Graphics::DescriptorSetLayoutInfo set_layouts[] =
    {
        { SceneRenderer::GlobalSceneSet },
    };

    global_scene_layout = info.graphics_device->create_pipeline_layout(
        {
            .constant_blocks = {},
            .set_layout_infos = set_layouts,
        }
    );

    GPU::DescriptorPoolSize pool_sizes[] =
    {
        { .type = GPU::DescriptorType::UniformBuffer, .count = u32(info.max_frames_in_flight), },
    };

    global_scene_pool = graphics_device->create_descriptor_pool(info.max_frames_in_flight, pool_sizes);

    global_scene_set = allocator->array<Graphics::DescriptorSet*>(info.max_frames_in_flight);

    for(usize i = 0; i < info.max_frames_in_flight; i++)
    {
        global_scene_set[i] = (global_scene_pool->allocate(global_scene_layout->get_layout(0)));
        // TODO: Setup global scene set
        //global_scene_set[i]->set_uniform_buffer(
        //    0, scene_uniform_buffer.get_buffer(),
        //    scene_uniform_buffer.get_buffer_info(i).offset,
        //    MaxSceneUniformSize
        //);
    }
}

void Renderer2D::destroy()
{
    graphics_device->get_graphics_queue()->wait_idle();
    graphics_device->get_present_queue()->wait_idle();
    
    allocator->free(Mem::to_bytes(global_scene_set));
    global_scene_pool->destroy();
    global_scene_layout->destroy();
    scene_uniform_buffer.destroy();
    sampler->destroy();
    Renderer::destroy();
}

SceneRenderer::SceneUniform* Renderer2D::get_scene_uniform(const FrameInfo& frame_info)
{
    return reinterpret_cast<SceneRenderer::SceneUniform*>(scene_uniform_buffer.get_mapped(frame_info.image_index).ptr());
}

Graphics::DescriptorSet* Renderer2D::get_global_set(const FrameInfo& frame_info)
{
    return global_scene_set[frame_info.frame_index];
}
