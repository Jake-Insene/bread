#pragma once
#include "collections/array.h"
#include "collections/ptr.h"
#include "display/window.h"
#include "gpu/gpu.h"
#include "graphics/queue.h"
#include "mem/allocator.h"


namespace Graphics
{

struct SwapChainInfo
{
    GPU::DeviceID device;
    Ptr<Queue> present_queue;
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
    Ptr<Queue> present_queue;
    GPU::QueueID last_submited_queue;
    Window window;
    GPU::SurfaceFormat surface_format;
    
    GPU::SwapChainID swap_chain;
    Array<ImageInfo> images;

    bool is_valid_swap_chain;
    bool pending_rebuild;

    void init(const mem::Allocator& _allocator, const SwapChainInfo& info);
    void destroy();

    void resize();

    bool acquire_image(u32* image_index, GPU::SemaphoreID present_complete);
    bool present(Queue& present_queue, u32 image_index, const Slice<GPU::SemaphoreID>& wait_semaphores);

    usize get_image_count() { return images.count; }
    ImageInfo& get_image(u32 image_index) { return images.get(image_index); }

    void _init_images();
    void _free_images();
    void _rebuild();
    bool _try_rebuild();
};

}
