#pragma once
#include "collections/free_list.h"
#include "os/os.h"
#include "platform/platform_header.h"


struct AndroidOS
{
    struct InternalData
    {
        Mem::Allocator* allocator;
    };

    static inline InternalData data;

    [[nodiscard]] static Mem::Allocator* get_allocator() { return data.allocator; }

    static void initialize(Mem::Allocator* allocator);
    static void shutdown();

    static f64 get_time();

    static OS::Handle load_library(StringView lib_path);
    static void unload_library(OS::Handle library);
    static OS::VoidFunction get_proc_address(OS::Handle library, StringView symbol_name);

    static void exit(u64 code);
    static usize get_page_size();

    static Slice<u8> map_memory(usize memory_size, OS::MapAccess access);
    static void unmap_memory(const Slice<u8>& memory);
    static OS::QueryMemory query_memory(const Slice<u8>& memory);

    static bool set_current_directory(StringView dir);
};
