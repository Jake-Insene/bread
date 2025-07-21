#pragma once

#if defined(DEBUG)
#define DebugAssert(cond, ...) \
    if(!(cond))\
    {\
        Log::error(__VA_ARGS__);\
        Debug::breakpoint();\
    }
#else
#define DebugAssert(cond, ...)
#endif

#include "debug/debug.h"
#include "log/log.h"
