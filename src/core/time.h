#pragma once
#include "core/types.h"

struct Time
{
    static void initialize();
    static void shutdown();

    static f64 get_time();
};
