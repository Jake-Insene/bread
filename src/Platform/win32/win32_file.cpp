#include "Platform/win32/win32_file.h"

#include "Collections/StringView.hpp"
#include "Mem/Allocator.hpp"
#include "Platform/platform_header.h"


namespace IO
{

static_assert(sizeof(File) == sizeof(HANDLE));

File& File::get_stderr()
{
	static HANDLE handle = 0;
	if(handle == 0)
	{
		handle = GetStdHandle(STD_ERROR_HANDLE);
	}
	return *reinterpret_cast<File*>(&handle);
}

File& File::get_stdout()
{
	static HANDLE handle = 0;
	if(handle == 0)
	{
		handle = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	return *reinterpret_cast<File*>(&handle);
}

File& File::get_stdin()
{
	static HANDLE handle = 0;
	if(handle == 0)
	{
		handle = GetStdHandle(STD_INPUT_HANDLE);
	}
	return *reinterpret_cast<File*>(&handle);
}

Slice<u8> File::read_all(Mem::Allocator& allocator, Collections::StringView path)
{
	Slice tmp = allocator.array<char>(path.len + 1);
	Mem::copy(tmp, path);

	HANDLE file = CreateFileA(tmp.ptr(), GENERIC_READ, FILE_SHARE_READ, 
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
    );

	allocator.free(Mem::to_bytes(tmp));

	if (file == INVALID_HANDLE_VALUE)
	{
		return {};
	}

	usize length = GetFileSize(file, nullptr);
    Slice bytes = allocator.alloc(length, sizeof(usize));
	(void)ReadFile(file, bytes.ptr(), DWORD(length), nullptr, nullptr);

	CloseHandle(file);

    return bytes;
}

bool File::exists(Mem::Allocator& allocator, Collections::StringView path)
{
	Slice tmp = allocator.array<char>(path.len + 1);
	Mem::copy(tmp, path);

	HANDLE file = CreateFileA(tmp.ptr(), 0, 0, nullptr, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
	);

	allocator.free(Mem::to_bytes(tmp));

	bool finded = file != INVALID_HANDLE_VALUE;
	CloseHandle(file);
	return finded;
}

File::File(Mem::Allocator& allocator, Collections::StringView path, OpenMode mode)
{
	Slice tmp = allocator.array<char>(path.len + 1);
	Mem::copy(tmp, path);
	UINT access = 0;

	if(Core::HasValue(mode & File::Read))
	{
		access |= GENERIC_READ;
	}
	if(Core::HasValue(mode & File::Write))
	{
		access |= GENERIC_WRITE;
	}

	UINT open_or_create = 0;
	if(Core::HasValue(mode & File::Create))
	{
		open_or_create |= CREATE_ALWAYS;
	}
	else
	{
		open_or_create |= OPEN_EXISTING;
	}

	HANDLE file = CreateFileA(tmp.ptr(), access, FILE_SHARE_READ,
		nullptr, open_or_create, FILE_ATTRIBUTE_NORMAL, nullptr
	);

	allocator.free(Mem::to_bytes(tmp));

	handle = reinterpret_cast<OS::Handle>(file);
}

File::~File()
{
	if(handle == 0
		|| handle == GetStdHandle(STD_ERROR_HANDLE)
		|| handle == GetStdHandle(STD_OUTPUT_HANDLE)
		|| handle == GetStdHandle(STD_INPUT_HANDLE))
	{
		return;
	}

	CloseHandle(reinterpret_cast<HANDLE>(handle));
}

void File::write(const Slice<const u8>& bytes)
{
	DebugAssert(handle != 0
		|| handle == GetStdHandle(STD_ERROR_HANDLE)
		|| handle == GetStdHandle(STD_OUTPUT_HANDLE)
		|| handle == GetStdHandle(STD_INPUT_HANDLE), "invalid file handler");
	(void)WriteFile(reinterpret_cast<HANDLE>(handle), bytes.ptr(), static_cast<DWORD>(bytes.len), 0, 0);
}

void File::put(u8 value)
{
	DebugAssert(handle != 0
		|| handle == GetStdHandle(STD_ERROR_HANDLE)
		|| handle == GetStdHandle(STD_OUTPUT_HANDLE)
		|| handle == GetStdHandle(STD_INPUT_HANDLE), "invalid file handler");
	WriteFile(reinterpret_cast<HANDLE>(handle), &value, 1, 0, 0);
}

void File::read(Slice<u8> bytes)
{
	DebugAssert(handle != 0
		|| handle == GetStdHandle(STD_ERROR_HANDLE)
		|| handle == GetStdHandle(STD_OUTPUT_HANDLE)
		|| handle == GetStdHandle(STD_INPUT_HANDLE), "invalid file handler");
	DWORD bytes_readed = 0;
	(void)ReadFile(reinterpret_cast<HANDLE>(handle), bytes.ptr(), static_cast<DWORD>(bytes.len), &bytes_readed, 0);
	DebugAssert(bytes_readed <= bytes.len, "read overflows");
}

void File::flush()
{
	DebugAssert(handle != 0
		|| handle == GetStdHandle(STD_ERROR_HANDLE)
		|| handle == GetStdHandle(STD_OUTPUT_HANDLE)
		|| handle == GetStdHandle(STD_INPUT_HANDLE), "invalid file handler");
	(void)FlushFileBuffers(reinterpret_cast<HANDLE>(handle));
}

}
