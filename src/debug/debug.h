#pragma once

struct Debug
{
    
    [[noreturn]] static void breakpoint()
    {
#if defined(BREAD_WIN32)
        __debugbreak();
#elif defined(BREAD_ANDROID)
        __builtin_trap();
#endif
    }
    
};

#if defined(SHOW_DEBUG_INFO)
#define DebugInfo(...) Log::info(__VA_ARGS__)
#else
#define DebugInfo(...)
#endif

#include "log/log.h"
