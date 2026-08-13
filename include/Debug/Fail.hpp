#pragma once
#include "Debug/Log.hpp"


#define Fatal(...) Log::error(__VA_ARGS__); Debug::breakpoint();

#define FailOn(cond, ...) \
    if((cond))\
    {\
        Fatal(__VA_ARGS__);\
    }


