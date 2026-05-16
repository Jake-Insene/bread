#include "platform/win32/win32_file.h"

#include "collections/string_view.h"
#include "mem/allocator.h"
#include "platform/platform_header.h"


Slice<u8> File::read_all(const mem::Allocator& allocator, StringView path)
{
	Slice<char> tmp = allocator.array<char>(path.len + 1);
	mem::copy(tmp, path);

	HANDLE file = CreateFileA(tmp.ptr(), GENERIC_READ, FILE_SHARE_READ, 
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
    );

	allocator.free(mem::to_bytes(tmp));

	if (file == INVALID_HANDLE_VALUE)
	{
		return {};
	}

	usize length = GetFileSize(file, nullptr);
    Slice<u8> bytes = allocator.alloc(length, sizeof(usize));
	(void)ReadFile(file, bytes.ptr(), DWORD(length), nullptr, nullptr);

	CloseHandle(file);

    return bytes;
}

File File::get_stderr()
{
	HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
	return File
	{
		.handle = reinterpret_cast<usize>(handle),
	};
}

File File::get_stdout()
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	return File
	{
		.handle = reinterpret_cast<usize>(handle),
	};
}

File File::get_stdin()
{
	HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
	return File
	{
		.handle = reinterpret_cast<usize>(handle),
	};
}

File File::open(const mem::Allocator& allocator, StringView path, OpenMode mode)
{
	Slice<char> tmp = allocator.array<char>(path.len + 1);
	mem::copy(tmp, path);
	UINT access = 0;

	if (HasValue(mode & File::Read))
	{
		access |= GENERIC_READ;
	}
	if (HasValue(mode & File::Write))
	{
		access |= GENERIC_WRITE;
	}

	UINT open_or_create = 0;
	if (HasValue(mode & File::Create))
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

	allocator.free(mem::to_bytes(tmp));

	return File
	{
		.handle = reinterpret_cast<usize>(file),
	};
}

bool File::exists(const mem::Allocator& allocator, StringView path)
{
	Slice<char> tmp = allocator.array<char>(path.len + 1);
	mem::copy(tmp, path);

	HANDLE file = CreateFileA(tmp.ptr(), 0, 0, nullptr, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
	);

	allocator.free(mem::to_bytes(tmp));

	bool finded = file != INVALID_HANDLE_VALUE;
	CloseHandle(file);
	return finded;
}

void File::destroy()
{
	if (handle == 0) 
	{
		return;
	}

	CloseHandle(reinterpret_cast<HANDLE>(handle));
}

void File::write(const Slice<const u8> bytes)
{
	DebugAssert(handle != 0, "invalid file handler");
	(void)WriteFile(reinterpret_cast<HANDLE>(handle), bytes.ptr(), static_cast<DWORD>(bytes.len), 0, 0);
}

void File::put(u8 value)
{
	WriteFile(reinterpret_cast<HANDLE>(handle), &value, 1, 0, 0);
}

void File::read(Slice<u8> bytes)
{
	DebugAssert(handle != 0, "invalid file handler");
	DWORD bytes_readed = 0;
	(void)ReadFile(reinterpret_cast<HANDLE>(handle), bytes.ptr(), static_cast<DWORD>(bytes.len), &bytes_readed, 0);
	DebugAssert(bytes_readed <= bytes.len, "read overflows");
}

void File::flush()
{
	DebugAssert(handle != 0, "invalid file handler");
	(void)FlushFileBuffers(reinterpret_cast<HANDLE>(handle));
}
