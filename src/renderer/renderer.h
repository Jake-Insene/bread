#pragma once
#include "math/mat4.h"
#include "math/vec2.h"
#include "math/color.h"
#include "math/rect_2d.h"
#include "graphics/device.h"
#include "render_device/core/gpu_memory_allocator.h"
#include "renderer/framed_buffer.h"


struct GPUMemoryAllocator;

struct RendererCreateInfo
{
    mem::Allocator allocator;
    Graphics::Device* graphics_device;
    GPUMemoryAllocator* gpu_memory_allocator;
    u32 max_frames_in_flight;
    Window target_window;
    GPU::TextureFormat surface_format;
};

struct Renderer
{
    static constexpr usize MaxFramesInFlight = 3;

    struct FrameInfo
    {
        u32 frame_index;
        u32 image_index;
        GPU::TextureID image;
    };

    struct RenderFrame
    {
        Graphics::Semaphore* present_complete_semaphore;
        Graphics::Fence* in_flight_fence;
    };

    mem::Allocator allocator;
    Graphics::Device* graphics_device;
    Graphics::CommandQueue* command_queue;
    Graphics::SwapChain* swap_chain;

    u32 max_frames_in_flight;
    u32 frame_index;
    bool frame_was_acquired;
    FrameInfo current_frame_info;
    
    Array<RenderFrame> frames;
    Array<Graphics::Semaphore*> render_finished_semaphores;

    void init(const RendererCreateInfo& info);
    void destroy();

    FrameInfo get_current_frame_info() const;

    void handle_resize();

    void begin_frame();
    void end_frame();

    virtual void render() = 0;
};
