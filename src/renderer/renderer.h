#pragma once
#include "display/window.h"
#include "graphics/command_pool.h"
#include "graphics/swap_chain.h"
#include "graphics/render_device.h"
#include "renderer/framed_pool.h"


struct GPUMemoryAllocator;

struct RendererCreateInfo
{
    Mem::Allocator* allocator;
    Graphics::RenderDevice* render_device;
    GPUMemoryAllocator* gpu_memory_allocator;
    u32 max_frames_in_flight;
    Graphics::SwapChain* swap_chain;
};

struct Renderer
{
    static constexpr u32 DefaultFramesInFlight = 3;

    enum class FrameFlags
    {
        Acquired = Bit(0),
    };

    struct FrameInfo
    {
        FrameFlags flags;
        u32 frame_index;
        u32 image_index;
        GPU::TextureID image;
        GPU::TextureViewID image_view;

        GPU::DescriptorPoolID pool;
    };

    struct RenderFrame
    {
        GPU::SemaphoreID present_complete_semaphore;
        GPU::FenceID in_flight_fence;
    };

    struct InternalData
    {
        Mem::Allocator* allocator;
        Graphics::RenderDevice* render_device;
        Graphics::CommandPool command_pool;
        Graphics::SwapChain* swap_chain;

        u32 max_frames_in_flight;
        u32 frame_index;
        FrameInfo current_frame_info;
        
        Array<RenderFrame> frames;
        Array<GPU::SemaphoreID> render_finished_semaphores;
        FramedPool frame_pool;
    } data;

    void init(const RendererCreateInfo& info);
    void destroy();

    [[nodiscard]] Graphics::RenderDevice* get_render_device() const { return data.render_device; }

    FrameInfo begin_frame();
    void end_frame();

    GPU::CommandBufferID acquire_command_buffer(const FrameInfo& frame_info);
    void submit_command_buffer(const FrameInfo& frame_info, const Slice<const GPU::PipelineStages>& wait_stages, GPU::CommandBufferID command_buffers);
    void present(const FrameInfo& frame_info);
};

EnableBitOp(Renderer::FrameFlags);
