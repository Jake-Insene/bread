#include "graphics/swap_chain.h"


namespace Graphics
{

void SwapChain::init(const mem::Allocator& _allocator, const SwapChainInfo& info)
{
    allocator = _allocator;

    device = info.device;
    present_queue = info.present_queue;
    window = info.window;
    surface_format = info.surface_format;

    swap_chain = GPU::SwapChainID::invalid();
    images = Array<ImageInfo>::with_size(allocator, 3);
    is_valid_swap_chain = false;
    pending_rebuild = true;

    _rebuild();
}

void SwapChain::destroy()
{
    _free_images();
    images.destroy();

    if(swap_chain.is_valid())
    {
        GPU::swap_chain_destroy(swap_chain);
    }
}

void SwapChain::resize()
{
}

bool SwapChain::acquire_image(u32* image_index, GPU::SemaphoreID present_complete)
{
    if(pending_rebuild == true)
    {
        _rebuild();
    }

    if(is_valid_swap_chain == false && _try_rebuild() == false)
    {
        return false;
    }

    u32 int_index = MaxValue<u32>;
    GPU::AcquireResult result = GPU::swap_chain_acquire_next_image(
        swap_chain,
        {
            .timeout = MaxValue<u64>,
            .semaphore = present_complete,
            .fence = GPU::FenceID::invalid(),
        }, 
        &int_index
    );

    if(result == GPU::AcquireResult::Suboptimal)
    {
        pending_rebuild = true;
        return true;
    }
    else if(result == GPU::AcquireResult::OutOfDate)
    {
        return false;
    }

    *image_index = int_index;
    return true;
}

bool SwapChain::present(Queue& present_queue, u32 image_index, const Slice<GPU::SemaphoreID>& wait_semaphores)
{
    GPU::AcquireResult result = present_queue.present(
        {
            .wait_semaphores = wait_semaphores,
            .swapchains = Slice(&swap_chain, 1),
            .image_indices = Slice(&image_index, 1),
        }
    );

    if(result == GPU::AcquireResult::Suboptimal || result == GPU::AcquireResult::OutOfDate)
    {
        return _try_rebuild();
    }

    return true;
}

void SwapChain::_init_images()
{
    images.resize(GPU::swap_chain_get_image_count(swap_chain));
    for(usize i = 0; i < images.count; i++)
    {
        images.get(i) = 
        {
            .texture = GPU::swap_chain_get_texture(swap_chain, i),
        };
    }
}

void SwapChain::_free_images()
{
    images.clear();
}

void SwapChain::_rebuild()
{
    present_queue.get()->wait_idle();

    _free_images();
    
    if(swap_chain.is_valid())
    {
        is_valid_swap_chain = false;
        pending_rebuild = true;
        GPU::swap_chain_destroy(swap_chain);
        swap_chain = GPU::SwapChainID::invalid();
    }

    Vector2I window_size = window.get_size();
    if(window_size.x == 0 || window_size.y == 0
        || window_size.x < 0 || window_size.y < 0)
    {
        return;
    }

    swap_chain = GPU::swap_chain_create(
        {
            .device = device,
            .surface = window.get_surface(),
            .present_mode = GPU::PresentMode::Immediate,
            .format = surface_format,
            .min_image_count = DefaultMinImageCount,
            .size = Vector2U(window.get_size()),
        }
    );
    is_valid_swap_chain = true;
    pending_rebuild = false;

    _init_images();
}

bool SwapChain::_try_rebuild()
{
    _rebuild();
    return is_valid_swap_chain;
}

}
