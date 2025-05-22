#pragma once
#include "core/log.h"

struct Debug
{
    
    template<typename... TArgs>
    static void info(const char* fmt, TArgs&&... args)
    {
#if SHOW_DEBUG_INFO
        if constexpr(sizeof...(args) == 0)
        {
            Log::info("%s", fmt);
        }
        else
        {
            Log::info(fmt, args...);
        }
#endif
    }

    template<typename... TArgs>
    static void warning(const char* fmt, TArgs&&... args)
    {
#if SHOW_DEBUG_INFO
        if constexpr (sizeof...(args) == 0)
        {
            Log::warning("%s", fmt);
        }
        else
        {
            Log::warning(fmt, args...);
        }
#endif
    }

    template<typename... TArgs>
    static void error(const char* fmt, TArgs&&... args)
    {
#if SHOW_DEBUG_INFO
        if constexpr (sizeof...(args) == 0)
        {
            Log::error("%s", fmt);
        }
        else
        {
            Log::error(fmt, args...);
        }
#endif
    }

    static void breakpoint()
    {
#if defined(ENGINE_WIN32)
        __debugbreak();
#elif defined(ENGINE_ANDROID)
        __builtin_trap();
#endif
    }
    
};
