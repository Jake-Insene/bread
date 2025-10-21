#pragma once
#include "log/log.h"


#define Fatal(...) Log::error(__VA_ARGS__); Debug::breakpoint();

#define FailOn(cond, ...) \
    if(cond)\
    {\
        Fatal(__VA_ARGS__);\
    }


