#pragma once
#include "core/templates.h"

namespace fmt
{
template<typename... TArgs>
struct FormatString;
}


struct Log
{
    template<typename... TArgs>
    static void error(const fmt::FormatString<TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args);

    template<typename... TArgs>
    static void warning(const fmt::FormatString<TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args);

    template<typename... TArgs>
    static void info(const fmt::FormatString<TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args);

    template<typename... TArgs>
    static void debug(const fmt::FormatString<TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args)
    {
#if DEBUG
        info(fmt, Forward<TArgs>(args)...);
#else
        (void)fmt;
        ((void)args, ...);
#endif
    }
};

#include "fmt/fmt.h"
#include "io/file.h"
#include "io/writer.h"

template<typename... TArgs>
void Log::error(const fmt::FormatString<TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args)
{
    auto err = File::get_stderr();
    if (err.handle == 0) return;
    fmt::format<true>(
        err.writer(), fmt, Forward<TArgs>(args)...
    );
    err.flush(); // Required on android
}

template<typename... TArgs>
void Log::warning(const fmt::FormatString<TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args)
{
    auto err = File::get_stderr();
    if (err.handle == 0) return;
    fmt::format<true>(
        err.writer(), fmt, Forward<TArgs>(args)...
    );
    err.flush(); // Required on android
}

template<typename... TArgs>
void Log::info(const fmt::FormatString<TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args)
{
    auto out = File::get_stdout();
    if (out.handle == 0) return;
    fmt::format<true>(
        out.writer(), fmt, Forward<TArgs>(args)...
    );
    out.flush(); // Required on android
}
