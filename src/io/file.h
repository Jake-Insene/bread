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

struct File
{
    enum OpenMode
    {
        Read = Bit(0),
        Write = Bit(1),
        Create = Bit(2),
    };

    usize handle;

    static Slice<u8> read_all(const mem::Allocator& allocator, StringView path);

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

    io::Writer writer();
};


