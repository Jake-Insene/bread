#include "renderer/sprite_renderer.h"

#include "graphics/shader.h"


void SpriteRenderer::init(const SpriteRendererCreateInfo& info)
{
    allocator = info.allocator;
    reneder_device = info.render_device;

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

    const GPU::DescriptorSetLayoutCreateInfo set_layouts[] =
    {
        // Global set
        { SceneRenderer::GlobalSceneSet },
        // Batch set
        {
            .bindings = frame_bindings,
        }
    };

    batch_set_layout[0] = GPU::descriptor_set_layout_create(reneder_device->get_device(),
        GPU::DescriptorSetLayoutCreateInfo::create(set_layouts[0].bindings)
    );

    batch_set_layout[1] = GPU::descriptor_set_layout_create(reneder_device->get_device(),
        GPU::DescriptorSetLayoutCreateInfo::create(set_layouts[1].bindings)
    );

    batch_pipeline_layout = GPU::pipeline_layout_create(reneder_device->get_device(),
        GPU::PipelineLayoutCreateInfo::create({}, batch_set_layout)
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
    GPU::PipelineCreateInfo pipeline_info =
    {
        .bind_point = GPU::PipelineBindPoint::Graphics,
        .shader_stages = shader_code.get_stages(),
        .vertex_input = GPU::VertexInput::input(bindings, Slice(&attributes[0], StreamAttributeCount)),
        .input_assembly = { .topology = GPU::PrimitiveTopology::TriangleList },
        .rasterizer_state = GPU::RasterizerState::state(
            GPU::PolygonMode::Fill, GPU::CullMode::Front, GPU::FrontFace::ClockWise),
        .multisample_state = GPU::MultisampleState::disable(),
        .depth_stencil_state = GPU::DepthStencilState::depth_stencil_disable(),
        .pipeline_layout = batch_pipeline_layout,
        .rendering_info = GPU::RenderingInfo::render_attachments(Slice(&image_format, 1)),
    };

    sprite_pipeline = GPU::pipeline_create(reneder_device->get_device(), pipeline_info);
    shader_code.destroy();

    instance_buffer_size = sizeof(StreamSpriteUnit) * info.initial_unit_per_batch;

    instance_buffer.init(
        {
            .allocator = allocator,
            .device = reneder_device->get_device(),
            .gpu_memory_allocator = info.gpu_memory_allocator,
            .frame_count = info.max_frames_in_flight,
            .buffer_size = instance_buffer_size,
            .usage = GPU::BufferUsage::VertexBuffer,
        }
    );

    GPU::DescriptorPoolSize pool_sizes[] =
    {
        { .type = GPU::DescriptorType::CombinedTextureSampler, .count = u32(MaxTexturesPerBatch * MaxBatchesPerFrame * info.max_frames_in_flight), },
    };

    descriptor_pool = GPU::descriptor_pool_create(reneder_device->get_device(),
        GPU::DescriptorPoolCreateInfo::create(u32(MaxBatchesPerFrame * info.max_frames_in_flight), pool_sizes)
    );

    const usize max_descriptor_set_count = MaxBatchesPerFrame * info.max_frames_in_flight;
    descriptor_sets = Array<GPU::DescriptorSetID>::with_size(allocator, max_descriptor_set_count);
    for(usize i = 0; i < max_descriptor_set_count; i++)
    {
        GPU::DescriptorSetID output_sets[1] = {};
        GPU::DescriptorSetLayoutID set_layout = batch_set_layout[1];

        GPU::descriptor_set_allocate(reneder_device->get_device(),
           {.pool = descriptor_pool, .set_layouts = Slice(&set_layout, 1)},
           output_sets
        );

        (void)descriptor_sets.add(output_sets[0]);
    }

    batches = Array<Batch>::with_size(allocator, 32);

    stream_count = 0;
}

void SpriteRenderer::destroy()
{
    GPU::descriptor_set_layout_destroy(batch_set_layout[0]);
    GPU::descriptor_set_layout_destroy(batch_set_layout[1]);

    GPU::pipeline_layout_destroy(batch_pipeline_layout);
    GPU::pipeline_destroy(sprite_pipeline);

    instance_buffer.destroy();

    GPU::descriptor_set_free(descriptor_pool, descriptor_sets.slice());
    GPU::descriptor_pool_destroy(descriptor_pool);
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

        GPU::DescriptorSetID set = descriptor_sets.get(base_set_index + set_offset);
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

void SpriteRenderer::begin_batch_record(const FrameInfo& frame_info, GPU::CommandBufferID command_buffer)
{
    FramedBuffer::BufferInfo vertex_buffer_info = instance_buffer.get_buffer_info(frame_info.frame_index);
    GPU::BufferID vb = instance_buffer.get_buffer();
    GPU::BufferID svb = instance_buffer.get_staging_buffer();

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
        GPU::command_buffer_copy_buffer(command_buffer,
            {
                .src_buffer = svb,
                .dest_buffer = vb,
                .copy_regions = Slice(&region, 1),
            }
        );
    }

    stream_count = 0;
}

void SpriteRenderer::end_batch_record(const FrameInfo& frame_info, GPU::CommandBufferID command_buffer)
{
    FramedBuffer::BufferInfo vertex_buffer_info = instance_buffer.get_buffer_info(frame_info.frame_index);

    const GPU::BufferID vb[] = { instance_buffer.get_buffer() };

    for (const Batch& batch : batches.iter())
    {
        GPU::command_buffer_bind_pipeline(command_buffer, GPU::PipelineBindPoint::Graphics, batch.pipeline);
        const GPU::DescriptorSetID sets[] = { frame_info.global_set, batch.set };
        GPU::command_buffer_bind_descriptor_sets(command_buffer, GPU::PipelineBindPoint::Graphics, batch_pipeline_layout, 0, sets);

        usize buffer_offset = vertex_buffer_info.offset + batch.offset;
        GPU::command_buffer_bind_vertex_buffers(command_buffer, 0, vb, Slice(&buffer_offset, 1));

        GPU::command_buffer_draw(command_buffer, batch.vertices_per_instance, batch.instance_count, 0, 0);
    }

    batches.clear();
}

void SpriteRenderer::commit_sprite(const StreamSpriteUnit& sprite, GPU::TextureViewID texture_view, GPU::SamplerID sampler)
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
                .set = GPU::DescriptorSetID::invalid(),
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

