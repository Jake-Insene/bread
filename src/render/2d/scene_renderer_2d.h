#pragma once
#include "collections/array.h"
#include "gpu/gpu.h"
#include "math/color.h"
#include "math/transform_2d.h"
#include "math/projection.h"
#include "systems/system.h"



struct SceneRenderer2D : System<SceneRenderer2D>
{
    static constexpr SystemDependency Dependencies[] =
    {
        SystemDependency::of("RenderDevice")
    };

    static constexpr StringView _name = "SceneRenderer2D";
    static constexpr SystemInfo get_system_info()
    {
        return System::get_system_info_with_name(_name);
    }

    static constexpr GPU::SurfaceFormat SwapChainFormat = GPU::SurfaceFormat::RGBA8Srgb;
    static constexpr usize SwapChainMinImageCount = 3;

    struct QuadInstance
    {
        // attib 0
        Vector2 xx;
        Vector2 yy;
        // attrib 1
        Vector2 zz;
        Color color;
        u32 material_index;
        // attrib 2
        Rect2D rect;
    };

    struct InstanceData
    {
        // attrib 0
        Vector2 xx;
        Vector2 yy;
        // attrib 1
        Vector2 zz;
        Color color;
        u32 texture_index;
    };

    static constexpr usize MaxQuadInstancePerBatch = 1 << 14;
    static constexpr usize MaxQuadInstancePerDrawCall = MaxQuadInstancePerBatch >> 4;
    static constexpr usize QuadInstancePerFrameSize = MaxQuadInstancePerBatch * sizeof(QuadInstance);
    static constexpr usize MaxFrameCount = 3;
    static constexpr usize VertexBufferHeapSize = MaxFrameCount * QuadInstancePerFrameSize;

    struct FrameUniformInfo
    {
        Mat4 view;
        Mat4 projection;
        Mat4 view_projection;
    };

    static constexpr usize FrameUniformBufferHeapSize = MaxFrameCount * sizeof(FrameUniformInfo);

    struct FrameInFlightInfo
    {
        GPU::CommandBufferID command_buffer;
        GPU::FenceID draw_fence;
        GPU::SemaphoreID present_semaphore;

        GPU::BufferID frame_ub;
        GPU::DescriptorSetID quad_frame_set;
        usize quad_count;
        
        usize vertex_heap_offset;
        usize frame_uniform_heap_offset;

        FrameUniformInfo frame_uniform_info;
    };

    mem::Allocator allocator;

    GPU::DeviceID device;
    GPU::QueueID graphics_queue;
    GPU::QueueID present_queue;

    GPU::DescriptorPoolID descriptor_pool;
    GPU::CommandPoolID command_pool;
    
    GPU::SwapChainID swap_chain;
    Slice<GPU::SemaphoreID> render_image_finish_semaphore;
    
    struct
    {
        GPU::MemoryHeapID vertex_buffer_heap;
        GPU::BufferID vertex_buffer;
        Slice<u8> mapped_vertex_buffer;

        GPU::MemoryHeapID frame_uniform_heap;
        GPU::BufferID frame_uniform_buffer;
        Slice<u8> mapped_frame_uniform_buffer;
    } memory;

    struct
    {
        GPU::DescriptorSetLayoutID quad_layout;
        GPU::PipelineID quad_pipeline;
    } pipelines;

    Slice<FrameInFlightInfo> frames_in_flight;

    u32 frame_index;
    bool can_render;

    [[nodiscard]] mem::Allocator get_allocator() { return allocator; }

    void initialize(const SystemInitializeInfo& info);
    void shutdown();

    void draw_rect(const Transform2D& transform, const Color& color, const Rect2D& rect);

    void dispatch();

    void _recreate_swap_chain();
    void _disable_rendering() { can_render = false; }

    FrameInFlightInfo _create_frame_info(GPU::CommandPoolID command_pool, u32 frame_index);
    void _destroy_frame_info(FrameInFlightInfo& frame_info);

    void _create_pipelines();
    void _destroy_pipelines();
};

