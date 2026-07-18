#pragma once
#include "mem/allocator.h"
#include "os/os.h"
#include "platform/platform_header.h"



struct Win32OS
{
    struct InternalData
    {
        Mem::Allocator* allocator;

        i64 frequency;
        f64 program_start;
        usize page_size;
    };

    static inline Win32OS::InternalData data;

    [[nodiscard]] static Mem::Allocator* get_allocator() { return data.allocator; }
};
