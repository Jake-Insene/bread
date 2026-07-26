#pragma once
#include "collections/slice.h"

#if BREAD_X64
#include <smmintrin.h>
#include <emmintrin.h>
#elif BREAD_ARM64
#include <arm_neon.h>
#endif


struct PlatformIntricics
{

	template<typename T>
	requires(IsFloatingPoint<T>)
	[[nodiscard]] static T sqrt(T x)
	{
		if constexpr (IsSame<T, f32>)
		{
#if BREAD_X64
			__m128 mm1 = _mm_set_ss(x);
			return _mm_cvtss_f32(_mm_sqrt_ss(mm1));
#elif BREAD_ARM64
            const float32x2_t v = vdup_n_f32(x);
            const float32x2_t result = vsqrt_f32(v);
            return vget_lane_f32(result, 0);
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
            return (x1 * x2 + y1 * y2);
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
#elif BREAD_ARM64
            float32x2_t v = vdup_n_f32(0.F);
            v = vset_lane_f32(x1, v, 0);
            v = vset_lane_f32(y1, v, 1);
            const float32x2_t squared = vmul_f32(v, v);
            const float sum = vget_lane_f32(vpadd_f32(squared, squared), 0);
            const float len = sqrt(sum);
            const float32x2_t inv_len = vdup_n_f32(1.0f / len);
            const float32x2_t result = vmul_f32(v, inv_len);
            x1 = vget_lane_f32(result, 0);
            y1 = vget_lane_f32(result, 1);
#endif
		}
	}

	static void copy(Slice<u8> dest, Slice<const u8> src);

	static void set(Slice<u8> dest, u8 value);
};