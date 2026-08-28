#pragma once
#include "Core/Header.hpp"


namespace Collections
{

struct StringResult
{
	u8 result[64];
	usize begin;
	usize len;
};

struct StringUtility
{
	template<typename T>
	static constexpr StringResult fp_to_string(T fp, i32 decimals)
	{
		StringResult result = {};
		u8* it = result.result;

		if (fp < 0.0f)
		{
			*it++ = '-';
			fp = -fp;
		}

		i32 integer_part = i32(fp);
		T fractional_part = fp - T(integer_part);

		u8 tmp[32] = {};
		u8* t = tmp;
		do
		{
			*t++ = '0' + (integer_part % 10);
			integer_part /= 10;
		} while (integer_part != 0);

		while (t != tmp)
		{
			*it++ = *--t;
		}

		if (decimals > 0)
		{
			*it++ = '.';
			for (i32 i = 0; i < decimals; i++)
			{
				fractional_part *= 10.0f;
				i32 digit = i32(fractional_part);
				*it++ = '0' + i8(digit);
				fractional_part -= digit;
			}
		}

		result.len = it - result.result;
		return result;
	}

	template<typename T>
	requires(Core::IsInteger<T>)
	static constexpr StringResult integer_to_string(T integer, i32 base)
	{
		using Type = Core::RemoveCVRef<T>;
		using Unsigned = Core::MakeUnsigned<Type>;
		StringResult result = {};
		auto end = result.result + 64;
		usize buffer_index = 0;

		Unsigned u = Unsigned(integer);
		if constexpr(Core::IsSigned<Type>)
		{
			u = integer < 0 ? Unsigned(-integer) : u;
		}

		switch (base)
		{
		case 10:
		{
			do
			{
				*--end = ('0' + u % 10);
				u /= 10;
				buffer_index++;
			} while (u != 0);

			if constexpr(Core::IsSigned<Type>)
			{
				if (integer < 0)
				{
					*--end = '-';
					buffer_index++;
				}
			}
		}
		break;
		case 16:
		{
			u32 hex_digit_count = 0;

			static constexpr char HexChar[16] =
			{
				'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
			};
			do
			{
				*--end = HexChar[u & 0xF];
				u >>= 4;
				hex_digit_count++;
				buffer_index++;
			} while (hex_digit_count < (sizeof(Type) * 8) / 4);

			if constexpr(Core::IsSigned<Type>)
			{
				if (integer < 0)
				{
					*--end = '-';
					buffer_index++;
				}
			}
		}
		break;
		default:
			break;
		}

		result.begin = end - result.result;
		result.len = buffer_index;
		return result;
	}
};

}
