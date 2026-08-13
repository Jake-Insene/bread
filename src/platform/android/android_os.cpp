#include "platform/android/android_os.h"

#include "Core/Header.hpp"
#include "Debug/Fail.hpp"
#include "platform/platform_header.h"


void AndroidOS::initialize(Mem::Allocator& allocator)
{
    // Ensures constructors are call.
    ConstructObject(AndroidOS::data);

    data.allocator = allocator;
}

void AndroidOS::shutdown()
{
}

f64 AndroidOS::get_time()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

void AndroidOS::exit(u64 code)
{
    Unused(code);
}

usize AndroidOS::get_page_size()
{
    return (usize)sysconf(_SC_PAGESIZE);
}

OS::Handle AndroidOS::load_library(StringView lib_path)
{
    Slice path = get_allocator().array<char>(lib_path.len + 1);
    Mem::copy(path, lib_path);
    
    OS::Handle library = reinterpret_cast<OS::Handle>(dlopen(path.ptr(), RTLD_NOW | RTLD_NOW));
    get_allocator().free(Mem::to_bytes(path));

    return library;
}

void AndroidOS::unload_library(OS::Handle library)
{
    dlclose(reinterpret_cast<void*>(library));
}

OS::VoidFunction AndroidOS::get_proc_address(OS::Handle library, StringView symbol_name)
{
    Slice symbol = get_allocator().array<char>(symbol_name.len + 1);
    Mem::copy(symbol, symbol_name);

    OS::VoidFunction func = reinterpret_cast<OS::VoidFunction>(
        dlsym(reinterpret_cast<void*>(library), symbol.ptr())
    );

    get_allocator().free(Mem::to_bytes(symbol));
    return func;
}


Slice<u8> AndroidOS::map_memory(usize memory_size, OS::MapAccess access)
{
    const usize aligned_size = Mem::align_up(memory_size, get_page_size());
    Slice<u8> ptr{};

    switch (access)
    {
    case OS::MapUnknown:
        break;
    case OS::MapReadWrite:
    {
        ptr.items = reinterpret_cast<u8*>(mmap(
            0, aligned_size,
            PROT_READ | PROT_WRITE,
            MAP_ANONYMOUS | MAP_PRIVATE,
            -1, 0
        ));
        ptr.len = aligned_size;
    }
    break;
    default:
        FailOn(true, "implement this!");
        break;
    }

    return ptr;
}

void AndroidOS::unmap_memory(const Slice<u8>& memory)
{
    munmap(memory.items, memory.len);
}

OS::QueryMemory AndroidOS::query_memory(const Slice<u8>& memory)
{
    Unused(memory);
    return OS::QueryMemory();
}

bool AndroidOS::set_current_directory(StringView dir)
{
    Unused(dir);
    return true;
}

