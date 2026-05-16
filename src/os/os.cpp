#include "os/os.h"

#include "collections/string_view.h"


#if BREAD_WIN32
#include "platform/win32/win32_os.h"
using PlatformOS = Win32OS;
#elif BREAD_ANDROID
#include "platform/android/android_os.h"
using PlatformOS = AndroidOS;
#endif


void OS::initialize(const mem::Allocator& allocator)
{
	PlatformOS::initialize(allocator);
}

void OS::shutdown()
{
	PlatformOS::shutdown();
}

f64 OS::get_time()
{
	return PlatformOS::get_time();
}

void OS::exit(u64 code)
{
	PlatformOS::exit(code);
}

usize OS::get_page_size()
{
	return PlatformOS::get_page_size();
}

OS::Handle OS::load_library(StringView lib_path)
{
	return PlatformOS::load_library(lib_path);
}

void OS::unload_library(Handle library)
{
	PlatformOS::unload_library(library);
}

OS::VoidFunction OS::get_proc_address(Handle library, StringView symbol_name)
{
	return PlatformOS::get_proc_address(library, symbol_name);
}

Slice<u8> OS::map_memory(usize memory_size, MapAccess access)
{
	return PlatformOS::map_memory(memory_size, access);
}

void OS::unmap_memory(const Slice<u8>& memory)
{
	PlatformOS::unmap_memory(memory);
}

OS::QueryMemory OS::query_memory(const Slice<u8>& memory)
{
	return PlatformOS::query_memory(memory);
}

bool OS::set_current_directory(StringView dir)
{
	return PlatformOS::set_current_directory(dir);
}

