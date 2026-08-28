#include "Platform/win32/win32_os.h"

#include "Debug/Fail.hpp"


alignas(alignof(Win32OS::InternalData)) static u8 place_holder_memory[sizeof(Win32OS::InternalData)]{};
static Win32OS::InternalData& get_data()
{
	return *reinterpret_cast<Win32OS::InternalData*>(place_holder_memory);
}

Mem::Allocator& Win32OS::get_allocator() { return get_data().allocator; }

void OS::initialize(Mem::Allocator& allocator)
{
    // Ensures constructors are call.
    Core::Mem::Placement(get_data(), allocator);
    
    // For get_time()
    LARGE_INTEGER platform_time;
    QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&get_data().frequency));
    QueryPerformanceCounter(&platform_time);

    get_data().program_start = f64(platform_time.QuadPart) / f64(get_data().frequency);

    SYSTEM_INFO info;
    GetSystemInfo(&info);
    get_data().page_size = static_cast<usize>(info.dwPageSize);
}

void OS::shutdown()
{
    Core::Mem::Destruct(get_data());
}

f64 OS::get_time()
{
    LARGE_INTEGER platform_time;
    QueryPerformanceCounter(&platform_time);
    return (f64(platform_time.QuadPart) / f64(get_data().frequency)) - get_data().program_start;
}

void OS::exit(u64 code)
{
    ExitProcess(static_cast<UINT>(code));
}

usize OS::get_page_size()
{
    return get_data().page_size;
}

OS::Handle OS::load_library(Collections::StringView lib_path)
{
    Slice path = Win32OS::get_allocator().array<char>(lib_path.len + 1);
    Mem::copy(path, lib_path);

    OS::Handle library = reinterpret_cast<OS::Handle>(LoadLibraryA(path.ptr()));
    Win32OS::get_allocator().free(Mem::to_bytes(path));

    return library;
}

void OS::unload_library(OS::Handle library)
{
    FreeLibrary(reinterpret_cast<HMODULE>(library));
}

OS::VoidFunction OS::get_proc_address(OS::Handle library, Collections::StringView symbol_name)
{
    Slice symbol = Win32OS::get_allocator().array<char>(symbol_name.len + 1);
    Mem::copy(symbol, symbol_name);
    
    OS::VoidFunction func = reinterpret_cast<OS::VoidFunction>(
        GetProcAddress(reinterpret_cast<HMODULE>(library), symbol.ptr())
    );
    
    Win32OS::get_allocator().free(Mem::to_bytes(symbol));
    return func;
}

Slice<u8> OS::map_memory(usize memory_size, OS::MapAccess access)
{
    const usize aligned_size = Mem::align_up(memory_size, get_page_size());
    Slice<u8> ptr{};

    switch (access)
    {
    case OS::Unknown:
        break;
    case OS::ReadWrite:
    {
        ptr.items = reinterpret_cast<u8*>(
            VirtualAllocEx(GetCurrentProcess(),
                nullptr, aligned_size,
                MEM_RESERVE | MEM_COMMIT,
                PAGE_READWRITE
            )
        );
        ptr.len = aligned_size;
    }
        break;
    case OS::ReadWriteExecute:
    {
        ptr.items = reinterpret_cast<u8*>(
            VirtualAllocEx(GetCurrentProcess(),
                nullptr, aligned_size,
                MEM_RESERVE | MEM_COMMIT,
                PAGE_EXECUTE_READWRITE
            )
        );
        ptr.len = aligned_size;
    }
        break;
    }

    return ptr;
}

void OS::unmap_memory(const Slice<u8>& memory)
{
    VirtualFreeEx(GetCurrentProcess(), memory.items, 0, MEM_RELEASE);
}

OS::QueryMemory OS::query_memory(const Slice<u8>& memory)
{
    MEMORY_BASIC_INFORMATION mem_info;
    VirtualQueryEx(GetCurrentProcess(), memory.items, &mem_info, sizeof(mem_info));
    
    return OS::QueryMemory
    {
        .base_address = MemoryAddress(mem_info.BaseAddress),
        .region_size = usize(mem_info.RegionSize),
    };    
}

bool OS::set_current_directory(Collections::StringView dir)
{
    Slice path = Win32OS::get_allocator().array<char>(dir.len + 1);
    Mem::copy(path, dir);
    bool result = SetCurrentDirectoryA(path.ptr()) == TRUE;
    Win32OS::get_allocator().free(Mem::to_bytes(path));

    return result;
}
