#pragma once
#include "math/mat4.h"
#include "math/vec2.h"
#include "math/color.h"
#include "math/rect_2d.h"
#include "graphics/pipeline_2d.h"
#include "renderer/framed_buffer.h"
#include "renderer/framed_pool.h"
#include "renderer/renderer.h"


struct RendererBatch2DCreateInfo
{
    mem::Allocator allocator;
    Graphics::Device* graphics_device;
    GPUMemoryAllocator* gpu_memory_allocator;
    u32 max_frames_in_flight;
    // this is more like max number of instances in a frame per type instead of a batch
    u32 max_instances_per_type;
    GPU::TextureFormat surface_format;
};

struct RendererBatch2D
{
    enum class BatchType
    {
        Unknown = 0,
        Sprite,
        Quad,
        Line,
        Circle,
    };

    struct FrameInfo : Renderer::FrameInfo
    {
        Vector2 viewport_size;
    };

    struct alignas(sizeof(Vector4)) SpriteInstance
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
    static_assert(sizeof(SpriteInstance) / sizeof(Vector4) <= GPU::MaxVertexInputAttributes, "Instance shouldn't use more than GPU::MaxVertexInputAttributes attributes");
    static_assert(mem::align_up(sizeof(SpriteInstance), sizeof(Vector4)) == sizeof(SpriteInstance), "Invalid Instance alignment");
    
    struct alignas(sizeof(Vector4)) QuadInstance
    {
        // attrib 0
        Vector2 xx;
        Vector2 yy;
        // attrib 1
        Vector2 zz;
        Color color;
        u32 material_index;
        // attrib 2
        Rect2D rect;
    };
    static_assert(sizeof(QuadInstance) / sizeof(Vector4) <= GPU::MaxVertexInputAttributes, "Instance shouldn't use more than GPU::MaxVertexInputAttributes attributes");
    static_assert(mem::align_up(sizeof(QuadInstance), sizeof(Vector4)) == sizeof(QuadInstance), "Invalid Instance alignment");

    struct alignas(sizeof(Vector4)) LineInstance
    {
        // attrib 0
        Vector2 xx;
        Vector2 yy;
        // attrib 1
        Vector2 zz;
        Color color;
        u32 material_index;
        // attrib 2
        Vector2 point1;
        Vector2 point2;
    };
    static_assert(sizeof(LineInstance) / sizeof(Vector4) <= GPU::MaxVertexInputAttributes, "Instance shouldn't use more than GPU::MaxVertexInputAttributes attributes");
    static_assert(mem::align_up(sizeof(LineInstance), sizeof(Vector4)) == sizeof(LineInstance), "Invalid Instance alignment");

    struct alignas(sizeof(Vector4)) CircleInstance
    {
        // attrib 0
        Vector2 xx;
        Vector2 yy;
        // attrib 1
        Vector2 zz;
        Color color;
        u32 material_index;
        // attrib 2
        Vector2 point;
        f32 radius;
    };
    static_assert(sizeof(CircleInstance) / sizeof(Vector4) <= GPU::MaxVertexInputAttributes, "Instance shouldn't use more than GPU::MaxVertexInputAttributes attributes");
    static_assert(mem::align_up(sizeof(CircleInstance), sizeof(Vector4)) == sizeof(CircleInstance), "Invalid Instance alignment");
    
    struct Batch
    {
        BatchType batch_type;
        Graphics::Pipeline* pipeline;
        Graphics::DescriptorSet* set;
        usize offset; // in instance buffer
        u32 vertices_per_instance;
        u32 instance_count;
        GPU::TextureID textures[16];
        Graphics::Sampler* samplers[16];
        u32 texture_count;
    };
    static constexpr usize MaxInstancePerBatch = 128;
    static constexpr usize MaxBatchesPerFrame = 64;

    mem::Allocator allocator;
    Graphics::Device* graphics_device;

    Graphics::PipelineLayout* batch_pipeline_layout;
    Graphics::Pipeline* sprite_pipeline;
    Graphics::Pipeline* quad_pipeline;
    Graphics::Pipeline* line_pipeline;
    Graphics::Pipeline* circle_pipeline;

    usize instance_buffer_size;
    usize sprite_offset_begin;
    usize sprite_offset_end;
    usize quad_offset_begin;
    usize quad_offset_end;
    usize line_offset_begin;
    usize line_offset_end;
    usize circle_offset_begin;
    usize circle_offset_end;

    FramedDeviceBuffer instance_buffer;
    FramedMappedBuffer uniform_buffer;
    Graphics::DescriptorPool* descriptor_pool;
    Array<Graphics::DescriptorSetRef> descriptor_sets;

    Array<Batch> batches;

    u32 sprite_count;
    u32 quad_count;
    u32 line_count;
    u32 circle_count;

    Array<SpriteInstance> sprites;
    Array<QuadInstance> quads;
    Array<LineInstance> lines;
    Array<CircleInstance> circles;

    BatchType last_batch_type;

    void init(const RendererBatch2DCreateInfo& batch_info);
    void destroy();

    void prepare_scene(const FrameInfo& frame_info);
    void build_batch(const FrameInfo& frame_info);
    void finish_scene(const FrameInfo& frame_info);

    void begin_batch_record(const FrameInfo& frame_info, Graphics::CommandEncoder& encoder);
    void end_batch_record(const FrameInfo& frame_info, Graphics::CommandEncoder& encoder);

    void commit_sprite(const SpriteInstance& sprite, GPU::TextureID texture, Graphics::Sampler* sampler);
    void commit_quad(const QuadInstance& quad);
    void commit_line(const LineInstance& line);
    void commit_circle(const CircleInstance& circle);
};

