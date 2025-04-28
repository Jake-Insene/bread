#include "platform/win32/win32_log.h"

#include "platform/platform_header.h"

#include <cstdio>

char fmt_buf[4096] = {};

void Log::error(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
	int l = std::vsprintf(fmt_buf, fmt, args);
	OutputDebugStringA(fmt_buf);
	OutputDebugStringA("\n");
    va_end(args);
}

void Log::warning(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int l = std::vsprintf(fmt_buf, fmt, args);
    OutputDebugStringA("bread[log]");
    OutputDebugStringA(fmt_buf);
    OutputDebugStringA("\n");
    va_end(args);
}

void Log::info(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int l = std::vsprintf(fmt_buf, fmt, args);
    OutputDebugStringA(fmt_buf);
    OutputDebugStringA("\n");
    va_end(args);
}
