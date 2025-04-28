#include "mem/utils.h"

#include "platform/platform_header.h"


namespace mem
{
    
    usize get_page_size()
    {
        return (usize)sysconf(_SC_PAGESIZE);
    }
    
}
