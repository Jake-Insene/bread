#pragma once
#include "collections/array.h"
#include "display/display.h"
#include "gpu/gpu.h"


namespace Graphics
{

struct SwapChainInfo
{
    Mem::Allocator* allocator;
    GPU::DeviceID device;
    GPU::QueueID present_queue;
    Display::WindowID window;
    GPU::TextureFormat surface_format;
};

struct SwapChain
{
    static constexpr GPU::TextureFormat DefaultSurfaceFormat = GPU::TextureFormat::RGBA8Srgb;
    static constexpr usize DefaultMinImageCount = 3;

    struct ImageInfo
    {
        GPU::TextureID image;
        GPU::TextureViewID image_view;
    };

    struct InternalData
    {
        Mem::Allocator* allocator;
        GPU::DeviceID device;
        GPU::QueueID present_queue;
        Display::WindowID window;
        GPU::TextureFormat surface_format;
        GPU::PresentMode present_mode;
        
        GPU::SwapChainID swap_chain;
        Array<ImageInfo> images;

        bool is_valid_swap_chain;
        bool pending_rebuild;
    } data;

    void init(const SwapChainInfo& info);
    void destroy();

    void resize();

    bool acquire_image(u32* image_index, GPU::SemaphoreID present_complete);
    bool present(u32 image_index, const Slice<const GPU::SemaphoreID>& wait_semaphores);

    void set_present_mode(GPU::PresentMode new_present_mode);
    [[nodiscard]] GPU::PresentMode get_present_mode() const { return data.present_mode; }

    usize get_image_count() const { return data.images.count; }
    ImageInfo& get_image(u32 image_index) { return data.images.get(image_index); }

    void _init_images();
    void _free_images();
    void _rebuild();
    bool _try_rebuild();
};

}
