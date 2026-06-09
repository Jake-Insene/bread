#include "renderer/sprite_renderer.h"

#include "graphics/command_buffer.h"
#include "graphics/descriptor_pool.h"
#include "graphics/descriptor_set.h"
#include "graphics/pipeline_layout.h"
#include "graphics/pipeline.h"


void SpriteRenderer::init(const SpriteRendererCreateInfo& info)
{
    allocator = info.allocator;
    graphics_device = info.graphics_device;

    Graphics::Shader shader_code = {};
    shader_code.init(
        allocator,
        {
            .file_path = "shaders/bread/Renderer2D/Instance.Sprite.slang.spirv",
            .vertex_name = "VertexMain",
            .fragment_name = "FragmentMain",
        }
    );

    // same layout for now
    GPU::DescriptorBinding frame_bindings[] =
    {
        { .type = GPU::DescriptorType::CombinedTextureSampler, .binding = 0, .count = MaxTexturesPerBatch, .stages = GPU::ShaderStage::Fragment, },
    };

    Graphics::DescriptorSetLayoutInfo set_layouts[] =
    {
        // Global set
        { SceneRenderer::GlobalSceneSet },
        // Batch set
        {
            .bindings = frame_bindings,
        }
    };

    batch_pipeline_layout = graphics_device->create_pipeline_layout(
        {
            .constant_blocks = {},
            .set_layout_infos = set_layouts,
        }
    );

    GPU::VertexBinding bindings[] =
    {
        { .binding = 0, .stride = sizeof(StreamSpriteUnit), .input_rate = GPU::InputRate::Instance },
    };

    GPU::VertexAttribute attributes[GPU::MaxVertexInputAttributes] = {};
    for(usize atti = 0; atti < StreamAttributeCount; atti++)
    {
        attributes[atti] =
        {
            .location = u32(atti),
            .binding = 0,
            .format = GPU::VertexFormat::RGBA32Float,
            .offset = u32(sizeof(Vector4) * atti),
        };
    }

    GPU::TextureFormat image_format = info.surface_format;
    Graphics::PipelineInfo pipeline_info =
    {
        .bind_point = GPU::PipelineBindPoint::Graphics,
        .shader = &shader_code,
        .vertex_input = GPU::VertexInput::input(bindings, Slice(&attributes[0], StreamAttributeCount)),
        .input_assembly = { .topology = GPU::PrimitiveTopology::TriangleList },
        .rasterizer_state = GPU::RasterizerState::state(
            GPU::PolygonMode::Fill, GPU::CullMode::Front, GPU::FrontFace::ClockWise),
        .multisample_state = GPU::MultisampleState::disable(),
        .depth_stencil_state = GPU::DepthStencilState::depth_stencil_disable(),
        .pipeline_layout = batch_pipeline_layout,
        .rendering_info = GPU::RenderingInfo::render_attachments(Slice(&image_format, 1)),
    };

    sprite_pipeline = graphics_device->create_pipeline(pipeline_info);
    shader_code.destroy();

    instance_buffer_size = sizeof(StreamSpriteUnit) * info.initial_unit_per_batch;

    instance_buffer.init(
        {
            .allocator = allocator,
            .graphics_device = graphics_device,
            .gpu_memory_allocator = info.gpu_memory_allocator,
            .buffer_size = instance_buffer_size,
            .frame_count = info.max_frames_in_flight,
            .usage = GPU::BufferUsage::VertexBuffer,
        }
    );

    GPU::DescriptorPoolSize pool_sizes[] =
    {
        { .type = GPU::DescriptorType::CombinedTextureSampler, .count = u32(MaxTexturesPerBatch * MaxBatchesPerFrame * info.max_frames_in_flight), },
    };

    descriptor_pool = graphics_device->create_descriptor_pool(u32(MaxBatchesPerFrame * info.max_frames_in_flight), pool_sizes);

    const usize max_descriptor_set_count = MaxBatchesPerFrame * info.max_frames_in_flight;
    descriptor_sets = Array<Graphics::DescriptorSet*>::with_size(allocator, max_descriptor_set_count);
    for(usize i = 0; i < max_descriptor_set_count; i++)
    {
        (void)descriptor_sets.add(descriptor_pool->allocate(batch_pipeline_layout->get_layout(1)));
    }

    batches = Array<Batch>::with_size(allocator, 32);

    stream_count = 0;
}

void SpriteRenderer::destroy()
{
    batch_pipeline_layout->destroy();

    sprite_pipeline->destroy();

    instance_buffer.destroy();
    descriptor_pool->destroy();
    descriptor_sets.destroy();

    batches.destroy();
}

void SpriteRenderer::build_batch(const FrameInfo& frame_info)
{
    // build batches
    u8* staging_ptr = instance_buffer.get_mapped_staging(frame_info.frame_index).ptr();

    // Stream sprites
    StreamSpriteUnit* stream_sprites = reinterpret_cast<StreamSpriteUnit*>(staging_ptr);
    Mem::copy(Slice(stream_sprites, stream_count), streams.slice());
    stream_count = streams.count;
    streams.clear();

    // updating batch sets
    usize base_set_index = frame_info.frame_index * MaxBatchesPerFrame;
    usize set_offset = 0;

    //GPU::WriteDescriptorInfo write_texture_sampler_array =
    //{
    //    .descriptor_set = GPU::DescriptorSetID::invalid(),
    //    .binding = 0,
    //    .array_element = 0,
    //    .count = 0,
    //    .type = GPU::DescriptorType::CombinedTextureSampler,
    //};
    // TODO: Update textures
    for (Batch& batch : batches.iter())
    {
        DebugAssert(set_offset < MaxBatchesPerFrame, "not enough batches for scene");

        Graphics::DescriptorSet* set = descriptor_sets.get(base_set_index + set_offset);
        batch.set = set;
        set_offset++;

        if (batch.texture_count > 0)
        {
            //GPU::descriptor_set_update_descriptors(
            //    {
            //        .device = set->parent->gpu_device,
            //        .write_infos = Slice(&write_texture_sampler_array, 1)
            //    }
            //)
            //set->set_combined_texture_sampler_array(0, Slice(batch.texture_views, batch.texture_count), GPU::TextureLayout::ShaderReadOnly, Slice(batch.samplers, batch.texture_count));
        }
        //set->sync_writes();
    }
}

void SpriteRenderer::finish_scene(const FrameInfo&)
{
}

void SpriteRenderer::begin_batch_record(const FrameInfo& frame_info, Graphics::CommandBuffer* command_buffer)
{
    FramedBuffer::BufferInfo vertex_buffer_info = instance_buffer.get_buffer_info(frame_info.frame_index);
    Graphics::Buffer* vb = instance_buffer.get_buffer();
    Graphics::Buffer* svb = instance_buffer.get_staging_buffer();

    // Copy per type
    // sprite
    if(stream_count > 0)
    {
        GPU::BufferCopyRegion region =
        {
            .src_offset = vertex_buffer_info.offset,
            .dest_offset = vertex_buffer_info.offset,
            .size = stream_count * sizeof(StreamSpriteUnit),
        };
        GPU::command_buffer_copy_buffer(command_buffer->gpu_command_buffer,
            {
                .src_buffer = svb->gpu_buffer,
                .dest_buffer = vb->gpu_buffer,
                .copy_regions = Slice(&region, 1),
            }
        );
    }

    stream_count = 0;
}

void SpriteRenderer::end_batch_record(const FrameInfo& frame_info, Graphics::CommandBuffer* command_buffer)
{
    FramedBuffer::BufferInfo vertex_buffer_info = instance_buffer.get_buffer_info(frame_info.frame_index);

    const Graphics::Buffer* vb[] = { instance_buffer.get_buffer() };

    for (const Batch& batch : batches.iter())
    {
        command_buffer->bind_pipeline(GPU::PipelineBindPoint::Graphics, batch.pipeline);
        const Graphics::DescriptorSet* sets[] = { frame_info.global_set, batch.set };
        command_buffer->bind_set(GPU::PipelineBindPoint::Graphics, batch_pipeline_layout, 0, sets);

        usize buffer_offset = vertex_buffer_info.offset + batch.offset;
        command_buffer->bind_vertex_buffers(0, vb, Slice(&buffer_offset, 1));

        command_buffer->draw(batch.vertices_per_instance, batch.instance_count, 0, 0);
    }

    batches.clear();
}

void SpriteRenderer::commit_sprite(const StreamSpriteUnit& sprite, GPU::TextureViewID texture_view, Graphics::Sampler* sampler)
{
    bool need_new_batch = batches.is_empty();
    if (!need_new_batch)
    {
        Batch& last_batch = batches.last();
        bool texture_found = false;
        for (u32 i = 0; i < last_batch.texture_count; i++)
        {
            if (last_batch.texture_views[i] == texture_view && last_batch.samplers[i] == sampler)
            {
                texture_found = true;
                break;
            }
        }
        if (!texture_found && last_batch.texture_count >= MaxTexturesPerBatch)
        {
            need_new_batch = true;
        }
    }

    if (need_new_batch)
    {
        (void)batches.add(
            {
                .pipeline = sprite_pipeline,
                .set = nullptr,
                .offset = (streams.count * sizeof(StreamSpriteUnit)),
                .vertices_per_instance = 6,
                .instance_count = 0,
                .texture_views = {},
                .samplers = {},
                .texture_count = 0,
            }
        );
    }

    Batch& current_batch = batches.last();
    
    StreamSpriteUnit new_sprite = sprite;
    new_sprite.texture_index = MaxValue<u32>;
    for (u32 i = 0; i < current_batch.texture_count; i++)
    {
        if (current_batch.texture_views[i] == texture_view && current_batch.samplers[i] == sampler)
        {
            new_sprite.texture_index = i;
            break;
        }
    }
    if (new_sprite.texture_index == MaxValue<u32>)
    {
        new_sprite.texture_index = current_batch.texture_count;
        current_batch.texture_views[new_sprite.texture_index] = texture_view;
        current_batch.samplers[new_sprite.texture_index] = sampler;
        current_batch.texture_count++;
    }

    (void)streams.add(new_sprite);
    current_batch.instance_count++;
}

