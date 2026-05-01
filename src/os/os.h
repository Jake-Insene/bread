#pragma once
#include "core/header.h"
#include "collections/string.h"


struct OS
{
    enum MapAccess
    {
        MapUnknown = 0,
        MapReadWrite,
        MapReadWriteExecute,
    };

    using VoidFunction = void(*)();

    struct QueryMemory
    {
        MemoryAddress base_address;
        usize region_size;
    };

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static f64 get_time();

    static void exit(u64 code);
    static usize get_page_size();

    static MemoryAddress load_library(StringView lib_path);
    static void unload_library(MemoryAddress library);
    static VoidFunction get_proc_address(MemoryAddress library, StringView symbol_name);

    static Slice<u8> map_memory(usize memory_size, MapAccess access);
    static void unmap_memory(const Slice<u8>& memory);
    static QueryMemory query_memory(const Slice<u8>& memory);

    static bool set_current_directory(StringView dir);
};
