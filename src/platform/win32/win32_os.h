#pragma once
#include "collections/free_list.h"
#include "os/os.h"
#include "platform/platform_header.h"



struct Win32OS
{
    static constexpr usize InitialThreadCount = 16;
    static constexpr usize InitialMutexCount = 1024;
    static constexpr usize InitialSemaphoreCount = 1024;

    static constexpr usize MaxThreadNameLen = 128;

    struct InternalData
    {
        mem::Allocator allocator;

        i64 frequency;
        f64 program_start;
        usize page_size;
    };

    static inline Win32OS::InternalData data;

    [[nodiscard]] static mem::Allocator& get_allocator() { return data.allocator; }

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static f64 get_time();

    static void exit(u64 code);
    static usize get_page_size();

    static OS::Handle load_library(StringView lib_path);
    static void unload_library(OS::Handle library);
    static OS::VoidFunction get_proc_address(OS::Handle library, StringView symbol_name);

    static Slice<u8> map_memory(usize memory_size, OS::MapAccess access);
    static void unmap_memory(const Slice<u8>& memory);
    static OS::QueryMemory query_memory(const Slice<u8>& memory);

    static bool set_current_directory(StringView dir);
};
