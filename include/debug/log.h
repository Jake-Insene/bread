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
    static void error(const Format::FormatString<Core::TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args);

    template<typename... TArgs>
    static void warning(const Format::FormatString<Core::TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args);

    template<typename... TArgs>
    static void info(const Format::FormatString<Core::TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args);

    template<typename... TArgs>
    static void debug([[maybe_unused]] const Format::FormatString<Core::TypeIdentity<TArgs>&&...>& fmt, [[maybe_unused]] TArgs&&... args)
    {
#if DEBUG
        info(fmt, Core::Forward<TArgs>(args)...);
#endif
    }
};

#include "fmt/fmt.h"
#include "io/file.h"

template<typename... TArgs>
void Log::error(const Format::FormatString<Core::TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args)
{
    IO::File& err = IO::File::get_stderr();
    if (err.handle == 0)
    {
        return;
    }
    Format::format<true>(
        err.writer(), fmt, Core::Forward<TArgs>(args)...
    );
    err.flush(); // Required on android
}

template<typename... TArgs>
void Log::warning(const Format::FormatString<Core::TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args)
{
    IO::File& err = IO::File::get_stderr();
    if (err.handle == 0)
    {
        return;
    }
    Format::format<true>(
        err.writer(), fmt, Core::Forward<TArgs>(args)...
    );
    err.flush(); // Required on android
}

template<typename... TArgs>
void Log::info(const Format::FormatString<Core::TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args)
{
    IO::File& out = IO::File::get_stdout();
    if (out.handle == 0)
    {
        return;
    }
    Format::format<true>(
        out.writer(), fmt, Core::Forward<TArgs>(args)...
    );
    out.flush(); // Required on android
}
