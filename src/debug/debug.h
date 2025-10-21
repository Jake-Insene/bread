#pragma once
#include "core/types.h"


struct Debug
{
    static void assert_info(const char* file_path, usize file_path_len,
        usize line, const char* msg, usize msg_len);

    [[noreturn]] static void breakpoint()
    {
#if defined(BREAD_MSVC)
        __debugbreak();
#elif defined(BREAD_CLANG)
        __builtin_trap();
#endif
    }
    
};

