#include "mem/utils.h"

#include "platform/instrinsics.h"



namespace mem
{

void _copy(Slice<u8> dest, Slice<const u8> src)
{
	PlatformIntricics::copy(dest, src);
}

void _set_zero(Slice<u8> dest)
{
	PlatformIntricics::setzero(dest);
}

}