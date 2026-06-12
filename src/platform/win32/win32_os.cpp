#include "platform/win32/win32_os.h"

#include "core/types.h"
#include "debug/fail.h"



void Win32OS::initialize(Mem::Allocator* allocator)
{
    // Ensures constructors are call.
    ConstructObject(data);
    
    data.allocator = allocator;
    
    // For get_time()
    LARGE_INTEGER platform_time;
    QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&data.frequency));
    QueryPerformanceCounter(&platform_time);

    data.program_start = f64(platform_time.QuadPart) / f64(data.frequency);

    SYSTEM_INFO info;
    GetSystemInfo(&info);
    data.page_size = static_cast<usize>(info.dwPageSize);
}

void Win32OS::shutdown()
{
}

f64 Win32OS::get_time()
{
    LARGE_INTEGER platform_time;
    QueryPerformanceCounter(&platform_time);
    return (f64(platform_time.QuadPart) / f64(data.frequency)) - data.program_start;
}

void Win32OS::exit(u64 code)
{
    ExitProcess(static_cast<UINT>(code));
}

usize Win32OS::get_page_size()
{
    return data.page_size;
}

OS::Handle Win32OS::load_library(StringView lib_path)
{
    Slice path = get_allocator()->array<char>(lib_path.len + 1);
    Mem::copy(path, lib_path);

    OS::Handle library = reinterpret_cast<OS::Handle>(LoadLibraryA(path.ptr()));
    get_allocator()->free(Mem::to_bytes(path));

    return library;
}

void Win32OS::unload_library(OS::Handle library)
{
    FreeLibrary(reinterpret_cast<HMODULE>(library));
}

OS::VoidFunction Win32OS::get_proc_address(OS::Handle library, StringView symbol_name)
{
    Slice symbol = get_allocator()->array<char>(symbol_name.len + 1);
    Mem::copy(symbol, symbol_name);
    
    OS::VoidFunction func = reinterpret_cast<OS::VoidFunction>(
        GetProcAddress(reinterpret_cast<HMODULE>(library), symbol.ptr())
    );
    
    get_allocator()->free(Mem::to_bytes(symbol));
    return func;
}

Slice<u8> Win32OS::map_memory(usize memory_size, OS::MapAccess access)
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

void Win32OS::unmap_memory(const Slice<u8>& memory)
{
    VirtualFreeEx(GetCurrentProcess(), memory.items, 0, MEM_RELEASE);
}

OS::QueryMemory Win32OS::query_memory(const Slice<u8>& memory)
{
    MEMORY_BASIC_INFORMATION mem_info;
    VirtualQueryEx(GetCurrentProcess(), memory.items, &mem_info, sizeof(mem_info));
    
    return OS::QueryMemory
    {
        .base_address = MemoryAddress(mem_info.BaseAddress),
        .region_size = usize(mem_info.RegionSize),
    };    
}

bool Win32OS::set_current_directory(StringView dir)
{
    Slice path = get_allocator()->array<char>(dir.len + 1);
    Mem::copy(path, dir);
    bool result = SetCurrentDirectoryA(path.ptr()) == TRUE;
    get_allocator()->free(Mem::to_bytes(path));

    return result;
}
