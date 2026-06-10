#pragma once
#include "math/vec2.h"
#include "math/color.h"
#include "math/rect_2d.h"
#include "render_device/render_device.h"
#include "renderer/framed_buffer.h"
#include "renderer/renderer.h"
#include "renderer/scene_renderer.h"


struct SpriteRendererCreateInfo
{
    Mem::Allocator* allocator;
    RenderDevice* render_device;
    GPUMemoryAllocator* gpu_memory_allocator;
    u32 max_frames_in_flight;
    u32 initial_unit_per_batch;
    GPU::TextureFormat surface_format;
};

struct SpriteRenderer
{
    static constexpr usize MaxTexturesPerBatch = 16;

    struct FrameInfo : Renderer::FrameInfo
    {
        GPU::DescriptorSetID global_set;
        Vector2 viewport_size;
    };

    struct alignas(sizeof(Vector4)) StreamSpriteUnit
    {
        // attrib 0
        Vector2 xx;
        Vector2 yy;
        // attrib 1
        Vector2 zz;
        Color color;
        u32 texture_index;
        // attrib 2
        Rect2D rect;
        // attrib 3
        Rect2D uv_rect;
    };
    static_assert(sizeof(StreamSpriteUnit) / sizeof(Vector4) <= GPU::MaxVertexInputAttributes, "Instance shouldn't use more than GPU::MaxVertexInputAttributes attributes");
    static_assert(Mem::align_up(sizeof(StreamSpriteUnit), sizeof(Vector4)) == sizeof(StreamSpriteUnit), "Invalid Instance alignment");
    
    static constexpr usize StreamAttributeCount = sizeof(StreamSpriteUnit)/sizeof(Vector4);

    struct Batch
    {
        GPU::PipelineID pipeline;
        GPU::DescriptorSetID set;
        usize offset; // in stream buffer
        u32 vertices_per_instance;
        u32 instance_count;
        GPU::TextureViewID texture_views[MaxTexturesPerBatch];
        GPU::SamplerID samplers[MaxTexturesPerBatch];
        u32 texture_count;
    };
    // TODO: Allow dynamic batch count
    static constexpr usize MaxBatchesPerFrame = 64;

    Mem::Allocator* allocator;
    RenderDevice* reneder_device;

    GPU::DescriptorSetLayoutID batch_set_layout[2];
    GPU::PipelineLayoutID batch_pipeline_layout;
    GPU::PipelineID sprite_pipeline;

    usize instance_buffer_size;

    FramedDeviceBuffer instance_buffer;
    GPU::DescriptorPoolID descriptor_pool;
    Array<GPU::DescriptorSetID> descriptor_sets;

    Array<Batch> batches;
    Array<StreamSpriteUnit> streams;

    u32 stream_count;

    void init(const SpriteRendererCreateInfo& info);
    void destroy();

    void build_batch(const FrameInfo& frame_info);
    void finish_scene(const FrameInfo& frame_info);

    void begin_batch_record(const FrameInfo& frame_info, GPU::CommandBufferID command_buffer);
    void end_batch_record(const FrameInfo& frame_info, GPU::CommandBufferID command_buffer);

    void commit_sprite(const StreamSpriteUnit& sprite, GPU::TextureViewID texture_view, GPU::SamplerID sampler);
};

