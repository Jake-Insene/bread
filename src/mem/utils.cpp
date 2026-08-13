#include "Mem/Utils.hpp"

#include "platform/instrinsics.h"



namespace Mem
{

void _copy(Slice<u8> dest, Slice<const u8> src)
{
	PlatformIntricics::copy(dest, src);
}

void _set(Slice<u8> dest, u8 value)
{
	PlatformIntricics::set(dest, value);
}

}