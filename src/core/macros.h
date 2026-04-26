#pragma once

#define Bit(n) (1<<n)


#define _MakeString(x) #x
#define MakeString(x) _MakeString(x)

#define _Concat(x, y) x##y
#define Concat(x, y) _Concat(x, y)

#define EnableBitOp(enum_type) \
	constexpr enum_type operator|(enum_type lhs, enum_type rhs)\
	{\
		return static_cast<enum_type>(static_cast<u64>(lhs) | static_cast<u64>(rhs));\
	}\
	constexpr void operator|=(enum_type& lhs, enum_type rhs)\
	{\
		reinterpret_cast<EnumIntType<enum_type>&>(lhs) |= static_cast<EnumIntType<enum_type>>(rhs);\
	}\
	constexpr enum_type operator&(enum_type lhs, enum_type rhs)\
	{\
		return static_cast<enum_type>(static_cast<u64>(lhs) & static_cast<u64>(rhs));\
	}\
	constexpr void operator&=(enum_type& lhs, enum_type rhs)\
	{\
		reinterpret_cast<EnumIntType<enum_type>&>(lhs) &= static_cast<EnumIntType<enum_type>>(rhs);\
	}\

