#pragma once
#include "core/log.h"
#include "debug/debug.h"

#define Fatal(...) Log::error(__VA_ARGS__); Debug::breakpoint();

#define FailOn(cond, ...) \
    if(cond)\
    {\
        Fatal(__VA_ARGS__);\
    }

#if DEBUG
#define DebugAssert(cond, ...) \
    if(!(cond))\
    {\
        Debug::error(__VA_ARGS__);\
        Debug::breakpoint();\
    }
#else
#define DebugAssert(cond, ...)
#endif // NDEBUG

// Utility
#define Bit(n) (1<<n)
