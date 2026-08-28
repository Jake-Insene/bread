#pragma once

#define Bit(n) (1<<n)


#define _MakeString(x) #x
#define MakeString(x) _MakeString(x)

#define _Concat(x, y) x##y
#define Concat(x, y) _Concat(x, y)

#define DisableCopy(type)\
	type(const type&) = delete;\
	type& operator=(const type&) = delete;

#define DisableMove(type)\
	type(type&&) = delete;\
	type& operator=(type&&) = delete;

#define EnableBitOp(enum_type)\
	constexpr enum_type operator|(enum_type lhs, enum_type rhs)\
	{\
		return static_cast<enum_type>(static_cast<u64>(lhs) | static_cast<u64>(rhs));\
	}\
	constexpr void operator|=(enum_type& lhs, enum_type rhs)\
	{\
		reinterpret_cast<Core::EnumIntType<enum_type>&>(lhs) |= static_cast<Core::EnumIntType<enum_type>>(rhs);\
	}\
	constexpr enum_type operator&(enum_type lhs, enum_type rhs)\
	{\
		return static_cast<enum_type>(static_cast<u64>(lhs) & static_cast<u64>(rhs));\
	}\
	constexpr void operator&=(enum_type& lhs, enum_type rhs)\
	{\
		reinterpret_cast<Core::EnumIntType<enum_type>&>(lhs) &= static_cast<Core::EnumIntType<enum_type>>(rhs);\
	}\
	constexpr enum_type operator^(enum_type lhs, enum_type rhs)\
	{\
		return static_cast<enum_type>(static_cast<u64>(lhs) ^ static_cast<u64>(rhs));\
	}\
	constexpr void operator^=(enum_type& lhs, enum_type rhs)\
	{\
		reinterpret_cast<Core::EnumIntType<enum_type>&>(lhs) ^= static_cast<Core::EnumIntType<enum_type>>(rhs);\
	}\

