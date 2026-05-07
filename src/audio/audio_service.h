#pragma once
#include "mem/allocator.h"
#include "os/atomic.h"
#include "os/thread.h"


struct AudioServiceCreateInfo
{
    mem::Allocator allocator;
};

struct AudioService
{
    struct InternalData
    {
        mem::Allocator allocator;

        Atomic<bool> request_destroy;

        Thread output_thread;
    } data;

    void initialize(const AudioServiceCreateInfo& info);
    void shutdown();

    void tick();
};
