#include "platform/android/android_log.h"

#include "platform/platform_header.h"

#include <cstdlib>

void Log::error(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    __android_log_vprint(ANDROID_LOG_ERROR, "cardofwar[log]", fmt, args);
    va_end(args);
}
    
void Log::warning(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    __android_log_vprint(ANDROID_LOG_WARN, "cardofwar[log]", fmt, args);
    va_end(args);
}
    
void Log::info(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    __android_log_vprint(ANDROID_LOG_INFO, "cardofwar[log]", fmt, args);
    va_end(args);
}
