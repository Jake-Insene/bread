#include "platform/instrinsics.h"
#include <emmintrin.h>
#include <xmmintrin.h>


void PlatformIntricics::copy(Slice<u8> dest, Slice<const u8> src)
{
	usize i = 0;
	for (i = 0; i < src.len; i += 16)
	{
		if ((src.len - i) < 16)
		{
			break;
		}

		__m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&src[i]));
		_mm_storeu_si128(reinterpret_cast<__m128i*>(&dest[i]), v);
	}

	usize remain = src.len & 0xF;
	for (usize j = 0; j < remain; j++)
	{
		dest[i + j] = src[i + j];
	}
}

void PlatformIntricics::setzero(Slice<u8> dest)
{
	static constexpr __m128 zeroes = {};
	usize i = 0;
	for (i = 0; i < dest.len; i += 4)
	{
		if ((dest.len - i) < 16)
		{
			break;
		}

		_mm_storeu_ps(reinterpret_cast<float*>(&dest[i]), zeroes);
	}

	usize remain = dest.len & 0xF;
	for (usize j = 0; j < remain; j++)
	{
		dest[i + j] = 0;
	}
}

