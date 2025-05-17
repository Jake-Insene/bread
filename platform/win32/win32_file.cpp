#include "platform/win32/win32_file.h"

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
