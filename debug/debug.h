#pragma once
#include "core/log.h"

struct Debug
{
    
    template<typename... TArgs>
    static void info(const char* fmt, TArgs&&... args)
    {
        if constexpr(sizeof...(args) == 0)
        {
            Log::info("%s", fmt);
        }
        else
        {
            Log::info(fmt, args...);
        }
    }
    
};
