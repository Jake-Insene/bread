#include "mem/utils.h"

#include "platform/platform_header.h"


namespace mem
{
	usize get_page_size()
	{
		SYSTEM_INFO system_info;
		GetSystemInfo(&system_info);
		return system_info.dwPageSize;
	}
}
