#pragma once
#include "collections/array.h"
#include "gpu/gpu.h"
#include "mem/allocator.h"


struct RenderDevice;

struct CommandEncoder
{
    GPU::CommandBufferID command_buffer;
};