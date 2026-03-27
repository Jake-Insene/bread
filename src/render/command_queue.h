#pragma once
#include "collections/array.h"
#include "collections/stack.h"
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "render/command_encoder.h"


struct RenderDevice;

struct CommandQueue
{
    mem::Allocator allocator;
    
    RenderDevice* render_device;
    GPU::CommandPoolID command_pool;

    struct WorkSubmit
    {
        GPU::FenceID fence;
        CommandEncoder encoder;
    };

    Array<CommandEncoder> encoders;
    Array<GPU::FenceID> work_fences;
    Array<WorkSubmit> work_submited;
    
    Stack<GPU::FenceID> free_fences;
    Stack<CommandEncoder> free_encoders;

    void initialize(const mem::Allocator& _allocator);
    void shutdown();

    CommandEncoder acquire_encoder();
    void execute(const CommandEncoder& encoder);

    void _remove_finished_work();
};