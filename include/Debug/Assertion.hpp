#pragma once
#include "Core/Types.hpp"


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


#if defined(DEBUG) || defined(BREAD_ENABLE_ASSERTIONS)
#define DebugAssert(cond, msg, ...) \
    if(!(cond))\
    {\
        Debug::assert_info(__FILE__, sizeof(__FILE__) - 1, __LINE__, msg, sizeof(msg));\
        Debug::breakpoint();\
    }
#else
#define DebugAssert(...)
#endif

