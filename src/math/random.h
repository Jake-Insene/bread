#pragma once
#include "core/templates.h"
#include "core/values.h"

namespace math
{

void random_seed(u64 seed);
void random_seed_default();

[[nodiscard]] u64 random_u64();

template<typename T>
[[nodiscard]] inline T random()
{
	if constexpr (IsAnyOf<T, i64, u64>)
	{
		return T(random_u64());
	}
	if constexpr (IsSigned<T>)
	{
		return random_u64() & ((u64(1) << (sizeof(T) * 8)) -1);
	}
	else if (IsUnsigned<T>)
	{
		return random_u64() & MaxValue<T>;
	}
	else if (IsSame<T, f64>)
	{
		return f64(random_u64());
	}
	else
	{
		return 0;
	}
}

template<typename T>
[[nodiscard]] inline T random_range(T min_value, T max_value)
{
	using UT = MakeUnsigned<T>;
	UT range = UT(UT(max_value) - UT(min_value)) + 1;

	// rejection threshold to avoid modulo bias
	u64 threshold = (~range) % range;

	u64 r;
	do
	{
		r = random_u64();
	} while (r < threshold);

	return T(min_value + T(r % range));
}

}