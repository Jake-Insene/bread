#pragma once
#include "core/log.h"

#define LOG_TAG "cardofwar[log]"
#define _LOG(priority, fmt, ...) \
  ((void)__android_log_print((priority), (LOG_TAG), (fmt)__VA_OPT__(, ) __VA_ARGS__))

struct AndroidLog : Log
{
    
};