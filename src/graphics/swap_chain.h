#pragma once
#include "collections/array.h"
#include "collections/ptr.h"
#include "display/window.h"
#include "gpu/gpu.h"
#include "graphics/device_object.h"
#include "graphics/queue.h"
#include "graphics/semaphore.h"
#include "mem/allocator.h"


namespace Graphics
{

struct SwapChainInfo
{
    GPU::DeviceID gpu_device;
    Ptr<Queue> present_queue;
    Window window;
    GPU::SurfaceFormat surface_format;
};

struct SwapChain : DeviceObject
{
    static constexpr usize DefaultMinImageCount = 3;

    struct ImageInfo
    {
        GPU::TextureID texture;
    };
    
    GPU::DeviceID gpu_device;
    Ptr<Queue> present_queue;
    Window window;
    GPU::SurfaceFormat surface_format;
    
    GPU::SwapChainID swap_chain;
    Array<ImageInfo> images;

    bool is_valid_swap_chain;
    bool pending_rebuild;

    void init(const mem::Allocator& _allocator, Device* _parent, const SwapChainInfo& info);
    void destroy();

    void resize();

    bool acquire_image(u32* image_index, Ptr<Semaphore> present_complete);
    bool present(Queue& present_queue, u32 image_index, const Slice<Ptr<Semaphore>>& wait_semaphores);

    usize get_image_count() { return images.count; }
    ImageInfo& get_image(u32 image_index) { return images.get(image_index); }

    void _init_images();
    void _free_images();
    void _rebuild();
    bool _try_rebuild();
};

}
