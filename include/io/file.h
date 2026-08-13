#pragma once
#include "Collections/StringView.hpp"
#include "os/os.h"


template<typename T>
struct Slice;

namespace Mem
{
struct Allocator;
}

namespace IO
{

struct Writer;

struct File
{
    DisableCopy(File);
    DisableMove(File);

    enum OpenMode
    {
        Read = Bit(0),
        Write = Bit(1),
        Create = Bit(2),
    };

    OS::Handle handle;

    static File& get_stderr();
    static File& get_stdout();
    static File& get_stdin();

    static Slice<u8> read_all(Mem::Allocator& allocator, StringView path);
    static bool exists(Mem::Allocator& allocator, StringView path);

    File(Mem::Allocator& allocator, StringView path, OpenMode mode);
    ~File();

    void write(const Slice<const u8>& bytes);
    void put(u8 value);
    void read(Slice<u8> bytes);

    void flush();

    Writer writer();
};

}

