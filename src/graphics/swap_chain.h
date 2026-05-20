#pragma once
#include "collections/array.h"
#include "collections/ptr.h"
#include "display/display.h"
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
    Queue* present_queue;
    Display::WindowID window;
    GPU::TextureFormat surface_format;
};

struct SwapChain : DeviceObject
{
    static constexpr GPU::TextureFormat DefaultSurfaceFormat = GPU::TextureFormat::RGBA8Unorm;
    static constexpr usize DefaultMinImageCount = 3;

    struct ImageInfo
    {
        GPU::TextureID texture;
    };
    
    GPU::DeviceID gpu_device;
    Queue* present_queue;
    Display::WindowID window;
    GPU::TextureFormat surface_format;
    GPU::PresentMode present_mode;
    
    GPU::SwapChainID swap_chain;
    Array<ImageInfo> images;

    bool is_valid_swap_chain;
    bool pending_rebuild;

    void init(mem::Allocator* _allocator, Device* _parent, const SwapChainInfo& info);
    void destroy();

    void resize();

    bool acquire_image(u32* image_index, Semaphore* present_complete);
    bool present(Queue* present_queue, u32 image_index, const Slice<Semaphore*>& wait_semaphores);

    void set_present_mode(GPU::PresentMode new_present_mode);
    [[nodiscard]] GPU::PresentMode get_present_mode() { return present_mode; }

    usize get_image_count() const { return images.count; }
    ImageInfo& get_image(u32 image_index) { return images.get(image_index); }

    void _init_images();
    void _free_images();
    void _rebuild();
    bool _try_rebuild();
};

}
