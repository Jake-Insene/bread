#pragma once
#include "core/log.h"
#include "debug/debug.h"

#define DEBUG (NDEBUG != 1)

#define Fatal(...) Log::error(__VA_ARGS__); Debug::breakpoint();

#define FailOn(cond, ...) \
    if(cond)\
    {\
        Fatal(__VA_ARGS__);\
    }

#if defined(NDEBUG)
#define DebugAssert(cond, ...)
#else
#define DebugAssert(cond, ...) \
    if(!(cond))\
    {\
        Log::error(__VA_ARGS__);\
        Debug::breakpoint();\
    }
#endif // NDEBUG

// Utility
#define Bit(n) (1<<n)
