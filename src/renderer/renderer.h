#pragma once
#include "math/vec2.h"
#include "math/color.h"
#include "math/rect_2d.h"
#include "graphics/device.h"
#include "render_device/core/gpu_memory_allocator.h"
#include "renderer/framed_buffer.h"


struct GPUMemoryAllocator;

struct RendererCreateInfo
{
    Mem::Allocator* allocator;
    Graphics::Device* graphics_device;
    GPUMemoryAllocator* gpu_memory_allocator;
    u32 max_frames_in_flight;
    Window* target_window;
    GPU::TextureFormat surface_format;
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
    };

    struct RenderFrame
    {
        Graphics::Semaphore* present_complete_semaphore;
        Graphics::Fence* in_flight_fence;
    };

    Mem::Allocator* allocator;
    Graphics::Device* graphics_device;
    Graphics::CommandQueue* command_queue;
    Graphics::SwapChain* swap_chain;

    u32 max_frames_in_flight;
    u32 frame_index;
    FrameInfo current_frame_info;
    
    Array<RenderFrame> frames;
    Array<Graphics::Semaphore*> render_finished_semaphores;

    void init(const RendererCreateInfo& info);
    void destroy();

    void handle_resize();

    FrameInfo begin_frame();
    void end_frame();

    virtual void render(const FrameInfo& frame_info) = 0;
};

EnableBitOp(Renderer::FrameFlags);
