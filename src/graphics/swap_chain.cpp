#include "graphics/swap_chain.h"


namespace Graphics
{

void SwapChain::init(mem::Allocator* _allocator, Device* _parent, const SwapChainInfo& info)
{
    DeviceObject::init(_allocator, _parent);
    gpu_device = info.gpu_device;
    present_queue = info.present_queue;
    window = info.window;
    surface_format = info.surface_format;
    present_mode = GPU::PresentMode::VSync;

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
    DeviceObject::destroy();
}

void SwapChain::resize()
{
    _try_rebuild();
}

bool SwapChain::acquire_image(u32* image_index, Graphics::Semaphore* present_complete)
{
    if(pending_rebuild)
    {
        _rebuild();
    }

    if(!is_valid_swap_chain && !_try_rebuild())
    {
        return false;
    }

    u32 int_index = MaxValue<u32>;
    GPU::AcquireResult result = GPU::swap_chain_acquire_next_image(
        swap_chain,
        {
            .timeout = MaxValue<u64>,
            .semaphore = present_complete->gpu_semaphore,
            .fence = GPU::FenceID::invalid(),
        }, 
        &int_index
    );

    if(result == GPU::AcquireResult::Suboptimal)
    {
        pending_rebuild = true;
        return true;
    }
    
    if(result == GPU::AcquireResult::OutOfDate)
    {
        return false;
    }

    *image_index = int_index;
    return true;
}

bool SwapChain::present(Queue* present_queue, u32 image_index, const Slice<Semaphore*>& wait_semaphores)
{
    Slice<GPU::SemaphoreID> gpu_wait_semaphores = allocator->array<GPU::SemaphoreID>(wait_semaphores.len);
    for(usize i = 0; i < gpu_wait_semaphores.len; i++)
    {
        gpu_wait_semaphores[i] = wait_semaphores[i]->gpu_semaphore;
    }

    GPU::AcquireResult result = present_queue->present(
        {
            .wait_semaphores = gpu_wait_semaphores,
            .swapchains = Slice(&swap_chain, 1),
            .image_indices = Slice(&image_index, 1),
        }
    );

    allocator->free(mem::to_bytes(gpu_wait_semaphores));
    if(result == GPU::AcquireResult::Suboptimal || result == GPU::AcquireResult::OutOfDate)
    {
        return _try_rebuild();
    }

    return true;
}

void SwapChain::set_present_mode(GPU::PresentMode new_present_mode)
{
    if(present_mode != new_present_mode)
    {
        present_mode = new_present_mode;
        pending_rebuild = true;
    }
}

void SwapChain::_init_images()
{
    images.resize(GPU::swap_chain_get_image_count(swap_chain));
    for(usize i = 0; i < images.count; i++)
    {
        images.get(i) = 
        {
            .texture = GPU::swap_chain_get_image(swap_chain, i),
        };
    }
}

void SwapChain::_free_images()
{
    images.clear();
}

void SwapChain::_rebuild()
{
    present_queue->wait_idle();

    _free_images();
    
    if(swap_chain.is_valid())
    {
        is_valid_swap_chain = false;
        pending_rebuild = true;
        GPU::swap_chain_destroy(swap_chain);
        swap_chain = GPU::SwapChainID::invalid();
    }

    Vector2I window_size = Display::window_get_size(window);
    if(window_size.x == 0 || window_size.y == 0
        || window_size.x < 0 || window_size.y < 0)
    {
        return;
    }

    swap_chain = GPU::swap_chain_create(
        {
            .device = gpu_device,
            .surface = Display::window_get_surface(window),
            .present_mode = present_mode,
            .format = surface_format,
            .min_image_count = DefaultMinImageCount,
            .size = Vector2U(window_size),
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
