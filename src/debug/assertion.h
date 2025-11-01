#pragma once
#include "debug/debug.h"


#if defined(DEBUG)
#define DebugAssert(cond, msg, ...) \
    if(!(cond))\
    {\
        Debug::assert_info(__FILE__, sizeof(__FILE__) - 1, __LINE__, msg, sizeof(msg));\
        Debug::breakpoint();\
    }
#else
#define DebugAssert(...)
#endif

