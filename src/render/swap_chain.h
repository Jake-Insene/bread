#pragma once
#include "collections/array.h"
#include "display/window.h"
#include "gpu/gpu.h"
#include "mem/allocator.h"


struct SwapChainInfo
{
    GPU::DeviceID device;
    GPU::QueueID present_queue;
    Window window;
    GPU::SurfaceFormat surface_format;
};

struct SwapChain
{
    static constexpr usize DefaultMinImageCount = 3;

    struct ImageInfo
    {
        GPU::TextureID texture;
    };
    
    mem::Allocator allocator;
    
    GPU::DeviceID device;
    GPU::QueueID present_queue;
    Window window;
    GPU::SurfaceFormat surface_format;
    
    GPU::SwapChainID swap_chain;
    Array<ImageInfo> images;

    bool is_valid_swap_chain;
    bool pending_rebuild;

    static SwapChain create(Window window, GPU::SurfaceFormat surface_format);

    void init(const mem::Allocator& _allocator, const SwapChainInfo& info);
    void destroy();

    void resize();

    bool acquire_image(u32* image_index, GPU::SemaphoreID present_complete);
    bool present(u32 image_index, const Slice<GPU::SemaphoreID>& wait_semaphores);

    usize get_image_count() { return images.count; }
    ImageInfo& get_image(u32 image_index) { return images.get(image_index); }

    void _init_images();
    void _free_images();
    void _rebuild();
    bool _try_rebuild();
};
