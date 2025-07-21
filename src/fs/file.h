#pragma once
#include "core/header.h"

struct StringView;

template<typename T>
struct Slice;

namespace mem
{
struct Allocator;
}

namespace io
{
struct Writer;
}

namespace fmt
{
template<typename... TArgs>
struct FormatString;

template<typename... TArgs>
void format(const io::Writer& writer, FormatString<TypeIdentity<TArgs>...>, TArgs...);

}

struct File
{
    enum OpenMode
    {
        Read = Bit(0),
        Write = Bit(1),
        Create = Bit(2),
    };

    usize handle;

    static Slice<u8> read_all(mem::Allocator& allocator, StringView path);

    static File get_stderr();
    static File get_stdout();
    static File get_stdin();

    static File open(StringView path, OpenMode mode);
    static bool exists(StringView path);

    void destroy();

    void write(const Slice<const u8> bytes);
    void put(u8 value);
    void read(Slice<u8> bytes);

    void flush();

    template<usize N, typename... TArgs>
    void print(const char(&fmt_str)[N], TArgs... args);

    io::Writer writer();
};

#include "fmt/fmt.h"

template<usize N, typename... TArgs>
void File::print(const char(&fmt_str)[N], TArgs... args)
{
    fmt::format(writer(), fmt_str, args...);
}



