#pragma once

using i8 = char;
using u8 = unsigned char;
using i16 = short;
using u16 = unsigned short;
using i32 = int;
using u32 = unsigned int;
using i64 = long long;
using u64 = unsigned long long;

using f32 = float;
using f64 = double;

using usize = u64;
using isize = i64;

using MemoryAddress = usize;

/*
* Checking type sizes.
*/
static_assert(sizeof(i8) == 1 && sizeof(u8) == 1);
static_assert(sizeof(i16) == 2 && sizeof(u16) == 2);
static_assert(sizeof(i32) == 4 && sizeof(u32) == 4);
static_assert(sizeof(i64) == 8 && sizeof(u64) == 8);
static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);

static_assert(sizeof(MemoryAddress) == sizeof(void*));

