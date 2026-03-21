#pragma once
#include "collections/array.h"
#include "graphics/graphics.h"
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

    static constexpr Graphics::SurfaceFormat SwapChainFormat = Graphics::SurfaceFormat::RGBA8Srgb;
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
        Graphics::CommandBufferID command_buffer;
        Graphics::FenceID draw_fence;
        Graphics::SemaphoreID present_semaphore;

        Graphics::BufferID frame_ub;
        Graphics::DescriptorSetID quad_frame_set;
        usize quad_count;
        
        usize vertex_heap_offset;
        usize frame_uniform_heap_offset;

        FrameUniformInfo frame_uniform_info;
    };

    mem::Allocator allocator;

    Graphics::DeviceID device;
    Graphics::QueueID graphics_queue;
    Graphics::QueueID present_queue;

    Graphics::CommandPoolID command_pool;
    
    Graphics::SwapChainID swap_chain;
    Slice<Graphics::SemaphoreID> render_image_finish_semaphore;
    
    struct
    {
        Graphics::MemoryHeapID vertex_buffer_heap;
        Graphics::BufferID vertex_buffer;
        Slice<u8> mapped_vertex_buffer;

        Graphics::MemoryHeapID frame_uniform_heap;
        Graphics::BufferID frame_uniform_buffer;
        Slice<u8> mapped_frame_uniform_buffer;
    } memory;

    struct
    {
        Graphics::DescriptorSetLayoutID quad_layout;
        Graphics::PipelineID quad_pipeline;
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

    FrameInFlightInfo _create_frame_info(Graphics::CommandPoolID command_pool, u32 frame_index);
    void _destroy_frame_info(FrameInFlightInfo& frame_info);

    void _create_pipelines();
    void _destroy_pipelines();
};

