#include "graphics/swap_chain.h"


namespace Graphics
{

void SwapChain::init(const SwapChainInfo& info)
{
    data.allocator = info.allocator;
    data.device = info.device;
    data.present_queue = info.present_queue;
    data.window = info.window;
    data.surface_format = info.surface_format;
    data.present_mode = GPU::PresentMode::VSync;

    data.swap_chain = GPU::SwapChainID::invalid();
    data.images = Array<ImageInfo>::with_size(info.allocator, 3);
    data.is_valid_swap_chain = false;
    data.pending_rebuild = true;

    _rebuild();
}

void SwapChain::destroy()
{
    _free_images();
    data.images.destroy();

    if(data.swap_chain.is_valid())
    {
        GPU::swap_chain_destroy(data.swap_chain);
    }
}

void SwapChain::resize()
{
    _try_rebuild();
}

bool SwapChain::acquire_image(u32* image_index, GPU::SemaphoreID present_complete)
{
    if(data.pending_rebuild)
    {
        _rebuild();
    }

    if(!data.is_valid_swap_chain && !_try_rebuild())
    {
        return false;
    }

    GPU::AcquireResult result = GPU::swap_chain_acquire_next_image(
        data.swap_chain,
        {
            .timeout = MaxValue<u64>,
            .semaphore = present_complete,
            .fence = GPU::FenceID::invalid(),
        }, 
        image_index
    );

    if(result == GPU::AcquireResult::Suboptimal)
    {
        data.pending_rebuild = true;
        return true;
    }
    
    if(result == GPU::AcquireResult::OutOfDate)
    {
        return false;
    }

    return true;
}

bool SwapChain::present(u32 image_index, const Slice<const GPU::SemaphoreID>& wait_semaphores)
{
    GPU::AcquireResult result = GPU::queue_present(
        data.present_queue,
        {
            .wait_semaphores = wait_semaphores,
            .swapchains = Slice(&data.swap_chain, 1),
            .image_indices = Slice(&image_index, 1),
        }
    );

    if(result == GPU::AcquireResult::Suboptimal || result == GPU::AcquireResult::OutOfDate)
    {
        return _try_rebuild();
    }

    return true;
}

void SwapChain::set_present_mode(GPU::PresentMode new_present_mode)
{
    if(data.present_mode != new_present_mode)
    {
        data.present_mode = new_present_mode;
        data.pending_rebuild = true;
    }
}

void SwapChain::_init_images()
{
    data.images.resize(GPU::swap_chain_get_image_count(data.swap_chain));
    for(usize i = 0; i < data.images.count; i++)
    {
        data.images.get(i) = 
        {
            .image = GPU::swap_chain_get_image(data.swap_chain, i),
            .image_view = GPU::swap_chain_get_image_view(data.swap_chain, i),
        };
    }
}

void SwapChain::_free_images()
{
    data.images.clear();
}

void SwapChain::_rebuild()
{
    GPU::queue_wait_idle(data.present_queue);

    _free_images();
    
    if(data.swap_chain.is_valid())
    {
        data.is_valid_swap_chain = false;
        data.pending_rebuild = true;
        GPU::swap_chain_destroy(data.swap_chain);
        data.swap_chain = GPU::SwapChainID::invalid();
    }

    Vector2I window_size = Display::window_get_size(data.window);
    if(window_size.x == 0 || window_size.y == 0
        || window_size.x < 0 || window_size.y < 0)
    {
        return;
    }

    data.swap_chain = GPU::swap_chain_create(
        data.device,
        {
            .surface = Display::window_get_surface(data.window),
            .present_mode = data.present_mode,
            .format = data.surface_format,
            .min_image_count = DefaultMinImageCount,
            .size = Vector2U(window_size),
        }
    );
    data.is_valid_swap_chain = true;
    data.pending_rebuild = false;

    _init_images();
}

bool SwapChain::_try_rebuild()
{
    _rebuild();
    return data.is_valid_swap_chain;
}

}
