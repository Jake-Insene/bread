#pragma once
#include "core/header.h"


namespace Format
{
template<typename... TArgs>
struct FormatString;
}


struct Log
{
    template<typename... TArgs>
    static void error(const Format::FormatString<TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args);

    template<typename... TArgs>
    static void warning(const Format::FormatString<TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args);

    template<typename... TArgs>
    static void info(const Format::FormatString<TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args);

    template<typename... TArgs>
    static void debug(const Format::FormatString<TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args)
    {
#if DEBUG
        info(fmt, Forward<TArgs>(args)...);
#else
        Unused(fmt, args...);
#endif
    }
};

#include "fmt/fmt.h"
#include "io/file.h"

template<typename... TArgs>
void Log::error(const Format::FormatString<TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args)
{
    File err = File::get_stderr();
    if (err.handle == 0)
    {
        return;
    }
    Format::format<true>(
        err.writer(), fmt, Forward<TArgs>(args)...
    );
    err.flush(); // Required on android
}

template<typename... TArgs>
void Log::warning(const Format::FormatString<TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args)
{
    File err = File::get_stderr();
    if (err.handle == 0)
    {
        return;
    }
    Format::format<true>(
        err.writer(), fmt, Forward<TArgs>(args)...
    );
    err.flush(); // Required on android
}

template<typename... TArgs>
void Log::info(const Format::FormatString<TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args)
{
    File out = File::get_stdout();
    if (out.handle == 0)
    {
        return;
    }
    Format::format<true>(
        out.writer(), fmt, Forward<TArgs>(args)...
    );
    out.flush(); // Required on android
}
