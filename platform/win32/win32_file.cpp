#include "platform/win32/win32_file.h"

#include "mem/allocator.h"
#include "platform/platform_header.h"

Slice<u8> File::read_all(mem::Allocator& allocator, StringView path)
{
    char tmp[256] = {};
    mem::copy(Slice(tmp), path);
	
	HANDLE file = CreateFileA(tmp, GENERIC_READ, FILE_SHARE_READ, 
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
    );

	if (file == INVALID_HANDLE_VALUE)
	{
		return {};
	}

	usize length = GetFileSize(file, nullptr);
    Slice<u8> bytes = allocator.alloc(length, sizeof(usize));
	(void)ReadFile(file, bytes.ptr(), length, nullptr, nullptr);

	CloseHandle(file);

    return bytes;
}

File File::get_stderr()
{
	HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
	return File
	{

		.handle = (usize)handle,
	};
}

File File::get_stdout()
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	return File
	{

		.handle = (usize)handle,
	};
}

File File::get_stdin()
{
	HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
	return File
	{

		.handle = (usize)handle,
	};
}

File File::open(StringView path, OpenMode mode)
{
	char tmp[256] = {};
	mem::copy(Slice(tmp), path);

	UINT access = 0;

	if (mode & File::Read)
	{
		access |= GENERIC_READ;
	}
	if (mode & File::Write)
	{
		access |= GENERIC_WRITE;
	}

	UINT open_or_create = 0;
	if (mode & File::Create)
	{
		open_or_create |= CREATE_ALWAYS;
	}
	else
	{
		open_or_create |= OPEN_EXISTING;
	}

	HANDLE file = CreateFileA(tmp, access, FILE_SHARE_READ,
		nullptr, open_or_create, FILE_ATTRIBUTE_NORMAL, nullptr
	);

	return File
	{
		.handle = (usize)file,
	};
}


void File::destroy()
{
	if (handle == 0) 
		return;

	CloseHandle((HANDLE)handle);
}

void File::write(const Slice<const u8> bytes)
{
	DebugAssert(handle != 0, "invalid file handler");
	WriteFile((HANDLE)handle, bytes.ptr(), (DWORD)bytes.len, 0, 0);
}

void File::put(u8 value)
{
	WriteFile((HANDLE)handle, &value, 1, 0, 0);
}

void File::read(Slice<u8> bytes)
{
	DebugAssert(handle != 0, "invalid file handler");
	ReadFile((HANDLE)handle, bytes.ptr(), (DWORD)bytes.len, 0, 0);
}

