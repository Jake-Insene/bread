#include "platform/instrinsics.h"


void PlatformIntricics::copy(Slice<u8> dest, Slice<const u8> src)
{
	usize i = 0;
	for (i = 0; i < src.len; i += 16)
	{
		if ((src.len - i) < 16)
		{
			break;
		}
#if BREAD_X64
		const __m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&src[i]));
		_mm_storeu_si128(reinterpret_cast<__m128i*>(&dest[i]), v);
#elif BREAD_ARM64
        const uint8x16_t v = vld1q_u8(reinterpret_cast<const u8*>(&src[i]));
        vst1q_u8(reinterpret_cast<u8*>(&dest[i]), v);
#endif
	}

    usize remain = src.len & 0xF;
    for (usize j = 0; j < remain; j++)
    {
        dest[i + j] = src[i + j];
    }
}

void PlatformIntricics::set(Slice<u8> dest, u8 value)
{
	usize i = 0;
#if BREAD_X64
	const __m128i values = _mm_set1_epi8(static_cast<i8>(value));
#endif

	for (i = 0; i < dest.len; i += 16)
	{
		if ((dest.len - i) < 16)
		{
			break;
		}
#if BREAD_X64
		_mm_storeu_ps(reinterpret_cast<float*>(&dest[i]), values);
#elif BREAD_ARM64
		// TODO: set for android/arm
        // const uint8x16_t zeroes = vdupq_n_u8(0);
        // vst1q_u8(reinterpret_cast<u8*>(&dest[i]), zeroes);
#endif
	}

	usize remain = dest.len & 0xF;
	for (usize j = 0; j < remain; j++)
	{
		dest[i + j] = value;
	}
}

