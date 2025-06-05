#pragma once

#define Fatal(...) Log::error(__VA_ARGS__); Debug::breakpoint();

#define FailOn(cond, ...) \
    if(cond)\
    {\
        Fatal(__VA_ARGS__);\
    }


#include "log/log.h"
