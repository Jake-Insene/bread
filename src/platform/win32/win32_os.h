#pragma once
#include "mem/allocator.h"
#include "os/os.h"
#include "platform/platform_header.h"



struct Win32OS
{
    struct InternalData
    {
        Mem::Allocator& allocator;

        i64 frequency;
        f64 program_start;
        usize page_size;

        InternalData(Mem::Allocator& allocator)
        : allocator(allocator), frequency(), program_start(), page_size()
        {}
    };

    [[nodiscard]] static Mem::Allocator& get_allocator();
};
