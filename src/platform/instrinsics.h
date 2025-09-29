#pragma once
#include "collections/slice.h"

#include <intrin.h>
#include <xmmintrin.h>


struct PlatformIntricics
{

	template<typename T>
		requires(IsFloatingPoint<T>)
	[[nodiscard]] static T sqrt(T x)
	{
		if constexpr (IsSame<T, f32>)
		{
#if BREAD_X64
			__m128 mm = _mm_set_ss(x);
			return _mm_cvtss_f32(_mm_sqrt_ss(mm));
#else
#endif
		}
	}

	template<typename T>
		requires(IsFloatingPoint<T>)
	[[nodiscard]] static T vecdot(T x1, T y1, T x2, T y2)
	{
		if constexpr (IsSame<T, f32>)
		{
#if BREAD_X64
			const __m128 v1 = _mm_set_ps(0, 0, y1, x1);
			const __m128 v2 = _mm_set_ps(0, 0, y2, x2);
			const __m128 result = _mm_dp_ps(v1, v2, 0x3F);
			return _mm_cvtss_f32(result);
#else
#endif
		}
	}

	template<typename T>
		requires(IsFloatingPoint<T>)
	static void vecnormalize(T& x1, T& y1)
	{
		if constexpr (IsSame<T, f32>)
		{
#if BREAD_X64
			const __m128 v = _mm_set_ps(0, 0, y1, x1);
			const __m128 dot = _mm_set_ps1(vecdot(x1, y1, x1, y1));
			const __m128 isqr = _mm_rsqrt_ps(dot);
			const __m128 normalized = _mm_mul_ps(v, isqr);
			float fields[4] = {};
			_mm_store_ps(fields, normalized);
			x1 = fields[0];
			y1 = fields[1];
#else
#endif
		}
	}

	static void copy(Slice<u8> dest, Slice<const u8> src);

	static void setzero(Slice<u8> dest);
};