#pragma once
#include "core/log.h"

#include <cstdlib>

#define DEBUG (NDEBUG != 1)

#define Fatal(...) Log::error(__VA_ARGS__); std::abort()

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
        Fatal(__VA_ARGS__);\
    }
    
#endif // NDEBUG

// Utility
#define Bit(n) (1<<n)
