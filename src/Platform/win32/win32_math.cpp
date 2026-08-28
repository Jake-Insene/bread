#include "Platform/platform_header.h"

#include "Math/random.h"


namespace Math
{

static inline u64 __seed = 0;

static inline u64 __xorshift64()
{
	__seed ^= __seed << 13;
	__seed ^= __seed >> 7;
	__seed ^= __seed << 17;
	return __seed;
}

void random_seed(u64 seed)
{
	__seed = seed;
}

void random_seed_default()
{
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	__seed = ft.dwLowDateTime | (u64(ft.dwHighDateTime) << 32);
}

u64 random_u64()
{
	return __xorshift64();
}

i64 random_signed(i64 min_value, i64 max_value)
{
	u64 range = u64(max_value) - u64(min_value) + 1;
	u64 threshold = (~range) % range;

	u64 r;
	do
	{
		r = random_u64();
	} while (r < threshold);

	return min_value + i64(r % range);
}


}
