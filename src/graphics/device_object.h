#pragma once
#include "mem/allocator.h"


namespace Graphics
{

struct Device;
struct Queue;
struct SwapChain;
struct Fence;
struct Semaphore;
struct MemoryHeap;
struct Buffer;
struct Sampler;
struct Texture;
struct TextureView;
struct DescriptorPool;
struct DescriptorSet;
struct PipelineLayout;
struct Pipeline;
struct CommandPool;
struct CommandBuffer;

struct DeviceObject
{
    Device* parent;
    Mem::Allocator* allocator;

    void init(Mem::Allocator* _allocator, Device* _parent);
    void destroy();
};

}
