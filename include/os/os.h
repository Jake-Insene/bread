#pragma once
#include "core/header.h"
#include "collections/string.h"


struct OS
{
#if defined(BREAD_WIN32)
    using Handle = void*;
#else
    using Handle = MemoryAddress;
#endif

    enum MapAccess
    {
        Unknown = 0,
        ReadWrite,
        ReadWriteExecute,
    };

    using VoidFunction = void(*)();

    struct QueryMemory
    {
        MemoryAddress base_address;
        usize region_size;
    };

    static void initialize(Mem::Allocator& allocator);
    static void shutdown();

    static f64 get_time();

    static void exit(u64 code);
    static usize get_page_size();

    static Handle load_library(StringView lib_path);
    static void unload_library(Handle library);
    static VoidFunction get_proc_address(Handle library, StringView symbol_name);

    static Slice<u8> map_memory(usize memory_size, MapAccess access);
    static void unmap_memory(const Slice<u8>& memory);
    static QueryMemory query_memory(const Slice<u8>& memory);

    static bool set_current_directory(StringView dir);
};
